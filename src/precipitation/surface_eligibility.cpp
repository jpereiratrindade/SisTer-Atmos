#include "sister/atmos/precipitation/surface_eligibility.hpp"

#include <expected>

namespace sister::atmos::precipitation {

std::expected<
    EligibleRepresentation,
    SurfaceEligibilityError
>
validate_spatial_representation(
    SurfaceRequest request
) noexcept {
    if (
        request.requested_representation
            == SpatialRepresentation::h3_surface
        && !request.native_grid_geometry_available
    ) {
        return std::unexpected(
            SurfaceEligibilityError::native_geometry_required
        );
    }

    return EligibleRepresentation{
        .product = request.product,
        .native_grid_geometry_available =
            request.native_grid_geometry_available,
        .representation =
            request.requested_representation,
    };
}

}  // namespace sister::atmos::precipitation
