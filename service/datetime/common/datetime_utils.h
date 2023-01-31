#pragma once

#include <chrono>

#include "service/datetime/common/datetime.pb.h"

namespace tvsc::service::datetime {

// Days in a year is the average length of the Gregorian year, per
// https://en.cppreference.com/w/cpp/chrono/duration
constexpr float DAYS_IN_YEAR{365.2425};

std::chrono::nanoseconds as_duration(DatetimeRequest::Precision precision);

template <typename Clock = std::chrono::system_clock>
std::chrono::time_point<Clock> as_time_point(const DatetimeReply& reply,
                                             DatetimeRequest::Precision precision) {
  using namespace std::literals::chrono_literals;
  return std::chrono::time_point<Clock>{reply.datetime() * as_duration(precision)};
}

}  // namespace tvsc::service::datetime
