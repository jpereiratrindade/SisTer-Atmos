#pragma once

#include "sister/atmos/precipitation/domain_types.hpp"

#include <chrono>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace sister::atmos::precipitation {

enum class TemporalCoverageError : std::uint8_t {
    invalid_calendar_date,
    period_end_before_start,
    empty_cell_set,
    duplicate_native_cell_series,
    duplicate_observation_date,
    observation_outside_requested_period,
    cell_product_mismatch,
    no_common_temporal_coverage,
    non_contiguous_common_temporal_coverage,
    precipitation_accumulation_overflow,
};

[[nodiscard]]
constexpr std::string_view
to_string(TemporalCoverageError error) noexcept {
    switch (error) {
    case TemporalCoverageError::invalid_calendar_date:
        return "invalid_calendar_date";
    case TemporalCoverageError::period_end_before_start:
        return "period_end_before_start";
    case TemporalCoverageError::empty_cell_set:
        return "empty_cell_set";
    case TemporalCoverageError::duplicate_native_cell_series:
        return "duplicate_native_cell_series";
    case TemporalCoverageError::duplicate_observation_date:
        return "duplicate_observation_date";
    case TemporalCoverageError::observation_outside_requested_period:
        return "observation_outside_requested_period";
    case TemporalCoverageError::cell_product_mismatch:
        return "cell_product_mismatch";
    case TemporalCoverageError::no_common_temporal_coverage:
        return "no_common_temporal_coverage";
    case TemporalCoverageError::
        non_contiguous_common_temporal_coverage:
        return "non_contiguous_common_temporal_coverage";
    case TemporalCoverageError::precipitation_accumulation_overflow:
        return "precipitation_accumulation_overflow";
    }

    return "unknown_temporal_coverage_error";
}

class CalendarDate final {
public:
    [[nodiscard]]
    static std::expected<CalendarDate, TemporalCoverageError>
    create(
        int year,
        unsigned month,
        unsigned day
    ) noexcept;

    [[nodiscard]]
    static constexpr CalendarDate
    from_sys_days(std::chrono::sys_days value) noexcept {
        return CalendarDate{value};
    }

    [[nodiscard]]
    constexpr std::chrono::sys_days
    value() const noexcept {
        return value_;
    }

    friend bool operator==(
        const CalendarDate&,
        const CalendarDate&
    ) = default;

    friend auto operator<=>(
        const CalendarDate&,
        const CalendarDate&
    ) = default;

private:
    explicit constexpr CalendarDate(
        std::chrono::sys_days value
    ) noexcept
        : value_(value) {}

    std::chrono::sys_days value_;
};

class TemporalPeriod final {
public:
    [[nodiscard]]
    static std::expected<TemporalPeriod, TemporalCoverageError>
    create(
        CalendarDate start,
        CalendarDate end
    ) noexcept;

    [[nodiscard]]
    constexpr CalendarDate start() const noexcept {
        return start_;
    }

    [[nodiscard]]
    constexpr CalendarDate end() const noexcept {
        return end_;
    }

    [[nodiscard]]
    constexpr bool contains(
        CalendarDate date
    ) const noexcept {
        return date >= start_ && date <= end_;
    }

    friend bool operator==(
        const TemporalPeriod&,
        const TemporalPeriod&
    ) = default;

private:
    constexpr TemporalPeriod(
        CalendarDate start,
        CalendarDate end
    ) noexcept
        : start_(start),
          end_(end) {}

    CalendarDate start_;
    CalendarDate end_;
};

struct DailyPrecipitation final {
    CalendarDate date;
    std::optional<PrecipitationMm> precipitation;
};

struct NativeCellTemporalSeries final {
    NativeCellKey cell;
    std::span<const DailyPrecipitation> observations;
};

struct CellTemporalAccumulation final {
    NativeCellKey cell;
    PrecipitationMm precipitation;
};

struct AnalyticalTemporalContext final {
    MeteorologicalProduct used_product;
    TemporalPeriod effective_period;
};

struct TemporalCoverage final {
    TemporalPeriod requested_period;
    AnalyticalTemporalContext context;
    std::vector<CalendarDate> missing_dates;
    std::vector<CellTemporalAccumulation> cell_totals;
    std::size_t available_days;
    bool complete;
};

[[nodiscard]]
std::expected<
    TemporalCoverage,
    TemporalCoverageError
>
common_temporal_coverage(
    MeteorologicalProduct used_product,
    TemporalPeriod requested_period,
    std::span<const NativeCellTemporalSeries> cells
);

}  // namespace sister::atmos::precipitation
