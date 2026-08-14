#include "sister/atmos/precipitation/native_samples.hpp"

#include <expected>
#include <map>
#include <utility>
#include <vector>

namespace sister::atmos::precipitation {

namespace {

struct NativeCellKeyLess final {
    [[nodiscard]]
    bool operator()(
        const NativeCellKey& lhs,
        const NativeCellKey& rhs
    ) const noexcept {
        const auto lhs_product = std::to_underlying(lhs.product);
        const auto rhs_product = std::to_underlying(rhs.product);

        if (lhs_product != rhs_product) {
            return lhs_product < rhs_product;
        }

        if (lhs.latitude.value() != rhs.latitude.value()) {
            return lhs.latitude.value() < rhs.latitude.value();
        }

        return lhs.longitude.value() < rhs.longitude.value();
    }
};

}  // namespace

std::expected<
    std::vector<UniqueNativeObservation>,
    NativeSampleError
>
deduplicate_native_samples(
    std::span<const NativeSample> samples
) {
    if (samples.empty()) {
        return std::unexpected(
            NativeSampleError::empty_sample_set
        );
    }

    std::map<
        NativeCellKey,
        UniqueNativeObservation,
        NativeCellKeyLess
    > unique;

    for (const auto& sample : samples) {
        const auto found = unique.find(sample.cell);

        if (found == unique.end()) {
            unique.emplace(
                sample.cell,
                UniqueNativeObservation{
                    .cell = sample.cell,
                    .precipitation = sample.precipitation,
                    .sample_count = 1,
                }
            );

            continue;
        }

        if (found->second.precipitation != sample.precipitation) {
            return std::unexpected(
                NativeSampleError::
                    conflicting_native_cell_values
            );
        }

        ++found->second.sample_count;
    }

    std::vector<UniqueNativeObservation> result;
    result.reserve(unique.size());

    for (const auto& [cell, observation] : unique) {
        static_cast<void>(cell);
        result.push_back(observation);
    }

    return result;
}

}  // namespace sister::atmos::precipitation
