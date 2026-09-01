// SPDX-License-Identifier: GPL-3.0-or-later
#include "sister/atmos/http.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

namespace {

class FakeClimateProvider final : public sister::atmos::ClimateProvider {
public:
    std::expected<std::vector<sister::atmos::Location>, std::string>
    search_locations(std::string_view) const override {
        return std::vector<sister::atmos::Location>{{
            .id = "1", .name = "Pelotas", .admin1 = "Rio Grande do Sul",
            .country = "Brasil", .latitude = -31.77, .longitude = -52.34,
        }};
    }

    std::expected<sister::atmos::ClimateAnalysis, std::string>
    analyze(const sister::atmos::ClimateQuery& query) const override {
        return sister::atmos::ClimateAnalysis{
            .requested_product = query.requested_product,
            .used_product = "era5",
            .provider = "test-provider",
            .source_uri = "https://example.invalid/climate",
            .license = "test-license",
            .variable = "precipitation_sum",
            .timezone = "America/Sao_Paulo",
            .method = "test method",
            .requested_start = query.start_date,
            .requested_end = query.end_date,
            .effective_start = query.start_date,
            .effective_end = query.end_date,
            .missing_dates = {},
            .daily = {{.date = query.start_date, .precipitation_mm = 12.5}},
        };
    }

    std::expected<sister::atmos::TerritoryBoundary, std::string>
    municipality_boundary(std::string_view name, std::string_view) const override {
        return sister::atmos::TerritoryBoundary{
            .code = "4314407", .name = std::string{name},
            .source_uri = "https://example.invalid/mesh",
            .geojson = R"({"type":"FeatureCollection","features":[]})",
        };
    }

    std::expected<sister::atmos::SpatialAnalysis, std::string>
    operational_spatial_analysis(const sister::atmos::ClimateQuery&, std::string_view, std::string_view) const override {
        return sister::atmos::SpatialAnalysis{
            .analysis_mode = "operational_sampling", .used_product = "best_match_dynamic",
            .sampling_resolution = 6, .presentation_resolution = 5,
            .adaptive_sampling = false,
            .aggregation = "unweighted_mean_of_unique_sample_points",
            .cells = {{.h3_index = "86fake", .latitude = -31.0, .longitude = -52.0,
                .precipitation_mm = 10.0, .boundary = {{-52.1, -31.1}, {-51.9, -31.1}, {-52.0, -30.9}}}},
        };
    }
};

} // namespace

