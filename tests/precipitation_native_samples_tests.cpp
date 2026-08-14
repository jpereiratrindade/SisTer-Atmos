#include "sister/atmos/precipitation/native_samples.hpp"

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
T require_value(
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
            .latitude = require_value(
                p::Latitude::create(latitude),
                "latitude inválida no fixture"
            ),
            .longitude = require_value(
                p::Longitude::create(longitude),
                "longitude inválida no fixture"
            ),
        },
        .precipitation = require_value(
            p::PrecipitationMm::create(precipitation_mm),
            "precipitação inválida no fixture"
        ),
    };
}

}  // namespace

int main() {
    bool ok = true;

    // Entrada vazia deve ser semanticamente explícita.
    {
        constexpr std::array<p::NativeSample, 0> samples{};

        const auto result =
            p::deduplicate_native_samples(samples);

        ok &= expect(
            !result
                && result.error()
                    == p::NativeSampleError::empty_sample_set,
            "conjunto vazio rejeitado"
        );
    }

    // GC-001 — mesma célula e mesmo valor representam
    // uma única observação espacial efetiva.
    {
        const std::array samples{
            sample(
                p::MeteorologicalProduct::era5,
                -30.0,
                -51.0,
                12.0
            ),
            sample(
                p::MeteorologicalProduct::era5,
                -30.0,
                -51.0,
                12.0
            ),
        };

        const auto result =
            p::deduplicate_native_samples(samples);

        ok &= expect(
            result.has_value(),
            "GC-001 aceito"
        );

        if (result) {
            ok &= expect(
                result->size() == 1,
                "GC-001 produz uma célula única"
            );

            if (result->size() == 1) {
                ok &= expect(
                    result->front().sample_count == 2,
                    "GC-001 preserva sample_count=2"
                );

                ok &= expect(
                    result->front().precipitation.value()
                        == 12.0,
                    "GC-001 preserva precipitação 12 mm"
                );
            }
        }
    }

    // GC-002 — mesma célula com valores divergentes
    // não pode ser conciliada silenciosamente.
    {
        const std::array samples{
            sample(
                p::MeteorologicalProduct::era5,
                -30.0,
                -51.0,
                12.0
            ),
            sample(
                p::MeteorologicalProduct::era5,
                -30.0,
                -51.0,
                13.0
            ),
        };

        const auto result =
            p::deduplicate_native_samples(samples);

        ok &= expect(
            !result
                && result.error()
                    == p::NativeSampleError::
                        conflicting_native_cell_values,
            "GC-002 rejeita conflito"
        );
    }

    // Mesmo ponto geométrico, produtos diferentes:
    // identidades científicas diferentes.
    {
        const std::array samples{
            sample(
                p::MeteorologicalProduct::era5,
                -30.0,
                -51.0,
                12.0
            ),
            sample(
                p::MeteorologicalProduct::best_match_dynamic,
                -30.0,
                -51.0,
                12.0
            ),
        };

        const auto result =
            p::deduplicate_native_samples(samples);

        ok &= expect(
            result && result->size() == 2,
            "produto participa da identidade da célula"
        );
    }

    // Coordenadas diferentes não são fundidas por proximidade.
    {
        const std::array samples{
            sample(
                p::MeteorologicalProduct::era5,
                -30.0,
                -51.0,
                12.0
            ),
            sample(
                p::MeteorologicalProduct::era5,
                -30.000001,
                -51.0,
                12.0
            ),
        };

        const auto result =
            p::deduplicate_native_samples(samples);

        ok &= expect(
            result && result->size() == 2,
            "não existe deduplicação espacial por epsilon"
        );
    }

    // A ordem de consulta não altera o resultado canônico.
    {
        const auto a = sample(
            p::MeteorologicalProduct::era5,
            -31.0,
            -52.0,
            5.0
        );

        const auto b = sample(
            p::MeteorologicalProduct::era5,
            -30.0,
            -51.0,
            10.0
        );

        const std::array forward{a, b};
        const std::array reverse{b, a};

        const auto first =
            p::deduplicate_native_samples(forward);

        const auto second =
            p::deduplicate_native_samples(reverse);

        ok &= expect(
            first
                && second
                && first->size() == second->size(),
            "ordenações produzem cardinalidade equivalente"
        );

        if (
            first
            && second
            && first->size() == second->size()
        ) {
            bool same = true;

            for (std::size_t i = 0; i < first->size(); ++i) {
                same &= (
                    (*first)[i].cell == (*second)[i].cell
                    && (*first)[i].precipitation
                        == (*second)[i].precipitation
                    && (*first)[i].sample_count
                        == (*second)[i].sample_count
                );
            }

            ok &= expect(
                same,
                "resultado é independente da ordem de consulta"
            );
        }
    }

    ok &= expect(
        p::to_string(
            p::NativeSampleError::
                conflicting_native_cell_values
        ) == "conflicting_native_cell_values",
        "erro científico possui identidade textual estável"
    );

    if (ok) {
        std::cout
            << "[PASS] deduplicação de células nativas\n";
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
