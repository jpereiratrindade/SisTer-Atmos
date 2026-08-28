// SPDX-License-Identifier: GPL-3.0-or-later
#include "sister/atmos/http.hpp"

#include <sstream>
#include <utility>

namespace sister::atmos::http {
namespace {

constexpr std::string_view kHomePage = R"HTML(<!doctype html>
<html lang="pt-BR">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>SisTer Atmos</title>
  <link rel="stylesheet" href="/assets/atmos.css">
</head>
<body>
  <main class="shell">
    <header class="hero">
      <div class="brand">SisTer Atmos</div>
      <div class="badges" aria-label="estado do sistema">
        <span class="badge ready">PRONTO</span>
        <span class="badge evolving">INCOMPLETO</span>
      </div>
      <h1>Inteligência climática e territorial em evolução contínua.</h1>
      <p class="lead">
        O Atmos permanece operacional no escopo que já declara, enquanto novas
        capacidades são incorporadas sem transformar evolução em indisponibilidade.
      </p>
    </header>

    <section class="grid" aria-label="contrato de evolução">
      <article>
        <h2>Pronto</h2>
        <p>
          O runtime está apto a servir as capacidades declaradas neste corte.
          Readiness mede operação presente, não a distância até um produto final.
        </p>
      </article>
      <article>
        <h2>Incompleto</h2>
        <p>
          O sistema assume evolução contínua. Capacidade futura é backlog; não é
          motivo para declarar indisponível aquilo que já foi constituído e verificado.
        </p>
      </article>
    </section>

    <section class="contract">
      <h2>Contrato observável</h2>
      <nav aria-label="endpoints de observabilidade">
        <a href="/api/status">estado do Atmos</a>
        <a href="/_sister/health">health</a>
        <a href="/_sister/ready">readiness</a>
      </nav>
    </section>

    <footer>
      <strong>sister_atmos</strong> · C++23 · climate_intelligence
    </footer>
  </main>
</body>
</html>)HTML";

constexpr std::string_view kStyleSheet = R"CSS(:root {
  color-scheme: light dark;
  font-family: ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
  background: #f4f7f6;
  color: #13201b;
}

* { box-sizing: border-box; }

body {
  margin: 0;
  min-height: 100vh;
  background: linear-gradient(145deg, #edf4f1 0%, #f9fbfa 48%, #eef2f7 100%);
}

.shell {
  width: min(980px, calc(100% - 32px));
  margin: 0 auto;
  padding: 72px 0 36px;
}

.hero,
article,
.contract {
  background: rgba(255, 255, 255, 0.88);
  border: 1px solid rgba(19, 32, 27, 0.11);
  border-radius: 24px;
  box-shadow: 0 16px 50px rgba(19, 32, 27, 0.08);
}

.hero { padding: clamp(28px, 5vw, 58px); }
.brand { font-size: 0.95rem; font-weight: 800; letter-spacing: 0.08em; text-transform: uppercase; }
.badges { display: flex; gap: 8px; margin: 22px 0; flex-wrap: wrap; }
.badge { border-radius: 999px; padding: 7px 11px; font-size: 0.78rem; font-weight: 800; letter-spacing: 0.06em; }
.ready { background: #dff6e8; color: #14532d; }
.evolving { background: #fff1c7; color: #704d00; }
h1 { max-width: 760px; margin: 0; font-size: clamp(2.2rem, 6vw, 4.7rem); line-height: 0.98; letter-spacing: -0.045em; }
.lead { max-width: 760px; margin: 28px 0 0; font-size: clamp(1.05rem, 2vw, 1.3rem); line-height: 1.65; color: #3d5149; }
.grid { display: grid; grid-template-columns: repeat(2, minmax(0, 1fr)); gap: 18px; margin-top: 18px; }
article, .contract { padding: 28px; }
h2 { margin: 0 0 12px; font-size: 1.2rem; }
article p { margin: 0; line-height: 1.65; color: #465950; }
.contract { margin-top: 18px; }
nav { display: flex; gap: 12px; flex-wrap: wrap; }
a { color: #0d5f46; font-weight: 700; text-decoration-thickness: 1px; text-underline-offset: 3px; }
footer { padding: 24px 4px 0; color: #617069; font-size: 0.9rem; }

@media (max-width: 700px) {
  .shell { padding-top: 24px; }
  .grid { grid-template-columns: 1fr; }
}

@media (prefers-color-scheme: dark) {
  :root { background: #0d1512; color: #ecf5f0; }
  body { background: linear-gradient(145deg, #0d1512 0%, #111b17 55%, #10151d 100%); }
  .hero, article, .contract { background: rgba(19, 30, 25, 0.92); border-color: rgba(236, 245, 240, 0.12); }
  .lead, article p { color: #bfd0c7; }
  a { color: #7ad7b7; }
  footer { color: #91a69c; }
  .ready { background: #153c27; color: #a7efc1; }
  .evolving { background: #453814; color: #ffe39a; }
})CSS";

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
        .body = R"({"system_id":"sister_atmos","status":"ready","version":"0.1.0","scope":"current_declared_capabilities"})",
    };
}

Response status_response() {
    return Response{
        .status = 200,
        .content_type = "application/json; charset=utf-8",
        .body = R"({"system_id":"sister_atmos","operational_status":"ready","readiness_scope":"current_declared_capabilities","complete":false,"evolution_status":"incomplete","completion_policy":"continuous_evolution"})",
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
            .content_type = "text/html; charset=utf-8",
            .body = std::string{kHomePage},
        };
    }

    if (path == "/assets/atmos.css") {
        return Response{
            .status = 200,
            .content_type = "text/css; charset=utf-8",
            .body = std::string{kStyleSheet},
        };
    }

    if (path == "/api/status") {
        return status_response();
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
