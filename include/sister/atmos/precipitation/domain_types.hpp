#pragma once

#include <cstdint>
#include <expected>
#include <string_view>

namespace sister::atmos::precipitation {

enum class DomainError : std::uint8_t {
    non_finite_latitude,
    latitude_out_of_range,
    non_finite_longitude,
    longitude_out_of_range,
    non_finite_precipitation,
    negative_precipitation,
    non_finite_represented_area,
    non_positive_represented_area,
};

[[nodiscard]]
constexpr std::string_view to_string(DomainError error) noexcept {
    switch (error) {
    case DomainError::non_finite_latitude:
        return "non_finite_latitude";
    case DomainError::latitude_out_of_range:
        return "latitude_out_of_range";
    case DomainError::non_finite_longitude:
        return "non_finite_longitude";
    case DomainError::longitude_out_of_range:
        return "longitude_out_of_range";
    case DomainError::non_finite_precipitation:
        return "non_finite_precipitation";
    case DomainError::negative_precipitation:
        return "negative_precipitation";
    case DomainError::non_finite_represented_area:
        return "non_finite_represented_area";
    case DomainError::non_positive_represented_area:
        return "non_positive_represented_area";
    }

    return "unknown_domain_error";
}

enum class MeteorologicalProduct : std::uint8_t {
    era5,
    best_match_dynamic,
    nasa_power_merra2,
};

class Latitude final {
public:
    [[nodiscard]]
    static std::expected<Latitude, DomainError>
    create(double value) noexcept;

    [[nodiscard]]
    constexpr double value() const noexcept {
        return value_;
    }

    friend bool operator==(const Latitude&, const Latitude&) = default;

private:
    explicit constexpr Latitude(double value) noexcept
        : value_(value) {}

    double value_;
};

class Longitude final {
public:
    [[nodiscard]]
    static std::expected<Longitude, DomainError>
    create(double value) noexcept;

    [[nodiscard]]
    constexpr double value() const noexcept {
        return value_;
    }

    friend bool operator==(const Longitude&, const Longitude&) = default;

private:
    explicit constexpr Longitude(double value) noexcept
        : value_(value) {}

    double value_;
};

class PrecipitationMm final {
public:
    [[nodiscard]]
    static std::expected<PrecipitationMm, DomainError>
    create(double value) noexcept;

    [[nodiscard]]
    constexpr double value() const noexcept {
        return value_;
    }

    friend bool operator==(
        const PrecipitationMm&,
        const PrecipitationMm&
    ) = default;

private:
    explicit constexpr PrecipitationMm(double value) noexcept
        : value_(value) {}

    double value_;
};

class RepresentedArea final {
public:
    [[nodiscard]]
    static std::expected<RepresentedArea, DomainError>
    create(double value) noexcept;

    [[nodiscard]]
    constexpr double value() const noexcept {
        return value_;
    }

    friend bool operator==(
        const RepresentedArea&,
        const RepresentedArea&
    ) = default;

private:
    explicit constexpr RepresentedArea(double value) noexcept
        : value_(value) {}

    double value_;
};

struct NativeCellKey final {
    MeteorologicalProduct product;
    Latitude latitude;
    Longitude longitude;

    friend bool operator==(
        const NativeCellKey&,
        const NativeCellKey&
    ) = default;
};

}  // namespace sister::atmos::precipitation
