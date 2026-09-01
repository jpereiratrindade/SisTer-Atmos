// SPDX-License-Identifier: GPL-3.0-or-later
#include "sister/atmos/http.hpp"
#include "sister/atmos/web_assets.hpp"

#include <cctype>
#include <cmath>
#include <optional>
#include <sstream>
#include <utility>

namespace sister::atmos::http {
namespace {

Response json_error(int status, std::string_view error, std::string_view message = {}) {
    std::ostringstream body;
    body << "{\"error\":\"" << error << '\"';
    if (!message.empty()) {
        body << ",\"message\":\"";
        for (const char ch : message) {
            if (ch == '\"' || ch == '\\') body << '\\';
            if (ch == '\n' || ch == '\r') body << ' ';
            else body << ch;
        }
        body << '\"';
    }
    body << '}';
    return {.status = status, .content_type = "application/json; charset=utf-8", .body = body.str()};
}

std::string url_decode(std::string_view value) {
    std::string result;
    result.reserve(value.size());
    const auto hex = [](char ch) -> int {
        if (ch >= '0' && ch <= '9') return ch - '0';
        if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
        if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
        return -1;
    };
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '+') result.push_back(' ');
        else if (value[i] == '%' && i + 2 < value.size()) {
            const int hi = hex(value[i + 1]);
            const int lo = hex(value[i + 2]);
            if (hi >= 0 && lo >= 0) {
                result.push_back(static_cast<char>(hi * 16 + lo));
                i += 2;
            } else result.push_back(value[i]);
        } else result.push_back(value[i]);
    }
    return result;
}

std::optional<std::string> query_parameter(std::string_view target, std::string_view name) {
    const auto query = target.find('?');
    if (query == std::string_view::npos) return std::nullopt;
    auto remaining = target.substr(query + 1);
    while (!remaining.empty()) {
        const auto amp = remaining.find('&');
        const auto item = remaining.substr(0, amp);
        const auto equal = item.find('=');
        if (equal != std::string_view::npos && item.substr(0, equal) == name) return url_decode(item.substr(equal + 1));
        if (amp == std::string_view::npos) break;
        remaining.remove_prefix(amp + 1);
    }
    return std::nullopt;
}

std::string_view route_path(std::string_view target) {
    const auto query = target.find('?');
    return target.substr(0, query);
}

std::optional<std::string> json_string(std::string_view body, std::string_view key) {
    const auto needle = "\"" + std::string{key} + "\"";
    auto pos = body.find(needle);
    if (pos == std::string_view::npos) return std::nullopt;
    pos = body.find(':', pos + needle.size());
    if (pos == std::string_view::npos) return std::nullopt;
    pos = body.find('\"', pos + 1);
    if (pos == std::string_view::npos) return std::nullopt;
    const auto end = body.find('\"', pos + 1);
    if (end == std::string_view::npos) return std::nullopt;
    return std::string{body.substr(pos + 1, end - pos - 1)};
}

std::optional<double> json_number(std::string_view body, std::string_view key) {
    const auto needle = "\"" + std::string{key} + "\"";
    auto pos = body.find(needle);
    if (pos == std::string_view::npos) return std::nullopt;
    pos = body.find(':', pos + needle.size());
    if (pos == std::string_view::npos) return std::nullopt;
    ++pos;
    while (pos < body.size() && std::isspace(static_cast<unsigned char>(body[pos])) != 0) ++pos;
    try {
        std::size_t used = 0;
        const double value = std::stod(std::string{body.substr(pos)}, &used);
        if (used == 0 || !std::isfinite(value)) return std::nullopt;
        return value;
    } catch (...) { return std::nullopt; }
}

Response health_response() {
    return {.status = 200, .content_type = "application/json; charset=utf-8", .body = R"({"system_id":"sister_atmos","status":"ok","version":"0.2.0"})"};
}

Response ready_response() {
    return {.status = 200, .content_type = "application/json; charset=utf-8", .body = R"({"system_id":"sister_atmos","status":"ready","version":"0.2.0","scope":"current_declared_capabilities","declared_capability":"governed_precipitation_explorer"})"};
}

Response status_response() {
    return {.status = 200, .content_type = "application/json; charset=utf-8", .body = R"({"system_id":"sister_atmos","operational_status":"ready","readiness_scope":"current_declared_capabilities","complete":false,"evolution_status":"incomplete","completion_policy":"continuous_evolution","capabilities":["location_search","best_match_precipitation","era5_precipitation","nasa_power_precipitation","temporal_coverage","provenance","municipality_boundary","operational_h3_sampling","rs_territorial_classification","location_comparison"]})"};
}

} // namespace

Application::Application()
    : provider_(make_remote_climate_provider()) {}

Application::Application(std::shared_ptr<const ClimateProvider> provider) noexcept
    : provider_(std::move(provider)) {}

