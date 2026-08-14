#include "sister/atmos/precipitation/presentation_aggregation.hpp"

#include <cmath>
#include <expected>
#include <limits>

namespace sister::atmos::precipitation {

namespace {

class CompensatedSum final {
public:
    void add(long double value) noexcept {
        const long double next = sum_ + value;

        if (std::fabs(sum_) >= std::fabs(value)) {
            correction_ += (sum_ - next) + value;
        } else {
            correction_ += (value - next) + sum_;
        }

        sum_ = next;
    }

    [[nodiscard]]
    long double value() const noexcept {
        return sum_ + correction_;
    }

private:
    long double sum_{0.0L};
    long double correction_{0.0L};
};

}  // namespace

std::expected<
    PresentationBucketSummary,
    PresentationAggregationError
>
aggregate_presentation_bucket(
    std::span<const PresentationContribution> contributions
) noexcept {
    if (contributions.empty()) {
        return std::unexpected(
            PresentationAggregationError::empty_contribution_set
        );
    }

    const PresentationBucketId bucket = contributions.front().bucket;
    CompensatedSum precipitation_sum;

    for (const auto& contribution : contributions) {
        if (contribution.bucket != bucket) {
            return std::unexpected(
                PresentationAggregationError::mixed_presentation_buckets
            );
        }

        precipitation_sum.add(
            static_cast<long double>(
                contribution.precipitation.value()
            )
        );
    }

    const long double mean =
        precipitation_sum.value()
        / static_cast<long double>(contributions.size());

    constexpr long double max_double =
        static_cast<long double>(
            std::numeric_limits<double>::max()
        );

    if (
        !std::isfinite(mean)
        || mean < 0.0L
        || mean > max_double
    ) {
        return std::unexpected(
            PresentationAggregationError::non_finite_mean
        );
    }

    const auto precipitation =
        PrecipitationMm::create(
            static_cast<double>(mean)
        );

    if (!precipitation) {
        return std::unexpected(
            PresentationAggregationError::non_finite_mean
        );
    }

    return PresentationBucketSummary{
        .bucket = bucket,
        .precipitation = *precipitation,
        .monitored_points = contributions.size(),
        .aggregation = PresentationAggregation::spatial_mean,
    };
}

}  // namespace sister::atmos::precipitation
