#include "sister/atmos/precipitation/surface_eligibility.hpp"

#include <cstdlib>
#include <iostream>

namespace p = sister::atmos::precipitation;

namespace {

bool expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "[FAIL] " << message << '\n';
        return false;
    }

    return true;
}

}  // namespace

int main() {
    bool ok = true;

    // GC-005 — NASA POWER sem geometria nativa conhecida
    // não pode reivindicar superfície meteorológica H3.
    {
        const auto result =
            p::validate_spatial_representation({
                .product =
                    p::MeteorologicalProduct::nasa_power_merra2,
                .native_grid_geometry_available = false,
                .requested_representation =
                    p::SpatialRepresentation::h3_surface,
            });

        ok &= expect(
            !result
                && result.error()
                    == p::SurfaceEligibilityError::
                        native_geometry_required,
            "GC-005 rejeita superfície sem geometria nativa"
        );
    }

    // O produto pontual continua admissível como pontos.
    // A regra proíbe fabricar superfície, não usar o produto.
    {
        const auto result =
            p::validate_spatial_representation({
                .product =
                    p::MeteorologicalProduct::nasa_power_merra2,
                .native_grid_geometry_available = false,
                .requested_representation =
                    p::SpatialRepresentation::point_samples,
            });

        ok &= expect(
            result.has_value(),
            "produto pontual permanece admissível como pontos"
        );

        if (result) {
            ok &= expect(
                result->representation
                    == p::SpatialRepresentation::point_samples,
                "representação pontual preservada"
            );

            ok &= expect(
                !result->native_grid_geometry_available,
                "ausência de geometria nativa permanece explícita"
            );
        }
    }

    // Uma superfície pode ser admitida quando a geometria
    // meteorológica nativa está explicitamente disponível.
    {
        const auto result =
            p::validate_spatial_representation({
                .product = p::MeteorologicalProduct::era5,
                .native_grid_geometry_available = true,
                .requested_representation =
                    p::SpatialRepresentation::h3_surface,
            });

        ok &= expect(
            result.has_value(),
            "superfície com geometria nativa é admissível"
        );

        if (result) {
            ok &= expect(
                result->representation
                    == p::SpatialRepresentation::h3_surface,
                "pedido de superfície preservado"
            );

            ok &= expect(
                result->product
                    == p::MeteorologicalProduct::era5,
                "produto permanece rastreável"
            );
        }
    }

    ok &= expect(
        p::to_string(
            p::SurfaceEligibilityError::
                native_geometry_required
        ) == "native_geometry_required",
        "erro científico possui identidade textual estável"
    );

    if (ok) {
        std::cout
            << "[PASS] GC-005 surface eligibility\n";
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
