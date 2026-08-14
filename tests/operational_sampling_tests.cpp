#include "sister/atmos/precipitation/operational_sampling.hpp"

#include <array>
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

template <typename T>
T require_domain_value(
    std::expected<T, p::DomainError> result,
    const char* message
) {
    if (!result) {
        std::cerr
            << "[FAIL] "
            << message
            << ": "
            << p::to_string(result.error())
            << '\n';

        std::exit(EXIT_FAILURE);
    }

    return *result;
}

p::NativeSample sample(
    p::MeteorologicalProduct product,
    double latitude,
    double longitude,
    double precipitation_mm
) {
    return {
        .cell = {
            .product = product,
            .latitude = require_domain_value(
                p::Latitude::create(latitude),
                "latitude inválida no fixture"
            ),
            .longitude = require_domain_value(
                p::Longitude::create(longitude),
                "longitude inválida no fixture"
            ),
        },
        .precipitation = require_domain_value(
            p::PrecipitationMm::create(precipitation_mm),
            "precipitação inválida no fixture"
        ),
    };
}

}  // namespace

int main() {
    bool ok = true;

    // Conjunto vazio é semanticamente inválido.
    {
        constexpr std::array<
            p::UniqueNativeObservation,
            0
        > observations{};

        const auto result =
            p::operational_sampling_mean(observations);

        ok &= expect(
            !result
                && result.error()
                    == p::OperationalSamplingError::
                        empty_observation_set,
            "conjunto operacional vazio rejeitado"
        );
    }

    // GC-004 — 10 mm e 30 mm em dois pontos Best Match
    // únicos produzem média simples de 20 mm e não
    // constituem superfície meteorológica nativa.
    {
        const std::array samples{
            sample(
                p::MeteorologicalProduct::best_match_dynamic,
                -30.0,
                -51.0,
                10.0
            ),
            sample(
                p::MeteorologicalProduct::best_match_dynamic,
                -30.001,
                -51.001,
                30.0
            ),
        };

        const auto unique =
            p::deduplicate_native_samples(samples);

        ok &= expect(
            unique.has_value(),
            "GC-004 deduplicação aceita"
        );

        if (unique) {
            const auto result =
                p::operational_sampling_mean(*unique);

            ok &= expect(
                result.has_value(),
                "GC-004 síntese operacional aceita"
            );

            if (result) {
                ok &= expect(
                    result->precipitation.value() == 20.0,
                    "GC-004 precipitação média = 20 mm"
                );

                ok &= expect(
                    result->unique_sample_points == 2,
                    "GC-004 possui dois pontos únicos"
                );

                ok &= expect(
                    result->aggregation
                        == p::OperationalAggregation::
                            unweighted_mean_unique_sample_points,
                    "GC-004 usa média simples de pontos únicos"
                );

                ok &= expect(
                    result->surface_kind
                        == p::OperationalSurfaceKind::
                            operational_sampling_not_native_pixel_surface,
                    "GC-004 não reivindica superfície nativa"
                );
            }
        }
    }

    // Duplicação de uma consulta não aumenta seu peso.
    {
        const std::array samples{
            sample(
                p::MeteorologicalProduct::best_match_dynamic,
                -30.0,
                -51.0,
                10.0
            ),
            sample(
                p::MeteorologicalProduct::best_match_dynamic,
                -30.0,
                -51.0,
                10.0
            ),
            sample(
                p::MeteorologicalProduct::best_match_dynamic,
                -30.001,
                -51.001,
                30.0
            ),
        };

        const auto unique =
            p::deduplicate_native_samples(samples);

        ok &= expect(
            unique && unique->size() == 2,
            "duplicata reduzida a dois pontos efetivos"
        );

        if (unique) {
            const auto result =
                p::operational_sampling_mean(*unique);

            ok &= expect(
                result
                    && result->precipitation.value() == 20.0
                    && result->unique_sample_points == 2,
                "sample_count não funciona como peso espacial"
            );
        }
    }

    // Um produto com geometria nativa não deve ser tratado
    // silenciosamente como Best Match operacional.
    {
        const std::array samples{
            sample(
                p::MeteorologicalProduct::era5,
                -30.0,
                -51.0,
                20.0
            ),
        };

        const auto unique =
            p::deduplicate_native_samples(samples);

        if (unique) {
            const auto result =
                p::operational_sampling_mean(*unique);

            ok &= expect(
                !result
                    && result.error()
                        == p::OperationalSamplingError::
                            non_operational_product,
                "produto não operacional rejeitado"
            );
        } else {
            ok &= expect(
                false,
                "fixture ERA5 deveria deduplicar"
            );
        }
    }

    ok &= expect(
        p::to_string(
            p::OperationalSamplingError::
                non_operational_product
        ) == "non_operational_product",
        "erro operacional possui identidade textual estável"
    );

    if (ok) {
        std::cout
            << "[PASS] GC-004 operational sampling mean\n";
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
