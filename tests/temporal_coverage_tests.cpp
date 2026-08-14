#include "sister/atmos/precipitation/temporal_coverage.hpp"

#include <array>
#include <cstdlib>
#include <expected>
#include <iostream>
#include <optional>

namespace p = sister::atmos::precipitation;

namespace {

bool expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "[FAIL] " << message << '\n';
        return false;
    }

    return true;
}

template <typename T, typename E>
T require_value(
    std::expected<T, E> result,
    const char* message
) {
    if (!result) {
        std::cerr << "[FAIL] " << message << '\n';
        std::exit(EXIT_FAILURE);
    }

    return *result;
}

p::CalendarDate date(
    int year,
    unsigned month,
    unsigned day
) {
    return require_value(
        p::CalendarDate::create(year, month, day),
        "data inválida no fixture"
    );
}

p::PrecipitationMm precipitation(double value) {
    return require_value(
        p::PrecipitationMm::create(value),
        "precipitação inválida no fixture"
    );
}

p::NativeCellKey cell(
    double latitude,
    double longitude
) {
    return {
        .product = p::MeteorologicalProduct::era5,
        .latitude = require_value(
            p::Latitude::create(latitude),
            "latitude inválida no fixture"
        ),
        .longitude = require_value(
            p::Longitude::create(longitude),
            "longitude inválida no fixture"
        ),
    };
}

p::TemporalPeriod period(
    p::CalendarDate start,
    p::CalendarDate end
) {
    return require_value(
        p::TemporalPeriod::create(start, end),
        "período inválido no fixture"
    );
}

}  // namespace

