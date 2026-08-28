// SPDX-License-Identifier: GPL-3.0-or-later
#include "sister/atmos/http.hpp"

#include <sstream>
#include <utility>

namespace sister::atmos::http {
namespace {

Response health_response() {
    return Response{
        .status = 200,
        .content_type = "application/json; charset=utf-8",
        .body = R"({"system_id":"sister_atmos","status":"ok","version":"0.1.0"})",
    };
}

Response ready_response() {
    return Response{
        .status = 200,
        .content_type = "application/json; charset=utf-8",
        .body = R"({"system_id":"sister_atmos","status":"ready","version":"0.1.0"})",
    };
}

} // namespace

Response Application::handle(const std::string_view method, const std::string_view path, [[maybe_unused]] const std::string_view req_body) const {
    if (method != "GET" && method != "HEAD") {
        return Response{
            .status = 405,
            .content_type = "application/json; charset=utf-8",
            .body = R"({"error":"method_not_allowed"})",
        };
    }

    if (path == "/" || path == "/index.html") {
        return Response{
            .status = 200,
            .content_type = "application/json; charset=utf-8",
            .body = R"({"service":"sister-atmos","role":"climate_intelligence","status":"online"})",
        };
    }

    if (path == "/health" || path == "/api/health" || path == "/_sister/health") {
        return health_response();
    }

    if (path == "/_sister/ready") {
        return ready_response();
    }

    return Response{
        .status = 404,
        .content_type = "application/json; charset=utf-8",
        .body = R"({"error":"not_found"})",
    };
}

std::string reason_phrase(const int status) {
    switch (status) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        default: return "Error";
    }
}

std::string serialize_response(const Response& response) {
    std::ostringstream out;
    out << "HTTP/1.1 " << response.status << ' ' << reason_phrase(response.status) << "\r\n"
        << "Content-Type: " << response.content_type << "\r\n"
        << "Content-Length: " << response.body.size() << "\r\n"
        << "Connection: close\r\n"
        << "Cache-Control: no-store\r\n"
        << "X-Content-Type-Options: nosniff\r\n"
        << "Referrer-Policy: no-referrer\r\n"
        << "Content-Security-Policy: default-src 'self'; frame-ancestors 'none'\r\n"
        << "\r\n"
        << response.body;
    return out.str();
}

} // namespace sister::atmos::http
