#pragma once

#include "sister/atmos/precipitation/domain_types.hpp"

#include <cstdint>
#include <expected>
#include <string_view>

namespace sister::atmos::precipitation {

enum class SpatialRepresentation : std::uint8_t {
    point_samples,
    h3_surface,
};

enum class SurfaceEligibilityError : std::uint8_t {
    native_geometry_required,
};

[[nodiscard]]
constexpr std::string_view
to_string(SurfaceEligibilityError error) noexcept {
    switch (error) {
    case SurfaceEligibilityError::native_geometry_required:
        return "native_geometry_required";
    }

    return "unknown_surface_eligibility_error";
}

struct SurfaceRequest final {
    MeteorologicalProduct product;
    bool native_grid_geometry_available;
    SpatialRepresentation requested_representation;
};

struct EligibleRepresentation final {
    MeteorologicalProduct product;
    bool native_grid_geometry_available;
    SpatialRepresentation representation;
};

[[nodiscard]]
std::expected<
    EligibleRepresentation,
    SurfaceEligibilityError
>
validate_spatial_representation(
    SurfaceRequest request
) noexcept;

}  // namespace sister::atmos::precipitation
