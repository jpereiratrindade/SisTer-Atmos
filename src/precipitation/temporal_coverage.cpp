#include "sister/atmos/precipitation/temporal_coverage.hpp"

#include <cmath>
#include <expected>
#include <limits>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace sister::atmos::precipitation {

std::expected<CalendarDate, TemporalCoverageError>
CalendarDate::create(
    int year,
    unsigned month,
    unsigned day
) noexcept {
    const std::chrono::year_month_day candidate{
        std::chrono::year{year},
        std::chrono::month{month},
        std::chrono::day{day},
    };

    if (!candidate.ok()) {
        return std::unexpected(
            TemporalCoverageError::invalid_calendar_date
        );
    }

    return CalendarDate{
        std::chrono::sys_days{candidate}
    };
}

std::expected<TemporalPeriod, TemporalCoverageError>
TemporalPeriod::create(
    CalendarDate start,
    CalendarDate end
) noexcept {
    if (end < start) {
        return std::unexpected(
            TemporalCoverageError::period_end_before_start
        );
    }

    return TemporalPeriod{start, end};
}

namespace {

struct NativeCellKeyLess final {
    [[nodiscard]]
    bool operator()(
        const NativeCellKey& lhs,
        const NativeCellKey& rhs
    ) const noexcept {
        const auto lhs_product =
            std::to_underlying(lhs.product);

        const auto rhs_product =
            std::to_underlying(rhs.product);

        if (lhs_product != rhs_product) {
            return lhs_product < rhs_product;
        }

        if (lhs.latitude.value() != rhs.latitude.value()) {
            return lhs.latitude.value() < rhs.latitude.value();
        }

        return lhs.longitude.value()
            < rhs.longitude.value();
    }
};

using DailyMap = std::map<
    std::chrono::sys_days,
    std::optional<PrecipitationMm>
>;

}  // namespace

std::expected<
    TemporalCoverage,
    TemporalCoverageError
>
common_temporal_coverage(
    MeteorologicalProduct used_product,
    TemporalPeriod requested_period,
    std::span<const NativeCellTemporalSeries> cells
) {
    if (cells.empty()) {
        return std::unexpected(
            TemporalCoverageError::empty_cell_set
        );
    }

    std::set<NativeCellKey, NativeCellKeyLess> unique_cells;
    std::vector<DailyMap> daily_maps;
    daily_maps.reserve(cells.size());

    for (const auto& cell : cells) {
        if (cell.cell.product != used_product) {
            return std::unexpected(
                TemporalCoverageError::cell_product_mismatch
            );
        }

        if (!unique_cells.insert(cell.cell).second) {
            return std::unexpected(
                TemporalCoverageError::
                    duplicate_native_cell_series
            );
        }

        DailyMap daily;

        for (const auto& observation : cell.observations) {
            if (!requested_period.contains(observation.date)) {
                return std::unexpected(
                    TemporalCoverageError::
                        observation_outside_requested_period
                );
            }

            const auto [iterator, inserted] =
                daily.emplace(
                    observation.date.value(),
                    observation.precipitation
                );

            static_cast<void>(iterator);

            if (!inserted) {
                return std::unexpected(
                    TemporalCoverageError::
                        duplicate_observation_date
                );
            }
        }

        daily_maps.push_back(std::move(daily));
    }

    std::vector<CalendarDate> common_dates;
    std::vector<CalendarDate> missing_dates;

    std::vector<long double> accumulations(
        cells.size(),
        0.0L
    );

    const auto first_day =
        requested_period.start().value();

    const auto last_day =
        requested_period.end().value();

    for (
        auto day = first_day;
        day <= last_day;
        day += std::chrono::days{1}
    ) {
        bool common = true;

        for (const auto& daily : daily_maps) {
            const auto found = daily.find(day);

            if (
                found == daily.end()
                || !found->second.has_value()
            ) {
                common = false;
                break;
            }
        }

        const CalendarDate date =
            CalendarDate::from_sys_days(day);

        if (!common) {
            missing_dates.push_back(date);
            continue;
        }

        common_dates.push_back(date);

        for (
            std::size_t index = 0;
            index < daily_maps.size();
            ++index
        ) {
            const auto found =
                daily_maps[index].find(day);

            const long double precipitation =
                static_cast<long double>(
                    found->second->value()
                );

            accumulations[index] += precipitation;

            if (!std::isfinite(accumulations[index])) {
                return std::unexpected(
                    TemporalCoverageError::
                        precipitation_accumulation_overflow
                );
            }
        }
    }

    if (common_dates.empty()) {
        return std::unexpected(
            TemporalCoverageError::
                no_common_temporal_coverage
        );
    }

    /*
     * TemporalPeriod representa intervalo civil contínuo.
     *
     * Se as datas comuns tiverem uma lacuna interna, usar
     * apenas first()/last() produziria falsa continuidade.
     * Enquanto o domínio não possuir representação de
     * cobertura segmentada, esse estado é rejeitado.
     */
    const auto common_span_days =
        (
            common_dates.back().value()
            - common_dates.front().value()
        ).count() + 1;

    if (
        common_span_days
        != static_cast<std::int64_t>(
            common_dates.size()
        )
    ) {
        return std::unexpected(
            TemporalCoverageError::
                non_contiguous_common_temporal_coverage
        );
    }

    const auto effective_period =
        TemporalPeriod::create(
            common_dates.front(),
            common_dates.back()
        );

    if (!effective_period) {
        return std::unexpected(
            effective_period.error()
        );
    }

    constexpr long double max_double =
        static_cast<long double>(
            std::numeric_limits<double>::max()
        );

    std::vector<CellTemporalAccumulation> totals;
    totals.reserve(cells.size());

    for (
        std::size_t index = 0;
        index < cells.size();
        ++index
    ) {
        const long double accumulated =
            accumulations[index];

        if (
            !std::isfinite(accumulated)
            || accumulated < 0.0L
            || accumulated > max_double
        ) {
            return std::unexpected(
                TemporalCoverageError::
                    precipitation_accumulation_overflow
            );
        }

        const auto precipitation =
            PrecipitationMm::create(
                static_cast<double>(accumulated)
            );

        if (!precipitation) {
            return std::unexpected(
                TemporalCoverageError::
                    precipitation_accumulation_overflow
            );
        }

        totals.push_back(
            CellTemporalAccumulation{
                .cell = cells[index].cell,
                .precipitation = *precipitation,
            }
        );
    }

    return TemporalCoverage{
        .requested_period = requested_period,
        .context = {
            .used_product = used_product,
            .effective_period = *effective_period,
        },
        .missing_dates = std::move(missing_dates),
        .cell_totals = std::move(totals),
        .available_days = common_dates.size(),
        .complete = (
            common_dates.size()
            == static_cast<std::size_t>(
                (
                    last_day - first_day
                ).count() + 1
            )
        ),
    };
}

}  // namespace sister::atmos::precipitation