int main() {
    sister::atmos::http::Application app;


    // Root is always a usable presentation shell, even while Atmos evolves.
    {
        const auto resp = app.handle("GET", "/");
        assert(resp.status == 200);
        assert(resp.content_type == "text/html; charset=utf-8");
        assert(resp.body.find("SisTer Atmos") != std::string::npos);
        assert(resp.body.find("PRONTO") != std::string::npos);
        assert(resp.body.find("INCOMPLETO") != std::string::npos);
        assert(resp.body.find("/assets/atmos.css") != std::string::npos);
        assert(resp.body.find("Explorador climático") != std::string::npos);
        assert(resp.body.find("Comparar localidades") != std::string::npos);
    }

    // Presentation assets are served by the same runtime; no external frontend is required.
    {
        const auto resp = app.handle("GET", "/assets/atmos.css");
        assert(resp.status == 200);
        assert(resp.content_type == "text/css; charset=utf-8");
        assert(resp.body.find(".system-layout") != std::string::npos);
    }

    // Readiness and completeness are orthogonal machine-readable states.
    {
        const auto resp = app.handle("GET", "/api/status");
        assert(resp.status == 200);
        assert(resp.body.find("\"operational_status\":\"ready\"") != std::string::npos);
        assert(resp.body.find("\"complete\":false") != std::string::npos);
        assert(resp.body.find("\"evolution_status\":\"incomplete\"") != std::string::npos);
        assert(resp.body.find("\"completion_policy\":\"continuous_evolution\"") != std::string::npos);
    }

    // Test GET /health
    {
        const auto resp = app.handle("GET", "/health");
        assert(resp.status == 200);
        assert(resp.body.find("\"system_id\":\"sister_atmos\"") != std::string::npos);
        assert(resp.body.find("\"status\":\"ok\"") != std::string::npos);
    }

    // Test GET /api/health
    {
        const auto resp = app.handle("GET", "/api/health");
        assert(resp.status == 200);
        assert(resp.body.find("\"system_id\":\"sister_atmos\"") != std::string::npos);
    }

    // Test GET /_sister/health
    {
        const auto resp = app.handle("GET", "/_sister/health");
        assert(resp.status == 200);
        assert(resp.body.find("\"system_id\":\"sister_atmos\"") != std::string::npos);
    }

    // Test GET /_sister/ready
    {
        const auto resp = app.handle("GET", "/_sister/ready");
        assert(resp.status == 200);
        assert(resp.body.find("\"system_id\":\"sister_atmos\"") != std::string::npos);
        assert(resp.body.find("\"status\":\"ready\"") != std::string::npos);
        assert(resp.body.find("\"scope\":\"current_declared_capabilities\"") != std::string::npos);
        assert(resp.body.find("\"declared_capability\":\"governed_precipitation_explorer\"") != std::string::npos);
    }

    // Operational adapter must not publish a second OpenAPI authority.
    {
        const auto resp = app.handle("GET", "/openapi.yaml");
        assert(resp.status == 404);
    }

    // Test 404 for unknown route
    {
        const auto resp = app.handle("GET", "/unknown/route");
        assert(resp.status == 404);
        assert(resp.body.find("not_found") != std::string::npos);
    }

    // Test 405 for unsupported method
    {
        const auto resp = app.handle("DELETE", "/health");
        assert(resp.status == 405);
        assert(resp.body.find("method_not_allowed") != std::string::npos);
    }

    // Governed location search and precipitation analysis use an injectable provider.
    {
        sister::atmos::http::Application climate_app{std::make_shared<FakeClimateProvider>()};
        const auto locations = climate_app.handle("GET", "/api/locations?q=Pelotas");
        assert(locations.status == 200);
        assert(locations.body.find("\"name\":\"Pelotas\"") != std::string::npos);

        const auto territory = climate_app.handle("GET", "/api/territories/municipality?name=Pelotas&admin1=Rio+Grande+do+Sul");
        assert(territory.status == 200);
        assert(territory.body.find("\"code\":\"4314407\"") != std::string::npos);

        const auto analysis = climate_app.handle(
            "POST", "/api/analyses/precipitation",
            R"({"latitude":-31.77,"longitude":-52.34,"start_date":"2026-08-01","end_date":"2026-08-01","requested_product":"era5"})"
        );
        assert(analysis.status == 200);
        assert(analysis.body.find("\"total_mm\":12.50") != std::string::npos);
        assert(analysis.body.find("\"provider\":\"test-provider\"") != std::string::npos);

        const auto spatial = climate_app.handle(
            "POST", "/api/analyses/precipitation/spatial",
            R"({"latitude":-31.77,"longitude":-52.34,"start_date":"2026-08-01","end_date":"2026-08-01","requested_product":"best_match","municipality_name":"Pelotas","admin1":"Rio Grande do Sul"})"
        );
        assert(spatial.status == 200);
        assert(spatial.body.find("\"analysis_mode\":\"operational_sampling\"") != std::string::npos);
        assert(spatial.body.find("\"native_surface_claimed\":false") != std::string::npos);
    }

    // Invalid analysis requests are rejected before provider access.
    {
        const auto resp = app.handle("POST", "/api/analyses/precipitation", "{}");
        assert(resp.status == 400);
        assert(resp.body.find("invalid_request") != std::string::npos);
    }

    // Test serialization
    {
        const auto resp = app.handle("GET", "/health");
        const auto raw = sister::atmos::http::serialize_response(resp);
        assert(raw.find("HTTP/1.1 200 OK\r\n") == 0);
        assert(raw.find("Content-Type: application/json; charset=utf-8\r\n") != std::string::npos);
        assert(raw.find("X-Content-Type-Options: nosniff\r\n") != std::string::npos);
    }

    std::cout << "[PASS] sister_atmos_http_tests\n";
    return 0;
}
