#pragma once

#include "sister/atmos/precipitation/domain_types.hpp"

#include <cstdint>
#include <expected>
#include <string_view>

namespace sister::atmos::precipitation {

enum class AnalysisMode : std::uint8_t {
    consolidated_precipitation,
};

enum class ProductSelectionSemantics : std::uint8_t {
    direct_product_selection,
    fallback,
};

enum class ProductSelectionReason : std::uint8_t {
    era5_land_precipitation_unavailable,
};

enum class ProductSelectionError : std::uint8_t {
    unsupported_analysis_mode,
    ungoverned_requested_product,
    ungoverned_era5_land_precipitation_path,
};

[[nodiscard]]
constexpr std::string_view
to_string(ProductSelectionSemantics semantics) noexcept {
    switch (semantics) {
    case ProductSelectionSemantics::direct_product_selection:
        return "direct_product_selection";
    case ProductSelectionSemantics::fallback:
        return "fallback";
    }

    return "unknown_product_selection_semantics";
}

[[nodiscard]]
constexpr std::string_view
to_string(ProductSelectionError error) noexcept {
    switch (error) {
    case ProductSelectionError::unsupported_analysis_mode:
        return "unsupported_analysis_mode";
    case ProductSelectionError::
        ungoverned_requested_product:
        return "ungoverned_requested_product";
    case ProductSelectionError::
        ungoverned_era5_land_precipitation_path:
        return "ungoverned_era5_land_precipitation_path";
    }

    return "unknown_product_selection_error";
}

[[nodiscard]]
constexpr std::string_view
selection_reason_text(
    ProductSelectionReason reason
) noexcept {
    switch (reason) {
    case ProductSelectionReason::
        era5_land_precipitation_unavailable:
        return
            "ERA5-Land não disponibiliza precipitação";
    }

    return "razão de seleção desconhecida";
}

struct ProductSelectionRequest final {
    MeteorologicalProduct requested_product;
    AnalysisMode analysis_mode;
    bool era5_land_precipitation_available;
};

struct ProductSelection final {
    MeteorologicalProduct requested_product;
    MeteorologicalProduct used_product;
    ProductSelectionSemantics semantics;
    bool fallback_used;
    ProductSelectionReason reason;
};

[[nodiscard]]
std::expected<
    ProductSelection,
    ProductSelectionError
>
select_product(
    ProductSelectionRequest request
) noexcept;

}  // namespace sister::atmos::precipitation
