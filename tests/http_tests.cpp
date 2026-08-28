// SPDX-License-Identifier: GPL-3.0-or-later
#include "sister/atmos/http.hpp"

#include <cassert>
#include <iostream>
#include <string_view>

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
    }

    // Presentation assets are served by the same runtime; no external frontend is required.
    {
        const auto resp = app.handle("GET", "/assets/atmos.css");
        assert(resp.status == 200);
        assert(resp.content_type == "text/css; charset=utf-8");
        assert(resp.body.find(".shell") != std::string::npos);
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
