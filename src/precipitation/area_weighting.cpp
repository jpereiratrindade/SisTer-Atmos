#include "sister/atmos/precipitation/area_weighting.hpp"

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
    AreaWeightedPrecipitation,
    AreaWeightingError
>
area_weighted_precipitation(
    std::span<const AreaContribution> contributions
) noexcept {
    if (contributions.empty()) {
        return std::unexpected(
            AreaWeightingError::empty_contribution_set
        );
    }

    constexpr long double max_double =
        static_cast<long double>(
            std::numeric_limits<double>::max()
        );

    CompensatedSum area_sum;

    for (const auto& contribution : contributions) {
        const long double area =
            static_cast<long double>(
                contribution.represented_area.value()
            );

        area_sum.add(area);

        const long double current_area = area_sum.value();

        if (
            !std::isfinite(current_area)
            || current_area > max_double
        ) {
            return std::unexpected(
                AreaWeightingError::
                    represented_area_overflow
            );
        }
    }

    const long double total_area = area_sum.value();

    if (
        !std::isfinite(total_area)
        || total_area <= 0.0L
        || total_area > max_double
    ) {
        return std::unexpected(
            AreaWeightingError::represented_area_overflow
        );
    }

    CompensatedSum weighted_sum;

    for (const auto& contribution : contributions) {
        const long double precipitation =
            static_cast<long double>(
                contribution.precipitation.value()
            );

        const long double area =
            static_cast<long double>(
                contribution.represented_area.value()
            );

        const long double normalized_weight =
            area / total_area;

        const long double weighted_value =
            precipitation * normalized_weight;

        if (!std::isfinite(weighted_value)) {
            return std::unexpected(
                AreaWeightingError::
                    non_finite_weighted_result
            );
        }

        weighted_sum.add(weighted_value);
    }

    const long double weighted_result =
        weighted_sum.value();

    if (
        !std::isfinite(weighted_result)
        || weighted_result < 0.0L
        || weighted_result > max_double
    ) {
        return std::unexpected(
            AreaWeightingError::
                non_finite_weighted_result
        );
    }

    const auto precipitation =
        PrecipitationMm::create(
            static_cast<double>(weighted_result)
        );

    if (!precipitation) {
        return std::unexpected(
            AreaWeightingError::
                non_finite_weighted_result
        );
    }

    const auto represented_area =
        RepresentedArea::create(
            static_cast<double>(total_area)
        );

    if (!represented_area) {
        return std::unexpected(
            AreaWeightingError::
                represented_area_overflow
        );
    }

    return AreaWeightedPrecipitation{
        .precipitation = *precipitation,
        .represented_area = *represented_area,
        .contributors = contributions.size(),
    };
}

}  // namespace sister::atmos::precipitation
