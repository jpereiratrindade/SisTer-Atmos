#include "sister/atmos/precipitation/operational_sampling.hpp"

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
    OperationalSamplingSummary,
    OperationalSamplingError
>
operational_sampling_mean(
    std::span<const UniqueNativeObservation> observations
) noexcept {
    if (observations.empty()) {
        return std::unexpected(
            OperationalSamplingError::empty_observation_set
        );
    }

    CompensatedSum precipitation_sum;

    for (const auto& observation : observations) {
        if (
            observation.cell.product
            != MeteorologicalProduct::best_match_dynamic
        ) {
            return std::unexpected(
                OperationalSamplingError::non_operational_product
            );
        }

        precipitation_sum.add(
            static_cast<long double>(
                observation.precipitation.value()
            )
        );
    }

    const long double mean =
        precipitation_sum.value()
        / static_cast<long double>(observations.size());

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
            OperationalSamplingError::non_finite_mean
        );
    }

    const auto precipitation =
        PrecipitationMm::create(
            static_cast<double>(mean)
        );

    if (!precipitation) {
        return std::unexpected(
            OperationalSamplingError::non_finite_mean
        );
    }

    return OperationalSamplingSummary{
        .precipitation = *precipitation,
        .unique_sample_points = observations.size(),
        .aggregation =
            OperationalAggregation::
                unweighted_mean_unique_sample_points,
        .surface_kind =
            OperationalSurfaceKind::
                operational_sampling_not_native_pixel_surface,
    };
}

}  // namespace sister::atmos::precipitation
