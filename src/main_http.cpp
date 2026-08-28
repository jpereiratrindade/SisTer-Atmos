// SPDX-License-Identifier: GPL-3.0-or-later
#include "sister/atmos/http_server.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

sister::atmos::http::ServerOptions parse_options(int argc, char** argv) {
    sister::atmos::http::ServerOptions options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg{argv[i]};
        if (arg == "--bind" && i + 1 < argc) {
            options.bind_address = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            const auto val = std::stoul(argv[++i]);
            if (val == 0 || val > 65535) {
                throw std::invalid_argument("Porta fora do intervalo permitido (1-65535)");
            }
            options.port = static_cast<std::uint16_t>(val);
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Uso: sister-atmos-http [--bind 127.0.0.1] [--port 8095]\n";
            std::exit(0);
        }
    }
    return options;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        sister::atmos::http::Server server{options};
        return server.run();
    } catch (const std::exception& e) {
        std::cerr << "[SisTer-Atmos] Erro fatal: " << e.what() << '\n';
        return 1;
    }
}
