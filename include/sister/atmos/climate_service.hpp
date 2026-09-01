// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <expected>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace sister::atmos {

struct Location final {
    std::string id;
    std::string name;
    std::string admin1;
    std::string country;
    double latitude{};
    double longitude{};
};

struct DailyClimateValue final {
    std::string date;
    double precipitation_mm{};
};

struct ClimateQuery final {
    double latitude{};
    double longitude{};
    std::string start_date;
    std::string end_date;
    std::string requested_product;
};

struct ClimateAnalysis final {
    std::string requested_product;
    std::string used_product;
    std::string provider;
    std::string source_uri;
    std::string license;
    std::string variable;
    std::string timezone;
    std::string method;
    std::string requested_start;
    std::string requested_end;
    std::string effective_start;
    std::string effective_end;
    std::vector<std::string> missing_dates;
    std::vector<DailyClimateValue> daily;
};

struct TerritoryBoundary final {
    std::string code;
    std::string name;
    std::string source_uri;
    std::string geojson;
};

struct SpatialCell final {
    std::string h3_index;
    double latitude{};
    double longitude{};
    double precipitation_mm{};
    std::vector<std::pair<double, double>> boundary;
};

struct SpatialAnalysis final {
    std::string analysis_mode;
    std::string used_product;
    int sampling_resolution{};
    int presentation_resolution{};
    bool adaptive_sampling{};
    std::string aggregation;
    std::vector<SpatialCell> cells;
};

class ClimateProvider {
public:
    virtual ~ClimateProvider() = default;

    [[nodiscard]] virtual std::expected<std::vector<Location>, std::string>
    search_locations(std::string_view query) const = 0;

    [[nodiscard]] virtual std::expected<ClimateAnalysis, std::string>
    analyze(const ClimateQuery& query) const = 0;

    [[nodiscard]] virtual std::expected<TerritoryBoundary, std::string>
    municipality_boundary(std::string_view name, std::string_view admin1) const = 0;

    [[nodiscard]] virtual std::expected<SpatialAnalysis, std::string>
    operational_spatial_analysis(
        const ClimateQuery& query,
        std::string_view municipality_name,
        std::string_view admin1
    ) const = 0;
};

[[nodiscard]] std::shared_ptr<const ClimateProvider> make_remote_climate_provider();

[[nodiscard]] std::string locations_to_json(const std::vector<Location>& locations);
[[nodiscard]] std::string analysis_to_json(const ClimateAnalysis& analysis);
[[nodiscard]] std::string territory_to_json(const TerritoryBoundary& territory);
[[nodiscard]] std::string spatial_analysis_to_json(const SpatialAnalysis& analysis);

} // namespace sister::atmos
