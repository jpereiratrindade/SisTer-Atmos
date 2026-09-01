// SPDX-License-Identifier: GPL-3.0-or-later
#include "sister/atmos/http_server.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <stdexcept>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace sister::atmos::http {
namespace {

volatile std::sig_atomic_t g_stop = 0;

void handle_signal(int) {
    g_stop = 1;
}

void write_all(int fd, const std::string& payload) {
    std::size_t written = 0;
    while (written < payload.size()) {
        const auto rc = ::send(fd, payload.data() + written, payload.size() - written, MSG_NOSIGNAL);
        if (rc < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error(std::string{"Erro no send: "} + std::strerror(errno));
        }
        written += static_cast<std::size_t>(rc);
    }
}

void parse_request_line(std::string_view raw, std::string& method, std::string& path, std::string& body) {
    auto line_end = raw.find("\r\n");
    if (line_end == std::string_view::npos) {
        line_end = raw.find('\n');
        if (line_end == std::string_view::npos) return;
    }

    std::string_view req_line = raw.substr(0, line_end);
    auto sp1 = req_line.find(' ');
    if (sp1 == std::string_view::npos) return;
    auto sp2 = req_line.find(' ', sp1 + 1);
    if (sp2 == std::string_view::npos) return;

    method = std::string{req_line.substr(0, sp1)};
    std::string_view full_path = req_line.substr(sp1 + 1, sp2 - sp1 - 1);

    // Preserve the query string for application-level routing and decoding.
    path = std::string{full_path};

    auto body_pos = raw.find("\r\n\r\n");
    if (body_pos != std::string_view::npos) {
        body = std::string{raw.substr(body_pos + 4)};
    } else {
        body_pos = raw.find("\n\n");
        if (body_pos != std::string_view::npos) {
            body = std::string{raw.substr(body_pos + 2)};
        }
    }
}

} // namespace

Server::Server(ServerOptions options) noexcept
    : options_(std::move(options)) {}

int Server::run() {
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    const int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        throw std::runtime_error(std::string{"Falha ao criar socket: "} + std::strerror(errno));
    }

    const int yes = 1;
    if (::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        ::close(server_fd);
        throw std::runtime_error(std::string{"Falha no setsockopt: "} + std::strerror(errno));
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(options_.port);
    if (::inet_pton(AF_INET, options_.bind_address.c_str(), &address.sin_addr) != 1) {
        ::close(server_fd);
        throw std::invalid_argument("Endereço de bind inválido: " + options_.bind_address);
    }

    if (::bind(server_fd, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) < 0) {
        const auto msg = std::string{"Falha no bind: "} + std::strerror(errno);
        ::close(server_fd);
        throw std::runtime_error(msg);
    }

    if (::listen(server_fd, 32) < 0) {
        const auto msg = std::string{"Falha no listen: "} + std::strerror(errno);
        ::close(server_fd);
        throw std::runtime_error(msg);
    }

    std::cout << "[SisTer-Atmos] HTTP Server escutando em http://"
              << options_.bind_address << ':' << options_.port << "/\n";
    std::cout.flush();

    std::vector<std::jthread> workers;
    const auto handle_client = [this](const int client_fd) {
        try {
            std::string raw_request;
            raw_request.resize(32768U);
            const auto received = ::recv(client_fd, raw_request.data(), raw_request.size(), 0);
            if (received > 0) {
                raw_request.resize(static_cast<std::size_t>(received));
                std::string method;
                std::string path;
                std::string body;
                parse_request_line(raw_request, method, path, body);
                const auto resp = app_.handle(method, path, body);
                write_all(client_fd, serialize_response(resp));
            }
        } catch (const std::exception& e) {
            const Response err_resp{
                .status = 500,
                .content_type = "application/json; charset=utf-8",
                .body = R"({"error":"internal_error","message":")" + std::string{e.what()} + R"("})",
            };
            try { write_all(client_fd, serialize_response(err_resp)); }
            catch (...) {}
        }
        ::close(client_fd);
    };

    while (g_stop == 0) {
        pollfd ready{.fd = server_fd, .events = POLLIN, .revents = 0};
        const int poll_rc = ::poll(&ready, 1, 250);
        if (poll_rc < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error(std::string{"Falha no poll: "} + std::strerror(errno));
        }
        if (poll_rc == 0) continue;

        const int client_fd = ::accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error(std::string{"Falha no accept: "} + std::strerror(errno));
        }

        workers.emplace_back(handle_client, client_fd);
    }

    ::close(server_fd);
    std::cout << "\n[SisTer-Atmos] Servidor HTTP finalizado com sucesso.\n";
    return 0;
}

} // namespace sister::atmos::http
