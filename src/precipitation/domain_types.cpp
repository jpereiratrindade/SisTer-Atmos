#include "sister/atmos/precipitation/domain_types.hpp"

#include <cmath>
#include <expected>

namespace sister::atmos::precipitation {

std::expected<Latitude, DomainError>
Latitude::create(double value) noexcept {
    if (!std::isfinite(value)) {
        return std::unexpected(DomainError::non_finite_latitude);
    }

    if (value < -90.0 || value > 90.0) {
        return std::unexpected(DomainError::latitude_out_of_range);
    }

    return Latitude{value};
}

std::expected<Longitude, DomainError>
Longitude::create(double value) noexcept {
    if (!std::isfinite(value)) {
        return std::unexpected(DomainError::non_finite_longitude);
    }

    if (value < -180.0 || value > 180.0) {
        return std::unexpected(DomainError::longitude_out_of_range);
    }

    return Longitude{value};
}

std::expected<PrecipitationMm, DomainError>
PrecipitationMm::create(double value) noexcept {
    if (!std::isfinite(value)) {
        return std::unexpected(
            DomainError::non_finite_precipitation
        );
    }

    if (value < 0.0) {
        return std::unexpected(DomainError::negative_precipitation);
    }

    return PrecipitationMm{value};
}

std::expected<RepresentedArea, DomainError>
RepresentedArea::create(double value) noexcept {
    if (!std::isfinite(value)) {
        return std::unexpected(
            DomainError::non_finite_represented_area
        );
    }

    if (value <= 0.0) {
        return std::unexpected(
            DomainError::non_positive_represented_area
        );
    }

    return RepresentedArea{value};
}

}  // namespace sister::atmos::precipitation
