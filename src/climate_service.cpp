// SPDX-License-Identifier: GPL-3.0-or-later
#include "sister/atmos/climate_service.hpp"

#include <h3api.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <curl/curl.h>
#include <iomanip>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace sister::atmos {
namespace {

constexpr std::string_view kOpenMeteoLicense = "CC BY 4.0";

std::string escape_json(std::string_view value) {
    std::ostringstream out;
    for (const char raw_ch : value) {
        const auto ch = static_cast<unsigned char>(raw_ch);
        switch (ch) {
        case '\"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\b': out << "\\b"; break;
        case '\f': out << "\\f"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (ch < 0x20U) {
                out << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                    << static_cast<unsigned>(ch) << std::dec;
            } else {
                out << static_cast<char>(ch);
            }
        }
    }
    return out.str();
}

std::size_t curl_write(char* data, std::size_t size, std::size_t count, void* user) {
    const auto bytes = size * count;
    static_cast<std::string*>(user)->append(data, bytes);
    return bytes;
}

std::expected<std::string, std::string> http_get(const std::string& url) {
    using CurlHandle = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;
    CurlHandle handle{curl_easy_init(), curl_easy_cleanup};
    if (!handle) return std::unexpected("Não foi possível inicializar o cliente meteorológico.");

    std::string response;
    const auto* curl_runtime = curl_version_info(CURLVERSION_NOW);
    const std::string user_agent = std::string{"curl/"}
        + (curl_runtime != nullptr && curl_runtime->version != nullptr ? curl_runtime->version : "unknown")
        + " sister-atmos/0.2";
    curl_easy_setopt(handle.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle.get(), CURLOPT_WRITEFUNCTION, curl_write);
    curl_easy_setopt(handle.get(), CURLOPT_WRITEDATA, &response);
    // Some institutional proxies allow recognized HTTP clients by User-Agent.
    // Keep Atmos identifiable while preserving libcurl's canonical prefix.
    curl_easy_setopt(handle.get(), CURLOPT_USERAGENT, user_agent.c_str());
    curl_easy_setopt(handle.get(), CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(handle.get(), CURLOPT_TIMEOUT, 25L);
    curl_easy_setopt(handle.get(), CURLOPT_CONNECTTIMEOUT, 8L);
    curl_easy_setopt(handle.get(), CURLOPT_SSL_VERIFYPEER, 1L);
    if (std::getenv("ATMOS_CURL_VERBOSE") != nullptr) {
        curl_easy_setopt(handle.get(), CURLOPT_VERBOSE, 1L);
    }

    const auto result = curl_easy_perform(handle.get());
    if (result != CURLE_OK) {
        return std::unexpected(std::string{"Falha ao consultar a fonte: "} + curl_easy_strerror(result));
    }

    long status = 0;
    curl_easy_getinfo(handle.get(), CURLINFO_RESPONSE_CODE, &status);
    if (status < 200 || status >= 300) {
        return std::unexpected("A fonte meteorológica respondeu HTTP " + std::to_string(status) + ".");
    }
    return response;
}

std::string url_encode(std::string_view value) {
    using CurlHandle = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;
    CurlHandle handle{curl_easy_init(), curl_easy_cleanup};
    if (!handle) return {};
    char* encoded = curl_easy_escape(handle.get(), value.data(), static_cast<int>(value.size()));
    if (encoded == nullptr) return {};
    std::string result{encoded};
    curl_free(encoded);
    return result;
}

std::optional<std::string_view> member_value(std::string_view json, std::string_view key, std::size_t from = 0) {
    const std::string needle = "\"" + std::string{key} + "\"";
    auto pos = json.find(needle, from);
    if (pos == std::string_view::npos) return std::nullopt;
    pos = json.find(':', pos + needle.size());
    if (pos == std::string_view::npos) return std::nullopt;
    ++pos;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\n' || json[pos] == '\r' || json[pos] == '\t')) ++pos;
    if (pos >= json.size()) return std::nullopt;

    if (json[pos] == '\"') {
        const auto start = ++pos;
        bool escaped = false;
        for (; pos < json.size(); ++pos) {
            if (!escaped && json[pos] == '\"') return json.substr(start, pos - start);
            escaped = !escaped && json[pos] == '\\';
            if (json[pos] != '\\') escaped = false;
        }
        return std::nullopt;
    }
    if (json[pos] == '[' || json[pos] == '{') {
        const char opening = json[pos];
        const char closing = opening == '[' ? ']' : '}';
        const auto start = pos;
        int depth = 0;
        bool quoted = false;
        bool escaped = false;
        for (; pos < json.size(); ++pos) {
            const char ch = json[pos];
            if (quoted) {
                if (!escaped && ch == '\"') quoted = false;
                escaped = !escaped && ch == '\\';
                if (ch != '\\') escaped = false;
                continue;
            }
            if (ch == '\"') { quoted = true; continue; }
            if (ch == opening) ++depth;
            if (ch == closing && --depth == 0) return json.substr(start, pos - start + 1);
        }
        return std::nullopt;
    }
    const auto end = json.find_first_of(",}\r\n", pos);
    return json.substr(pos, end == std::string_view::npos ? json.size() - pos : end - pos);
}

std::optional<double> number_member(std::string_view json, std::string_view key) {
    const auto raw = member_value(json, key);
    if (!raw) return std::nullopt;
    try {
        std::size_t used = 0;
        const double value = std::stod(std::string{*raw}, &used);
        if (used == 0 || !std::isfinite(value)) return std::nullopt;
        return value;
    } catch (...) {
        return std::nullopt;
    }
}

std::vector<std::string> string_array(std::string_view raw) {
    std::vector<std::string> result;
    std::size_t pos = 0;
    while ((pos = raw.find('\"', pos)) != std::string_view::npos) {
        const auto end = raw.find('\"', pos + 1);
        if (end == std::string_view::npos) break;
        result.emplace_back(raw.substr(pos + 1, end - pos - 1));
        pos = end + 1;
    }
    return result;
}

std::vector<std::optional<double>> number_array(std::string_view raw) {
    std::vector<std::optional<double>> result;
    std::size_t pos = raw.find('[');
    if (pos == std::string_view::npos) return result;
    ++pos;
    while (pos < raw.size()) {
        while (pos < raw.size() && (raw[pos] == ' ' || raw[pos] == ',' || raw[pos] == '\n')) ++pos;
        if (pos >= raw.size() || raw[pos] == ']') break;
        const auto end = raw.find_first_of(",]", pos);
        const auto token = raw.substr(pos, end == std::string_view::npos ? raw.size() - pos : end - pos);
        if (token.find("null") != std::string_view::npos) {
            result.push_back(std::nullopt);
        } else {
            try { result.push_back(std::stod(std::string{token})); }
            catch (...) { result.push_back(std::nullopt); }
        }
        if (end == std::string_view::npos) break;
        pos = end + 1;
    }
    return result;
}

std::vector<std::string_view> object_array(std::string_view raw) {
    std::vector<std::string_view> objects;
    int depth = 0;
    std::size_t start = 0;
    bool quoted = false;
    bool escaped = false;
    for (std::size_t i = 0; i < raw.size(); ++i) {
        const char ch = raw[i];
        if (quoted) {
            if (!escaped && ch == '\"') quoted = false;
            escaped = !escaped && ch == '\\';
            if (ch != '\\') escaped = false;
            continue;
        }
        if (ch == '\"') { quoted = true; continue; }
        if (ch == '{' && depth++ == 0) start = i;
        if (ch == '}' && --depth == 0) objects.push_back(raw.substr(start, i - start + 1));
    }
    return objects;
}

std::vector<std::string> dates_between(std::string_view start, std::string_view end) {
    auto parse = [](std::string_view raw) -> std::optional<std::chrono::sys_days> {
        if (raw.size() != 10) return std::nullopt;
        try {
            const int y = std::stoi(std::string{raw.substr(0, 4)});
            const unsigned m = static_cast<unsigned>(std::stoul(std::string{raw.substr(5, 2)}));
            const unsigned d = static_cast<unsigned>(std::stoul(std::string{raw.substr(8, 2)}));
            const std::chrono::year_month_day date{std::chrono::year{y}, std::chrono::month{m}, std::chrono::day{d}};
            if (!date.ok()) return std::nullopt;
            return std::chrono::sys_days{date};
        } catch (...) { return std::nullopt; }
    };
    const auto first = parse(start);
    const auto last = parse(end);
    if (!first || !last || *last < *first) return {};
    std::vector<std::string> result;
    for (auto day = *first; day <= *last; day += std::chrono::days{1}) {
        const std::chrono::year_month_day date{day};
        std::ostringstream out;
        out << static_cast<int>(date.year()) << '-' << std::setw(2) << std::setfill('0')
            << static_cast<unsigned>(date.month()) << '-' << std::setw(2)
            << static_cast<unsigned>(date.day());
        result.push_back(out.str());
    }
    return result;
}

class RemoteClimateProvider final : public ClimateProvider {
public:
    [[nodiscard]] std::expected<std::vector<Location>, std::string>
    search_locations(std::string_view query) const override {
        if (query.size() < 2) return std::unexpected("Informe ao menos dois caracteres.");
        const auto body = http_get("https://geocoding-api.open-meteo.com/v1/search?count=12&language=pt&format=json&name=" + url_encode(query));
        if (!body) return std::unexpected(body.error());
        const auto results = member_value(*body, "results");
        if (!results) return std::vector<Location>{};

        std::vector<Location> locations;
        for (const auto object : object_array(*results)) {
            const auto name = member_value(object, "name");
            const auto lat = number_member(object, "latitude");
            const auto lon = number_member(object, "longitude");
            if (!name || !lat || !lon) continue;
            const auto id = member_value(object, "id");
            const auto admin = member_value(object, "admin1");
            const auto country = member_value(object, "country");
            locations.push_back(Location{
                .id = id ? std::string{*id} : std::to_string(locations.size()),
                .name = std::string{*name},
                .admin1 = admin ? std::string{*admin} : "",
                .country = country ? std::string{*country} : "",
                .latitude = *lat,
                .longitude = *lon,
            });
        }
        return locations;
    }

    [[nodiscard]] std::expected<ClimateAnalysis, std::string>
    analyze(const ClimateQuery& query) const override {
        if (!std::isfinite(query.latitude) || query.latitude < -90.0 || query.latitude > 90.0 ||
            !std::isfinite(query.longitude) || query.longitude < -180.0 || query.longitude > 180.0) {
            return std::unexpected("Coordenadas inválidas.");
        }
        const auto expected_dates = dates_between(query.start_date, query.end_date);
        if (expected_dates.empty() || expected_dates.size() > 3660) return std::unexpected("Período inválido ou maior que dez anos.");

        const std::string coordinates = "&latitude=" + std::to_string(query.latitude) +
            "&longitude=" + std::to_string(query.longitude);

        if (query.requested_product == "nasa_power") {
            const auto compact = [](std::string date) {
                date.erase(std::remove(date.begin(), date.end(), '-'), date.end());
                return date;
            };
            const std::string url = "https://power.larc.nasa.gov/api/temporal/daily/point?parameters=PRECTOTCORR&community=AG&format=JSON&time-standard=LST"
                + coordinates + "&start=" + compact(query.start_date) + "&end=" + compact(query.end_date);
            const auto body = http_get(url);
            if (!body) return std::unexpected(body.error());
            const auto values = member_value(*body, "PRECTOTCORR");
            if (!values) return std::unexpected("A NASA POWER não retornou PRECTOTCORR.");
            ClimateAnalysis result{
                .requested_product = query.requested_product,
                .used_product = "nasa_power_merra2",
                .provider = "NASA POWER",
                .source_uri = "https://power.larc.nasa.gov/api/temporal/daily/point",
                .license = "NASA POWER Data Access",
                .variable = "PRECTOTCORR",
                .timezone = "LST",
                .method = "daily modeled point series; missing values preserved",
                .requested_start = query.start_date,
                .requested_end = query.end_date,
                .effective_start = {}, .effective_end = {}, .missing_dates = {}, .daily = {},
            };
            for (const auto& date : expected_dates) {
                const auto precipitation = number_member(*values, compact(date));
                if (!precipitation || *precipitation <= -900.0) result.missing_dates.push_back(date);
                else result.daily.push_back({.date = date, .precipitation_mm = std::max(0.0, *precipitation)});
            }
            if (result.daily.empty()) return std::unexpected("A NASA POWER não possui precipitação disponível para o período.");
            result.effective_start = result.daily.front().date;
            result.effective_end = result.daily.back().date;
            return result;
        }

        std::string url;
        std::string used_product;
        if (query.requested_product == "best_match") {
            used_product = "best_match_dynamic";
            url = "https://api.open-meteo.com/v1/forecast?daily=precipitation_sum&timezone=America%2FSao_Paulo";
        } else if (query.requested_product == "era5") {
            used_product = "era5";
            url = "https://archive-api.open-meteo.com/v1/archive?daily=precipitation_sum&models=era5&timezone=America%2FSao_Paulo";
        } else {
            return std::unexpected("Produto não governado neste corte. Use best_match ou era5.");
        }
        url += coordinates + "&start_date=" + query.start_date + "&end_date=" + query.end_date;
        const auto body = http_get(url);
        if (!body) return std::unexpected(body.error());
        const auto daily = member_value(*body, "daily");
        if (!daily) return std::unexpected("A fonte não retornou a série diária esperada.");
        const auto times_raw = member_value(*daily, "time");
        const auto rain_raw = member_value(*daily, "precipitation_sum");
        if (!times_raw || !rain_raw) return std::unexpected("Série diária incompleta na resposta da fonte.");
        const auto times = string_array(*times_raw);
        const auto rain = number_array(*rain_raw);

        ClimateAnalysis result{
            .requested_product = query.requested_product,
            .used_product = used_product,
            .provider = "Open-Meteo",
            .source_uri = url.substr(0, url.find('?')),
            .license = std::string{kOpenMeteoLicense},
            .variable = "precipitation_sum",
            .timezone = "America/Sao_Paulo",
            .method = "daily modeled point series; missing values preserved",
            .requested_start = query.start_date,
            .requested_end = query.end_date,
            .effective_start = {},
            .effective_end = {},
            .missing_dates = {},
            .daily = {},
        };
        std::map<std::string, double, std::less<>> available;
        for (std::size_t i = 0; i < std::min(times.size(), rain.size()); ++i) {
            if (rain[i] && std::isfinite(*rain[i]) && *rain[i] >= 0.0) available[times[i]] = *rain[i];
        }
        for (const auto& date : expected_dates) {
            const auto it = available.find(date);
            if (it == available.end()) result.missing_dates.push_back(date);
            else result.daily.push_back({.date = date, .precipitation_mm = it->second});
        }
        if (result.daily.empty()) return std::unexpected("A fonte não possui precipitação disponível para o período.");
        result.effective_start = result.daily.front().date;
        result.effective_end = result.daily.back().date;
        return result;
    }

    [[nodiscard]] std::expected<TerritoryBoundary, std::string>
    municipality_boundary(std::string_view name, std::string_view admin1) const override {
        static const std::map<std::string_view, std::string_view, std::less<>> states{
            {"Acre", "AC"}, {"Alagoas", "AL"}, {"Amapá", "AP"}, {"Amazonas", "AM"},
            {"Bahia", "BA"}, {"Ceará", "CE"}, {"Distrito Federal", "DF"},
            {"Espírito Santo", "ES"}, {"Goiás", "GO"}, {"Maranhão", "MA"},
            {"Mato Grosso", "MT"}, {"Mato Grosso do Sul", "MS"}, {"Minas Gerais", "MG"},
            {"Pará", "PA"}, {"Paraíba", "PB"}, {"Paraná", "PR"}, {"Pernambuco", "PE"},
            {"Piauí", "PI"}, {"Rio de Janeiro", "RJ"}, {"Rio Grande do Norte", "RN"},
            {"Rio Grande do Sul", "RS"}, {"Rondônia", "RO"}, {"Roraima", "RR"},
            {"Santa Catarina", "SC"}, {"São Paulo", "SP"}, {"Sergipe", "SE"},
            {"Tocantins", "TO"},
        };
        const auto state = states.find(admin1);
        if (state == states.end()) return std::unexpected("A malha municipal está disponível para localidades brasileiras reconhecidas.");

        const std::string list_url = "https://servicodados.ibge.gov.br/api/v1/localidades/estados/"
            + std::string{state->second} + "/municipios?orderBy=nome";
        const auto list = http_get(list_url);
        if (!list) return std::unexpected(list.error());
        std::string code;
        for (const auto object : object_array(*list)) {
            const auto candidate = member_value(object, "nome");
            if (!candidate || *candidate != name) continue;
            const auto id = member_value(object, "id");
            if (id) code = std::string{*id};
            break;
        }
        if (code.empty()) return std::unexpected("O município não foi localizado no catálogo do IBGE.");

        const std::string mesh_url = "https://servicodados.ibge.gov.br/api/v3/malhas/municipios/" + code
            + "?formato=application%2Fvnd.geo%2Bjson&qualidade=minima";
        const auto mesh = http_get(mesh_url);
        if (!mesh) return std::unexpected(mesh.error());
        if (mesh->find("\"type\"") == std::string::npos) return std::unexpected("O IBGE não retornou uma malha GeoJSON válida.");
        return TerritoryBoundary{.code = code, .name = std::string{name}, .source_uri = mesh_url.substr(0, mesh_url.find('?')), .geojson = *mesh};
    }

    [[nodiscard]] std::expected<SpatialAnalysis, std::string>
    operational_spatial_analysis(
        const ClimateQuery& query,
        std::string_view municipality_name,
        std::string_view admin1
    ) const override {
        if (query.requested_product != "best_match") {
            return std::unexpected("A distribuição amostral H3 deste corte exige Best Match operacional.");
        }
        if (dates_between(query.start_date, query.end_date).empty()) return std::unexpected("Período espacial inválido.");

        int sampling_resolution = 6;
        constexpr int presentation_resolution = 5;
        const auto territory = municipality_boundary(municipality_name, admin1);
        if (!territory) return std::unexpected(territory.error());

        nlohmann::json geojson;
        try { geojson = nlohmann::json::parse(territory->geojson); }
        catch (...) { return std::unexpected("A malha municipal do IBGE não pôde ser interpretada."); }

        const auto cover_at_resolution = [&](int resolution) -> std::expected<std::vector<H3Index>, std::string> {
            std::vector<H3Index> collected;
            const auto add_polygon = [&](const nlohmann::json& coordinates) -> std::optional<std::string> {
                if (!coordinates.is_array() || coordinates.empty()) return "Polígono municipal vazio.";
                std::vector<std::vector<LatLng>> rings;
                for (const auto& ring_json : coordinates) {
                    std::vector<LatLng> ring;
                    for (const auto& point : ring_json) {
                        if (!point.is_array() || point.size() < 2) continue;
                        ring.push_back({.lat = degsToRads(point[1].get<double>()), .lng = degsToRads(point[0].get<double>())});
                    }
                    if (ring.size() >= 3) rings.push_back(std::move(ring));
                }
                if (rings.empty()) return "Polígono municipal sem anel exterior válido.";
                GeoLoop exterior{.numVerts = static_cast<int>(rings.front().size()), .verts = rings.front().data()};
                std::vector<GeoLoop> holes;
                for (std::size_t i = 1; i < rings.size(); ++i) holes.push_back({.numVerts = static_cast<int>(rings[i].size()), .verts = rings[i].data()});
                GeoPolygon polygon{.geoloop = exterior, .numHoles = static_cast<int>(holes.size()), .holes = holes.data()};
                int64_t maximum = 0;
                if (maxPolygonToCellsSize(&polygon, resolution, 0, &maximum) != E_SUCCESS || maximum <= 0) return "Falha ao dimensionar cobertura H3.";
                std::vector<H3Index> cells(static_cast<std::size_t>(maximum), H3_NULL);
                if (polygonToCells(&polygon, resolution, 0, cells.data()) != E_SUCCESS) return "Falha ao cobrir município com H3.";
                cells.erase(std::remove(cells.begin(), cells.end(), H3_NULL), cells.end());
                collected.insert(collected.end(), cells.begin(), cells.end());
                return std::nullopt;
            };
            const auto add_geometry = [&](const nlohmann::json& geometry) -> std::optional<std::string> {
                const auto type = geometry.value("type", "");
                const auto& coordinates = geometry["coordinates"];
                if (type == "Polygon") return add_polygon(coordinates);
                if (type == "MultiPolygon") {
                    for (const auto& polygon : coordinates) if (const auto error = add_polygon(polygon)) return error;
                    return std::nullopt;
                }
                return "Geometria municipal não suportada.";
            };
            if (geojson.value("type", "") == "FeatureCollection") {
                for (const auto& feature : geojson["features"]) if (const auto error = add_geometry(feature["geometry"])) return std::unexpected(*error);
            } else if (const auto error = add_geometry(geojson)) return std::unexpected(*error);
            std::sort(collected.begin(), collected.end());
            collected.erase(std::unique(collected.begin(), collected.end()), collected.end());
            return collected;
        };

        auto covered = cover_at_resolution(sampling_resolution);
        if (!covered) return std::unexpected(covered.error());
        bool adaptive_sampling = false;
        if (covered->size() > 800U) {
            adaptive_sampling = true;
            sampling_resolution = presentation_resolution;
            covered = cover_at_resolution(sampling_resolution);
            if (!covered) return std::unexpected(covered.error());
        }
        auto indexes = std::move(*covered);
        if (indexes.empty()) {
            const LatLng origin{.lat = degsToRads(query.latitude), .lng = degsToRads(query.longitude)};
            H3Index fallback = H3_NULL;
            if (latLngToCell(&origin, sampling_resolution, &fallback) == E_SUCCESS) indexes.push_back(fallback);
        }

        SpatialAnalysis result{
            .analysis_mode = "operational_sampling",
            .used_product = "best_match_dynamic",
            .sampling_resolution = sampling_resolution,
            .presentation_resolution = presentation_resolution,
            .adaptive_sampling = adaptive_sampling,
            .aggregation = "unweighted_mean_of_unique_sample_points",
            .cells = {},
        };
        std::map<H3Index, std::pair<double, std::size_t>> presentation_values;
        constexpr std::size_t batch_size = 40;
        for (std::size_t offset = 0; offset < indexes.size(); offset += batch_size) {
            const auto count = std::min(batch_size, indexes.size() - offset);
            std::ostringstream latitudes;
            std::ostringstream longitudes;
            latitudes << std::fixed << std::setprecision(6);
            longitudes << std::fixed << std::setprecision(6);
            for (std::size_t local = 0; local < count; ++local) {
                LatLng center{};
                if (cellToLatLng(indexes[offset + local], &center) != E_SUCCESS) continue;
                if (local != 0) { latitudes << ','; longitudes << ','; }
                latitudes << radsToDegs(center.lat);
                longitudes << radsToDegs(center.lng);
            }
            const std::string url = "https://api.open-meteo.com/v1/forecast?daily=precipitation_sum&timezone=America%2FSao_Paulo"
                "&latitude=" + latitudes.str() + "&longitude=" + longitudes.str()
                + "&start_date=" + query.start_date + "&end_date=" + query.end_date;
            const auto payload = http_get(url);
            if (!payload) return std::unexpected(payload.error());
            const auto locations = object_array(*payload);
            if (locations.size() != count) return std::unexpected("A fonte retornou quantidade inesperada de amostras H3.");
            for (std::size_t local = 0; local < count; ++local) {
                const auto daily = member_value(locations[local], "daily");
                const auto rain_raw = daily ? member_value(*daily, "precipitation_sum") : std::nullopt;
                if (!rain_raw) continue;
                double total = 0.0;
                bool available = false;
                for (const auto value : number_array(*rain_raw)) {
                    if (!value || !std::isfinite(*value) || *value < 0.0) continue;
                    total += *value;
                    available = true;
                }
                if (!available) continue;
                H3Index parent = H3_NULL;
                if (cellToParent(indexes[offset + local], presentation_resolution, &parent) != E_SUCCESS) continue;
                auto& aggregate = presentation_values[parent];
                aggregate.first += total;
                ++aggregate.second;
            }
        }
        for (const auto& [index, aggregate] : presentation_values) {
            LatLng center{};
            CellBoundary cell_boundary{};
            if (cellToLatLng(index, &center) != E_SUCCESS || cellToBoundary(index, &cell_boundary) != E_SUCCESS) continue;
            char h3_text[18]{};
            if (h3ToString(index, h3_text, sizeof(h3_text)) != E_SUCCESS) continue;
            SpatialCell cell{
                .h3_index = h3_text,
                .latitude = radsToDegs(center.lat),
                .longitude = radsToDegs(center.lng),
                .precipitation_mm = aggregate.first / static_cast<double>(aggregate.second),
                .boundary = {},
            };
            for (int vertex = 0; vertex < cell_boundary.numVerts; ++vertex) {
                const auto& point = cell_boundary.verts[vertex];
                cell.boundary.emplace_back(radsToDegs(point.lng), radsToDegs(point.lat));
            }
            result.cells.push_back(std::move(cell));
        }
        if (result.cells.empty()) return std::unexpected("Nenhuma amostra espacial ficou disponível.");
        return result;
    }
};

} // namespace

std::shared_ptr<const ClimateProvider> make_remote_climate_provider() {
    return std::make_shared<RemoteClimateProvider>();
}

std::string locations_to_json(const std::vector<Location>& locations) {
    std::ostringstream out;
    out << "{\"locations\":[";
    for (std::size_t i = 0; i < locations.size(); ++i) {
        if (i != 0) out << ',';
        const auto& item = locations[i];
        out << "{\"id\":\"" << escape_json(item.id) << "\",\"name\":\"" << escape_json(item.name)
            << "\",\"admin1\":\"" << escape_json(item.admin1) << "\",\"country\":\""
            << escape_json(item.country) << "\",\"latitude\":" << item.latitude
            << ",\"longitude\":" << item.longitude << '}';
    }
    out << "]}";
    return out.str();
}

std::string analysis_to_json(const ClimateAnalysis& analysis) {
    double total = 0.0;
    double maximum = 0.0;
    std::size_t rainy_days = 0;
    for (const auto& item : analysis.daily) {
        total += item.precipitation_mm;
        maximum = std::max(maximum, item.precipitation_mm);
        if (item.precipitation_mm >= 1.0) ++rainy_days;
    }
    const double mean = analysis.daily.empty() ? 0.0 : total / static_cast<double>(analysis.daily.size());
    std::ostringstream out;
    out << std::fixed << std::setprecision(2)
        << "{\"analysis_id\":\"atmos-" << std::chrono::system_clock::now().time_since_epoch().count()
        << "\",\"requested_product\":\"" << escape_json(analysis.requested_product)
        << "\",\"used_product\":\"" << escape_json(analysis.used_product)
        << "\",\"requested_period\":{\"start\":\"" << analysis.requested_start << "\",\"end\":\"" << analysis.requested_end
        << "\"},\"effective_period\":{\"start\":\"" << analysis.effective_start << "\",\"end\":\"" << analysis.effective_end
        << "\"},\"complete\":" << (analysis.missing_dates.empty() ? "true" : "false")
        << ",\"available_days\":" << analysis.daily.size() << ",\"missing_dates\":[";
    for (std::size_t i = 0; i < analysis.missing_dates.size(); ++i) {
        if (i != 0) out << ',';
        out << '\"' << analysis.missing_dates[i] << '\"';
    }
    out << "],\"summary\":{\"total_mm\":" << total << ",\"maximum_daily_mm\":" << maximum
        << ",\"mean_daily_mm\":" << mean << ",\"rainy_days\":" << rainy_days
        << "},\"daily\":[";
    double accumulated = 0.0;
    for (std::size_t i = 0; i < analysis.daily.size(); ++i) {
        if (i != 0) out << ',';
        accumulated += analysis.daily[i].precipitation_mm;
        out << "{\"date\":\"" << analysis.daily[i].date << "\",\"precipitation_mm\":"
            << analysis.daily[i].precipitation_mm << ",\"accumulated_mm\":" << accumulated << '}';
    }
    out << "],\"provenance\":{\"provider\":\"" << escape_json(analysis.provider)
        << "\",\"source_uri\":\"" << escape_json(analysis.source_uri)
        << "\",\"license\":\"" << escape_json(analysis.license)
        << "\",\"variable\":\"" << escape_json(analysis.variable)
        << "\",\"timezone\":\"" << escape_json(analysis.timezone)
        << "\",\"method\":\"" << escape_json(analysis.method)
        << "\",\"native_surface_claimed\":false}}";
    return out.str();
}

std::string territory_to_json(const TerritoryBoundary& territory) {
    std::ostringstream out;
    out << "{\"code\":\"" << escape_json(territory.code) << "\",\"name\":\""
        << escape_json(territory.name) << "\",\"source_uri\":\""
        << escape_json(territory.source_uri) << "\",\"boundary\":" << territory.geojson << '}';
    return out.str();
}

std::string spatial_analysis_to_json(const SpatialAnalysis& analysis) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(6)
        << "{\"analysis_mode\":\"" << escape_json(analysis.analysis_mode)
        << "\",\"used_product\":\"" << escape_json(analysis.used_product)
        << "\",\"sampling_resolution\":" << analysis.sampling_resolution
        << ",\"presentation_resolution\":" << analysis.presentation_resolution
        << ",\"adaptive_sampling\":" << (analysis.adaptive_sampling ? "true" : "false")
        << ",\"spatial_aggregation\":\"" << escape_json(analysis.aggregation)
        << "\",\"native_surface_claimed\":false,\"cells\":[";
    for (std::size_t i = 0; i < analysis.cells.size(); ++i) {
        if (i != 0) out << ',';
        const auto& cell = analysis.cells[i];
        out << "{\"h3_index\":\"" << escape_json(cell.h3_index)
            << "\",\"latitude\":" << cell.latitude << ",\"longitude\":" << cell.longitude
            << ",\"precipitation_mm\":" << cell.precipitation_mm << ",\"boundary\":[";
        for (std::size_t vertex = 0; vertex < cell.boundary.size(); ++vertex) {
            if (vertex != 0) out << ',';
            out << '[' << cell.boundary[vertex].first << ',' << cell.boundary[vertex].second << ']';
        }
        out << "]}";
    }
    out << "],\"provenance\":{\"provider\":\"Open-Meteo\",\"sampling\":\"H3 r6 unique centers\",\"presentation\":\"H3 r5 context\",\"license\":\"CC BY 4.0\"}}";
    return out.str();
}

} // namespace sister::atmos
