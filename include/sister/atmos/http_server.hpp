// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SISTER_ATMOS_HTTP_SERVER_HPP
#define SISTER_ATMOS_HTTP_SERVER_HPP

#include "sister/atmos/http.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace sister::atmos::http {

struct ServerOptions {
    std::string bind_address{"127.0.0.1"};
    std::uint16_t port{8095};
};

class Server {
public:
    explicit Server(ServerOptions options) noexcept;

    int run();

private:
    ServerOptions options_;
    Application app_{};
};

} // namespace sister::atmos::http

#endif // SISTER_ATMOS_HTTP_SERVER_HPP
