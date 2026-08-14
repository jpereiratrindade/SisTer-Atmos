#pragma once

#include "sister/atmos/precipitation/domain_types.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string_view>
#include <vector>

namespace sister::atmos::precipitation {

enum class NativeSampleError : std::uint8_t {
    empty_sample_set,
    conflicting_native_cell_values,
};

[[nodiscard]]
constexpr std::string_view
to_string(NativeSampleError error) noexcept {
    switch (error) {
    case NativeSampleError::empty_sample_set:
        return "empty_sample_set";
    case NativeSampleError::conflicting_native_cell_values:
        return "conflicting_native_cell_values";
    }

    return "unknown_native_sample_error";
}

struct NativeSample final {
    NativeCellKey cell;
    PrecipitationMm precipitation;
};

struct UniqueNativeObservation final {
    NativeCellKey cell;
    PrecipitationMm precipitation;
    std::size_t sample_count;
};

[[nodiscard]]
std::expected<
    std::vector<UniqueNativeObservation>,
    NativeSampleError
>
deduplicate_native_samples(
    std::span<const NativeSample> samples
);

}  // namespace sister::atmos::precipitation
