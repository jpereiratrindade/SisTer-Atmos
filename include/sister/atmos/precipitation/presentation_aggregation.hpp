#pragma once

#include "sister/atmos/precipitation/domain_types.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string_view>

namespace sister::atmos::precipitation {

enum class PresentationAggregationError : std::uint8_t {
    empty_contribution_set,
    mixed_presentation_buckets,
    non_finite_mean,
};

[[nodiscard]]
constexpr std::string_view
to_string(PresentationAggregationError error) noexcept {
    switch (error) {
    case PresentationAggregationError::empty_contribution_set:
        return "empty_contribution_set";
    case PresentationAggregationError::mixed_presentation_buckets:
        return "mixed_presentation_buckets";
    case PresentationAggregationError::non_finite_mean:
        return "non_finite_mean";
    }

    return "unknown_presentation_aggregation_error";
}

struct PresentationBucketId final {
    std::uint64_t opaque_value;

    friend bool operator==(
        const PresentationBucketId&,
        const PresentationBucketId&
    ) = default;
};

struct PresentationContribution final {
    PresentationBucketId bucket;
    PrecipitationMm precipitation;
};

enum class PresentationAggregation : std::uint8_t {
    spatial_mean,
};

struct PresentationBucketSummary final {
    PresentationBucketId bucket;
    PrecipitationMm precipitation;
    std::size_t monitored_points;
    PresentationAggregation aggregation;
};

[[nodiscard]]
std::expected<
    PresentationBucketSummary,
    PresentationAggregationError
>
aggregate_presentation_bucket(
    std::span<const PresentationContribution> contributions
) noexcept;

}  // namespace sister::atmos::precipitation