Response Application::handle(const std::string_view method, const std::string_view target, const std::string_view req_body) const {
    const auto path = route_path(target);
    const bool readable = method == "GET" || method == "HEAD";

    if ((path == "/" || path == "/index.html") && readable) return {.status = 200, .content_type = "text/html; charset=utf-8", .body = std::string{web_assets::index_html}};
    if (path == "/assets/atmos.css" && readable) return {.status = 200, .content_type = "text/css; charset=utf-8", .body = std::string{web_assets::stylesheet}};
    if (path == "/assets/atmos-overrides.css" && readable) return {.status = 200, .content_type = "text/css; charset=utf-8", .body = std::string{web_assets::style_overrides}};
    if (path == "/assets/atmos.js" && readable) return {.status = 200, .content_type = "text/javascript; charset=utf-8", .body = std::string{web_assets::javascript}};
    if (path == "/assets/rs-territories.csv" && readable) return {.status = 200, .content_type = "text/csv; charset=utf-8", .body = std::string{web_assets::rs_territories}};
    if (path == "/api/status" && readable) return status_response();
    if ((path == "/health" || path == "/api/health" || path == "/_sister/health") && readable) return health_response();
    if (path == "/_sister/ready" && readable) return ready_response();

    if (path == "/api/locations") {
        if (!readable) return json_error(405, "method_not_allowed");
        const auto query = query_parameter(target, "q");
        if (!query || query->size() < 2) return json_error(400, "invalid_query", "Informe q com ao menos dois caracteres.");
        if (!provider_) return json_error(503, "provider_unavailable", "Provider meteorológico indisponível.");
        const auto locations = provider_->search_locations(*query);
        if (!locations) return json_error(502, "provider_error", locations.error());
        return {.status = 200, .content_type = "application/json; charset=utf-8", .body = locations_to_json(*locations)};
    }

    if (path == "/api/territories/municipality") {
        if (!readable) return json_error(405, "method_not_allowed");
        const auto name = query_parameter(target, "name");
        const auto admin1 = query_parameter(target, "admin1");
        if (!name || !admin1 || name->empty() || admin1->empty()) return json_error(400, "invalid_query", "name e admin1 são obrigatórios.");
        if (!provider_) return json_error(503, "provider_unavailable", "Provider territorial indisponível.");
        const auto territory = provider_->municipality_boundary(*name, *admin1);
        if (!territory) return json_error(502, "territory_provider_error", territory.error());
        return {.status = 200, .content_type = "application/json; charset=utf-8", .body = territory_to_json(*territory)};
    }

    if (path == "/api/analyses/precipitation") {
        if (method != "POST") return json_error(405, "method_not_allowed");
        const auto latitude = json_number(req_body, "latitude");
        const auto longitude = json_number(req_body, "longitude");
        const auto start = json_string(req_body, "start_date");
        const auto end = json_string(req_body, "end_date");
        const auto product = json_string(req_body, "requested_product");
        if (!latitude || !longitude || !start || !end || !product) return json_error(400, "invalid_request", "latitude, longitude, start_date, end_date e requested_product são obrigatórios.");
        if (!provider_) return json_error(503, "provider_unavailable", "Provider meteorológico indisponível.");
        const auto analysis = provider_->analyze({.latitude = *latitude, .longitude = *longitude, .start_date = *start, .end_date = *end, .requested_product = *product});
        if (!analysis) return json_error(502, "provider_error", analysis.error());
        return {.status = 200, .content_type = "application/json; charset=utf-8", .body = analysis_to_json(*analysis)};
    }

    if (path == "/api/analyses/precipitation/spatial") {
        if (method != "POST") return json_error(405, "method_not_allowed");
        const auto latitude = json_number(req_body, "latitude");
        const auto longitude = json_number(req_body, "longitude");
        const auto start = json_string(req_body, "start_date");
        const auto end = json_string(req_body, "end_date");
        const auto product = json_string(req_body, "requested_product");
        const auto municipality = json_string(req_body, "municipality_name");
        const auto admin1 = json_string(req_body, "admin1");
        if (!latitude || !longitude || !start || !end || !product || !municipality || !admin1) return json_error(400, "invalid_request", "Campos espaciais obrigatórios ausentes.");
        if (!provider_) return json_error(503, "provider_unavailable", "Provider espacial indisponível.");
        const auto analysis = provider_->operational_spatial_analysis({.latitude = *latitude, .longitude = *longitude, .start_date = *start, .end_date = *end, .requested_product = *product}, *municipality, *admin1);
        if (!analysis) return json_error(502, "spatial_provider_error", analysis.error());
        return {.status = 200, .content_type = "application/json; charset=utf-8", .body = spatial_analysis_to_json(*analysis)};
    }

    if (method != "GET" && method != "HEAD" && method != "POST") return json_error(405, "method_not_allowed");
    return json_error(404, "not_found");
}

std::string reason_phrase(const int status) {
    switch (status) {
    case 200: return "OK";
    case 400: return "Bad Request";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    default: return "Error";
    }
}

std::string serialize_response(const Response& response) {
    std::ostringstream out;
    out << "HTTP/1.1 " << response.status << ' ' << reason_phrase(response.status) << "\r\n"
        << "Content-Type: " << response.content_type << "\r\nContent-Length: " << response.body.size()
        << "\r\nConnection: close\r\nCache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\n"
        << "Referrer-Policy: no-referrer\r\nPermissions-Policy: geolocation=(), microphone=(), camera=()\r\n"
        << "Content-Security-Policy: default-src 'self'; style-src 'self'; script-src 'self'; connect-src 'self'; img-src 'self' data:; frame-ancestors 'none'; base-uri 'none'; form-action 'self'\r\n\r\n"
        << response.body;
    return out.str();
}

} // namespace sister::atmos::http