int main() {
    bool ok = true;

    const auto july_01 = date(2026, 7, 1);
    const auto july_02 = date(2026, 7, 2);
    const auto july_03 = date(2026, 7, 3);

    const auto requested =
        period(july_01, july_03);

    // GC-007.
    {
        const std::array cell_a_daily{
            p::DailyPrecipitation{
                .date = july_01,
                .precipitation = precipitation(1.0),
            },
            p::DailyPrecipitation{
                .date = july_02,
                .precipitation = precipitation(2.0),
            },
            p::DailyPrecipitation{
                .date = july_03,
                .precipitation = std::nullopt,
            },
        };

        const std::array cell_b_daily{
            p::DailyPrecipitation{
                .date = july_01,
                .precipitation = precipitation(10.0),
            },
            p::DailyPrecipitation{
                .date = july_02,
                .precipitation = precipitation(20.0),
            },
            p::DailyPrecipitation{
                .date = july_03,
                .precipitation = precipitation(30.0),
            },
        };

        const std::array cells{
            p::NativeCellTemporalSeries{
                .cell = cell(-30.0, -51.0),
                .observations = cell_a_daily,
            },
            p::NativeCellTemporalSeries{
                .cell = cell(-31.0, -52.0),
                .observations = cell_b_daily,
            },
        };

        const auto result =
            p::common_temporal_coverage(
                p::MeteorologicalProduct::era5,
                requested,
                cells
            );

        ok &= expect(
            result.has_value(),
            "GC-007 aceito"
        );

        if (result) {
            ok &= expect(
                result->requested_period == requested,
                "período solicitado preservado"
            );

            ok &= expect(
                result->context.used_product
                    == p::MeteorologicalProduct::era5,
                "produto utilizado preservado no contexto"
            );

            ok &= expect(
                result->context.effective_period.start()
                    == july_01
                && result->context.effective_period.end()
                    == july_02,
                "período efetivo 01–02/07"
            );

            ok &= expect(
                result->available_days == 2,
                "GC-007 possui dois dias comuns"
            );

            ok &= expect(
                result->missing_dates.size() == 1
                && result->missing_dates.front()
                    == july_03,
                "03/07 permanece lacuna explícita"
            );

            ok &= expect(
                !result->complete,
                "GC-007 possui cobertura incompleta"
            );

            ok &= expect(
                result->cell_totals.size() == 2,
                "GC-007 produz dois totais"
            );

            if (result->cell_totals.size() == 2) {
                ok &= expect(
                    result->cell_totals[0]
                            .precipitation.value()
                        == 3.0,
                    "célula A acumula 3 mm"
                );

                ok &= expect(
                    result->cell_totals[1]
                            .precipitation.value()
                        == 30.0,
                    "célula B acumula apenas 30 mm"
                );

                ok &= expect(
                    result->cell_totals[1]
                            .precipitation.value()
                        != 60.0,
                    "03/07 não contamina acumulação de B"
                );
            }
        }
    }

    // Uma lacuna interna não pode ser escondida por um
    // effective_period contínuo 01–03.
    {
        const std::array cell_a_daily{
            p::DailyPrecipitation{
                .date = july_01,
                .precipitation = precipitation(1.0),
            },
            p::DailyPrecipitation{
                .date = july_02,
                .precipitation = std::nullopt,
            },
            p::DailyPrecipitation{
                .date = july_03,
                .precipitation = precipitation(3.0),
            },
        };

        const std::array cell_b_daily{
            p::DailyPrecipitation{
                .date = july_01,
                .precipitation = precipitation(10.0),
            },
            p::DailyPrecipitation{
                .date = july_02,
                .precipitation = precipitation(20.0),
            },
            p::DailyPrecipitation{
                .date = july_03,
                .precipitation = precipitation(30.0),
            },
        };

        const std::array cells{
            p::NativeCellTemporalSeries{
                .cell = cell(-30.0, -51.0),
                .observations = cell_a_daily,
            },
            p::NativeCellTemporalSeries{
                .cell = cell(-31.0, -52.0),
                .observations = cell_b_daily,
            },
        };

        const auto result =
            p::common_temporal_coverage(
                p::MeteorologicalProduct::era5,
                requested,
                cells
            );

        ok &= expect(
            !result
                && result.error()
                    == p::TemporalCoverageError::
                        non_contiguous_common_temporal_coverage,
            "lacuna interna não produz falso período 01–03"
        );
    }

    // Lacuna na borda é representável por um período
    // efetivo contínuo menor que o solicitado.
    {
        const std::array cell_a_daily{
            p::DailyPrecipitation{
                .date = july_01,
                .precipitation = std::nullopt,
            },
            p::DailyPrecipitation{
                .date = july_02,
                .precipitation = precipitation(2.0),
            },
            p::DailyPrecipitation{
                .date = july_03,
                .precipitation = precipitation(3.0),
            },
        };

        const std::array cell_b_daily{
            p::DailyPrecipitation{
                .date = july_01,
                .precipitation = precipitation(10.0),
            },
            p::DailyPrecipitation{
                .date = july_02,
                .precipitation = precipitation(20.0),
            },
            p::DailyPrecipitation{
                .date = july_03,
                .precipitation = precipitation(30.0),
            },
        };

        const std::array cells{
            p::NativeCellTemporalSeries{
                .cell = cell(-30.0, -51.0),
                .observations = cell_a_daily,
            },
            p::NativeCellTemporalSeries{
                .cell = cell(-31.0, -52.0),
                .observations = cell_b_daily,
            },
        };

        const auto result =
            p::common_temporal_coverage(
                p::MeteorologicalProduct::era5,
                requested,
                cells
            );

        ok &= expect(
            result.has_value(),
            "lacuna inicial é representável"
        );

        if (result) {
            ok &= expect(
                result->context.effective_period.start()
                    == july_02
                && result->context.effective_period.end()
                    == july_03,
                "effective_period torna-se 02–03"
            );

            ok &= expect(
                result->missing_dates.size() == 1
                && result->missing_dates.front()
                    == july_01,
                "01/07 permanece explicitamente ausente"
            );

            ok &= expect(
                result->cell_totals[0]
                        .precipitation.value()
                    == 5.0
                && result->cell_totals[1]
                        .precipitation.value()
                    == 50.0,
                "totais usam somente 02–03"
            );
        }
    }

    // Período inválido não pode existir.
    {
        const auto result =
            p::TemporalPeriod::create(
                july_03,
                july_01
            );

        ok &= expect(
            !result
                && result.error()
                    == p::TemporalCoverageError::
                        period_end_before_start,
            "período invertido rejeitado"
        );
    }

    // Data civil inválida não pode existir.
    {
        const auto result =
            p::CalendarDate::create(
                2026,
                2,
                30
            );

        ok &= expect(
            !result
                && result.error()
                    == p::TemporalCoverageError::
                        invalid_calendar_date,
            "30/02 rejeitado"
        );
    }

    // Sem células não há cobertura científica.
    {
        constexpr std::array<
            p::NativeCellTemporalSeries,
            0
        > cells{};

        const auto result =
            p::common_temporal_coverage(
                p::MeteorologicalProduct::era5,
                requested,
                cells
            );

        ok &= expect(
            !result
                && result.error()
                    == p::TemporalCoverageError::
                        empty_cell_set,
            "conjunto vazio de células rejeitado"
        );
    }

    // Produto da célula precisa ser o produto efetivamente
    // utilizado pelo contexto analítico.
    {
        const std::array daily{
            p::DailyPrecipitation{
                .date = july_01,
                .precipitation = precipitation(1.0),
            },
        };

        auto mismatched_cell = cell(-30.0, -51.0);
        mismatched_cell.product =
            p::MeteorologicalProduct::
                best_match_dynamic;

        const std::array cells{
            p::NativeCellTemporalSeries{
                .cell = mismatched_cell,
                .observations = daily,
            },
        };

        const auto result =
            p::common_temporal_coverage(
                p::MeteorologicalProduct::era5,
                requested,
                cells
            );

        ok &= expect(
            !result
                && result.error()
                    == p::TemporalCoverageError::
                        cell_product_mismatch,
            "produto divergente da célula é rejeitado"
        );
    }

    ok &= expect(
        p::to_string(
            p::TemporalCoverageError::
                no_common_temporal_coverage
        ) == "no_common_temporal_coverage",
        "erro temporal possui identidade estável"
    );

    ok &= expect(
        p::to_string(
            p::TemporalCoverageError::
                non_contiguous_common_temporal_coverage
        ) == "non_contiguous_common_temporal_coverage",
        "erro de descontinuidade possui identidade estável"
    );

    if (ok) {
        std::cout
            << "[PASS] cobertura temporal comum\n";
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
