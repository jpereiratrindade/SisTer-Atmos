#pragma once

#include "sister/atmos/precipitation/native_samples.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string_view>

namespace sister::atmos::precipitation {

enum class OperationalSamplingError : std::uint8_t {
    empty_observation_set,
    non_operational_product,
    non_finite_mean,
};

[[nodiscard]]
constexpr std::string_view
to_string(OperationalSamplingError error) noexcept {
    switch (error) {
    case OperationalSamplingError::empty_observation_set:
        return "empty_observation_set";
    case OperationalSamplingError::non_operational_product:
        return "non_operational_product";
    case OperationalSamplingError::non_finite_mean:
        return "non_finite_mean";
    }

    return "unknown_operational_sampling_error";
}

enum class OperationalAggregation : std::uint8_t {
    unweighted_mean_unique_sample_points,
};

enum class OperationalSurfaceKind : std::uint8_t {
    operational_sampling_not_native_pixel_surface,
};

struct OperationalSamplingSummary final {
    PrecipitationMm precipitation;
    std::size_t unique_sample_points;
    OperationalAggregation aggregation;
    OperationalSurfaceKind surface_kind;
};

[[nodiscard]]
std::expected<
    OperationalSamplingSummary,
    OperationalSamplingError
>
operational_sampling_mean(
    std::span<const UniqueNativeObservation> observations
) noexcept;

}  // namespace sister::atmos::precipitation
