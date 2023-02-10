#pragma once

#include <chrono>

#include "service/datetime/common/datetime.pb.h"

namespace tvsc::service::datetime {

// Days in a year is the average length of the Gregorian year, per
// https://en.cppreference.com/w/cpp/chrono/duration
constexpr float DAYS_IN_YEAR{365.2425};

constexpr int32_t SECONDS_IN_DAY{24 * 60 * 60};
constexpr int32_t SECONDS_IN_WEEK{7 * SECONDS_IN_DAY};
constexpr int32_t SECONDS_IN_YEAR{static_cast<int32_t>(DAYS_IN_YEAR * SECONDS_IN_DAY)};

// Per https://en.cppreference.com/w/cpp/chrono/duration
constexpr int32_t SECONDS_IN_MONTH{static_cast<int32_t>(SECONDS_IN_YEAR / 12)};

/**
 * Duration types that are not available in std::chrono. These are designed to supplement the
 * existing std::chrono::milliseconds, std::chrono::seconds, etc. Note that standard versions of
 * these are available in C++20, but we are trying to maintain compatibility with C++17.
 */
using DayDuration = std::chrono::duration<int32_t, std::ratio<SECONDS_IN_DAY>>;
using WeekDuration = std::chrono::duration<int32_t, std::ratio<SECONDS_IN_WEEK>>;
using MonthDuration = std::chrono::duration<int32_t, std::ratio<SECONDS_IN_MONTH>>;
using YearDuration = std::chrono::duration<int32_t, std::ratio<SECONDS_IN_YEAR>>;

/**
 * Constructs a duration in nanoseconds for the duration indicated by a count of the given unit.
 */
std::chrono::nanoseconds as_duration(int64_t count, TimeUnit unit);

/**
 * Constructs a time_point at Clock's epoch plus the duration given by count and unit.
 */
template <typename Clock = std::chrono::system_clock>
std::chrono::time_point<Clock> as_time_point(int64_t count, TimeUnit unit) {
  return std::chrono::time_point<Clock>{as_duration(count, unit)};
}

}  // namespace tvsc::service::datetime
