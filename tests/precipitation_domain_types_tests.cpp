#include "sister/atmos/precipitation/domain_types.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>

namespace p = sister::atmos::precipitation;

namespace {

bool expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "[FAIL] " << message << '\n';
        return false;
    }

    return true;
}

template <typename T>
bool expect_error(
    const std::expected<T, p::DomainError>& result,
    p::DomainError expected,
    const char* message
) {
    return expect(
        !result && result.error() == expected,
        message
    );
}

}  // namespace

int main() {
    bool ok = true;

    const auto lat_min = p::Latitude::create(-90.0);
    const auto lat_max = p::Latitude::create(90.0);

    ok &= expect(lat_min.has_value(), "latitude -90 válida");
    ok &= expect(lat_max.has_value(), "latitude 90 válida");

    ok &= expect_error(
        p::Latitude::create(90.0001),
        p::DomainError::latitude_out_of_range,
        "latitude > 90 rejeitada"
    );

    ok &= expect_error(
        p::Latitude::create(
            std::numeric_limits<double>::quiet_NaN()
        ),
        p::DomainError::non_finite_latitude,
        "latitude NaN rejeitada"
    );

    const auto lon_min = p::Longitude::create(-180.0);
    const auto lon_max = p::Longitude::create(180.0);

    ok &= expect(lon_min.has_value(), "longitude -180 válida");
    ok &= expect(lon_max.has_value(), "longitude 180 válida");

    ok &= expect_error(
        p::Longitude::create(180.0001),
        p::DomainError::longitude_out_of_range,
        "longitude > 180 rejeitada"
    );

    ok &= expect_error(
        p::Longitude::create(
            std::numeric_limits<double>::infinity()
        ),
        p::DomainError::non_finite_longitude,
        "longitude infinita rejeitada"
    );

    const auto zero_rain = p::PrecipitationMm::create(0.0);

    ok &= expect(
        zero_rain.has_value(),
        "precipitação zero válida"
    );

    ok &= expect_error(
        p::PrecipitationMm::create(-0.1),
        p::DomainError::negative_precipitation,
        "precipitação negativa rejeitada"
    );

    ok &= expect_error(
        p::PrecipitationMm::create(
            std::numeric_limits<double>::infinity()
        ),
        p::DomainError::non_finite_precipitation,
        "precipitação infinita rejeitada"
    );

    const auto area = p::RepresentedArea::create(1.5);

    ok &= expect(area.has_value(), "área positiva válida");

    ok &= expect_error(
        p::RepresentedArea::create(0.0),
        p::DomainError::non_positive_represented_area,
        "área zero rejeitada"
    );

    ok &= expect_error(
        p::RepresentedArea::create(-1.0),
        p::DomainError::non_positive_represented_area,
        "área negativa rejeitada"
    );

    ok &= expect_error(
        p::RepresentedArea::create(
            std::numeric_limits<double>::quiet_NaN()
        ),
        p::DomainError::non_finite_represented_area,
        "área NaN rejeitada"
    );

    if (
        lat_min
        && lon_min
        && zero_rain
    ) {
        const p::NativeCellKey first{
            .product = p::MeteorologicalProduct::era5,
            .latitude = *lat_min,
            .longitude = *lon_min,
        };

        const p::NativeCellKey second{
            .product = p::MeteorologicalProduct::era5,
            .latitude = *lat_min,
            .longitude = *lon_min,
        };

        ok &= expect(
            first == second,
            "identidade de célula nativa é determinística"
        );
    }

    ok &= expect(
        p::to_string(
            p::DomainError::negative_precipitation
        ) == "negative_precipitation",
        "erro possui identidade textual estável"
    );

    if (ok) {
        std::cout
            << "[PASS] tipos científicos de precipitação\n";
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
