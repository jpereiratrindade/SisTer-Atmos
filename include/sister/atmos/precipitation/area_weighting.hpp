#pragma once

#include "sister/atmos/precipitation/domain_types.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string_view>

namespace sister::atmos::precipitation {

enum class AreaWeightingError : std::uint8_t {
    empty_contribution_set,
    represented_area_overflow,
    non_finite_weighted_result,
};

[[nodiscard]]
constexpr std::string_view
to_string(AreaWeightingError error) noexcept {
    switch (error) {
    case AreaWeightingError::empty_contribution_set:
        return "empty_contribution_set";
    case AreaWeightingError::represented_area_overflow:
        return "represented_area_overflow";
    case AreaWeightingError::non_finite_weighted_result:
        return "non_finite_weighted_result";
    }

    return "unknown_area_weighting_error";
}

struct AreaContribution final {
    PrecipitationMm precipitation;
    RepresentedArea represented_area;
};

struct AreaWeightedPrecipitation final {
    PrecipitationMm precipitation;
    RepresentedArea represented_area;
    std::size_t contributors;
};

[[nodiscard]]
std::expected<
    AreaWeightedPrecipitation,
    AreaWeightingError
>
area_weighted_precipitation(
    std::span<const AreaContribution> contributions
) noexcept;

}  // namespace sister::atmos::precipitation
