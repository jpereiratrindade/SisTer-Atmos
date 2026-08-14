#include "sister/atmos/precipitation/area_weighting.hpp"

#include <array>
#include <cmath>
#include <cstdlib>
#include <expected>
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

bool near(
    double lhs,
    double rhs,
    double tolerance
) {
    return std::fabs(lhs - rhs) <= tolerance;
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

p::AreaContribution contribution(
    double precipitation_mm,
    double represented_area
) {
    return {
        .precipitation = require_value(
            p::PrecipitationMm::create(precipitation_mm),
            "precipitação inválida no fixture"
        ),
        .represented_area = require_value(
            p::RepresentedArea::create(represented_area),
            "área inválida no fixture"
        ),
    };
}

}  // namespace

int main() {
    bool ok = true;

    // Entrada vazia.
    {
        constexpr std::array<p::AreaContribution, 0>
            contributions{};

        const auto result =
            p::area_weighted_precipitation(contributions);

        ok &= expect(
            !result
                && result.error()
                    == p::AreaWeightingError::
                        empty_contribution_set,
            "conjunto vazio rejeitado"
        );
    }

    // GC-003.
    {
        const std::array contributions{
            contribution(10.0, 1.0),
            contribution(30.0, 0.5),
        };

        const auto result =
            p::area_weighted_precipitation(contributions);

        ok &= expect(
            result.has_value(),
            "GC-003 aceito"
        );

        if (result) {
            constexpr double expected =
                16.666666666666668;

            ok &= expect(
                near(
                    result->precipitation.value(),
                    expected,
                    1e-12
                ),
                "GC-003 média ponderada correta"
            );

            ok &= expect(
                near(
                    result->represented_area.value(),
                    1.5,
                    1e-15
                ),
                "GC-003 área total 1.5"
            );

            ok &= expect(
                result->contributors == 2,
                "GC-003 possui dois contribuintes"
            );

            ok &= expect(
                result->precipitation.value() != 40.0,
                "GC-003 não soma milímetros espacialmente"
            );
        }
    }

    // Um único contribuinte preserva o valor.
    {
        const std::array contributions{
            contribution(27.5, 4.0),
        };

        const auto result =
            p::area_weighted_precipitation(contributions);

        ok &= expect(
            result
                && result->precipitation.value() == 27.5
                && result->represented_area.value() == 4.0
                && result->contributors == 1,
            "um contribuinte preserva precipitação"
        );
    }

    // Ordem não altera resultado científico.
    {
        const auto first = contribution(10.0, 1.0);
        const auto second = contribution(30.0, 0.5);

        const std::array forward{first, second};
        const std::array reverse{second, first};

        const auto a =
            p::area_weighted_precipitation(forward);

        const auto b =
            p::area_weighted_precipitation(reverse);

        ok &= expect(
            a
                && b
                && near(
                    a->precipitation.value(),
                    b->precipitation.value(),
                    1e-12
                )
                && near(
                    a->represented_area.value(),
                    b->represented_area.value(),
                    1e-15
                ),
            "resultado independe da ordem"
        );
    }

    // Evita overflow intermediário de P × A.
    {
        const double maximum =
            std::numeric_limits<double>::max();

        const std::array contributions{
            contribution(maximum, 1.0),
            contribution(0.0, 1.0),
        };

        const auto result =
            p::area_weighted_precipitation(contributions);

        ok &= expect(
            result
                && std::isfinite(
                    result->precipitation.value()
                )
                && result->precipitation.value() > 0.0,
            "redução robusta com precipitação extrema"
        );

        if (result) {
            ok &= expect(
                near(
                    result->precipitation.value()
                        / maximum,
                    0.5,
                    1e-15
                ),
                "valor extremo ponderado corretamente"
            );
        }
    }

    // Área total não representável em double é rejeitada.
    {
        const double maximum =
            std::numeric_limits<double>::max();

        const std::array contributions{
            contribution(10.0, maximum),
            contribution(20.0, maximum),
        };

        const auto result =
            p::area_weighted_precipitation(contributions);

        ok &= expect(
            !result
                && result.error()
                    == p::AreaWeightingError::
                        represented_area_overflow,
            "overflow da área representada rejeitado"
        );
    }

    ok &= expect(
        p::to_string(
            p::AreaWeightingError::
                represented_area_overflow
        ) == "represented_area_overflow",
        "erro de agregação possui identidade estável"
    );

    if (ok) {
        std::cout
            << "[PASS] precipitação ponderada por área\n";
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
