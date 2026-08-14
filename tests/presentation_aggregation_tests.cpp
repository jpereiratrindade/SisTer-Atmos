#include "sister/atmos/precipitation/presentation_aggregation.hpp"

#include <array>
#include <cstdlib>
#include <iostream>
#include <span>

namespace p = sister::atmos::precipitation;

namespace {

bool expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "[FAIL] " << message << '\n';
        return false;
    }

    return true;
}

p::PrecipitationMm precipitation(double value) {
    const auto result = p::PrecipitationMm::create(value);

    if (!result) {
        std::cerr << "[FAIL] fixture de precipitação inválido\n";
        std::exit(EXIT_FAILURE);
    }

    return *result;
}

}  // namespace

int main() {
    bool ok = true;

    // GC-008 — dois acumulados no mesmo bucket de apresentação
    // são sintetizados por média espacial, nunca por soma espacial.
    {
        constexpr p::PresentationBucketId bucket{
            .opaque_value = 0xA1008U,
        };

        const std::array contributions{
            p::PresentationContribution{
                .bucket = bucket,
                .precipitation = precipitation(10.0),
            },
            p::PresentationContribution{
                .bucket = bucket,
                .precipitation = precipitation(30.0),
            },
        };

        const auto result =
            p::aggregate_presentation_bucket(contributions);

        ok &= expect(
            result.has_value(),
            "GC-008 agregação do bucket aceita"
        );

        if (result) {
            ok &= expect(
                result->bucket == bucket,
                "GC-008 preserva identidade do bucket"
            );

            ok &= expect(
                result->monitored_points == 2,
                "GC-008 possui dois pontos monitorados"
            );

            ok &= expect(
                result->precipitation.value() == 20.0,
                "GC-008 média espacial = 20 mm"
            );

            ok &= expect(
                result->precipitation.value() != 40.0,
                "GC-008 soma espacial de 40 mm é proibida"
            );

            ok &= expect(
                result->aggregation
                    == p::PresentationAggregation::spatial_mean,
                "GC-008 declara agregação por média espacial"
            );
        }
    }

    // Um conjunto vazio não constitui bucket apresentável.
    {
        const std::span<const p::PresentationContribution> empty;
        const auto result =
            p::aggregate_presentation_bucket(empty);

        ok &= expect(
            !result
                && result.error()
                    == p::PresentationAggregationError::
                        empty_contribution_set,
            "bucket vazio rejeitado"
        );
    }

    // Uma chamada não pode misturar unidades de apresentação.
    {
        const std::array contributions{
            p::PresentationContribution{
                .bucket = {.opaque_value = 1U},
                .precipitation = precipitation(10.0),
            },
            p::PresentationContribution{
                .bucket = {.opaque_value = 2U},
                .precipitation = precipitation(30.0),
            },
        };

        const auto result =
            p::aggregate_presentation_bucket(contributions);

        ok &= expect(
            !result
                && result.error()
                    == p::PresentationAggregationError::
                        mixed_presentation_buckets,
            "mistura de buckets rejeitada"
        );
    }

    ok &= expect(
        p::to_string(
            p::PresentationAggregationError::
                mixed_presentation_buckets
        ) == "mixed_presentation_buckets",
        "erro de bucket possui identidade textual estável"
    );

    if (ok) {
        std::cout
            << "[PASS] GC-008 presentation spatial mean\n";
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
