// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SISTER_ATMOS_HTTP_HPP
#define SISTER_ATMOS_HTTP_HPP

#include "sister/atmos/climate_service.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace sister::atmos::http {

struct Response {
    int status{200};
    std::string content_type{"application/json; charset=utf-8"};
    std::string body{};
};

class Application {
public:
    Application();
    explicit Application(std::shared_ptr<const ClimateProvider> provider) noexcept;

    [[nodiscard]] Response handle(std::string_view method, std::string_view path, std::string_view body = {}) const;

private:
    std::shared_ptr<const ClimateProvider> provider_;
};

[[nodiscard]] std::string reason_phrase(int status);
[[nodiscard]] std::string serialize_response(const Response& response);

} // namespace sister::atmos::http

#endif // SISTER_ATMOS_HTTP_HPP
