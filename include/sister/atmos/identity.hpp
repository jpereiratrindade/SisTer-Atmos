#pragma once

#include <string_view>

namespace sister::atmos {

struct Identity {
    std::string_view name;
    std::string_view system_id;
    std::string_view role;
    std::string_view domain;
    std::string_view language;
};

[[nodiscard]] constexpr Identity identity() noexcept {
    return {
        .name = "SisTer Atmos",
        .system_id = "sister_atmos",
        .role = "subsystem",
        .domain = "climate_intelligence",
        .language = "C++23",
    };
}

}  // namespace sister::atmos
