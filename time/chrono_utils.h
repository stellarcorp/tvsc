#pragma once

#include <chrono>
#include <string>

#ifdef __has_include
#if __has_include(<format>)

#include <format>

#else

#include <iomanip>
#include <iostream>
#include <sstream>

#endif
#endif

namespace tvsc::time {

inline std::string to_string(std::chrono::microseconds d) noexcept {
  using std::to_string;
  std::string result{};
  double count{d.count() / 1'000'000.};
  int hours{static_cast<int>(count / (60 * 60))};
  count -= hours * 60 * 60;
  int minutes{static_cast<int>(count / 60)};
  count -= minutes * 60;
  double seconds{count};

  if (hours != 0) {
    result
        .append(to_string(hours))    //
        .append(":")                 //
        .append(to_string(minutes))  //
        .append(":")                 //
        .append(to_string(seconds));
  } else if (minutes != 0) {
    result
        .append(to_string(minutes))  //
        .append(":")                 //
        .append(to_string(seconds));
  } else {
    result.append(to_string(seconds));
  }

  return result;
}

inline std::string to_string(std::chrono::milliseconds d) noexcept {
  using std::to_string;
  std::string result{};
  double count{d.count() / 1'000.};
  int hours{static_cast<int>(count / (60 * 60))};
  count -= hours * 60 * 60;
  int minutes{static_cast<int>(count / 60)};
  count -= minutes * 60;
  double seconds{count};

  if (hours != 0) {
    result
        .append(to_string(hours))    //
        .append(":")                 //
        .append(to_string(minutes))  //
        .append(":")                 //
        .append(to_string(seconds));
  } else if (minutes != 0) {
    result
        .append(to_string(minutes))  //
        .append(":")                 //
        .append(to_string(seconds));
  } else {
    result.append(to_string(seconds));
  }

  return result;
}

template <typename Rep, typename Period>
std::string to_string(std::chrono::duration<Rep, Period> d) noexcept {
  return to_string(std::chrono::duration_cast<std::chrono::microseconds>(d));
}

#if __cpp_lib_format >= 202311L

template <typename ClockType>
std::string format_timestamp(const std::chrono::time_point<ClockType>& tp) {
  using namespace std::chrono;

  // Convert to system_clock for conversion to time_t followed by a conversion to local time. time_t
  // only has a precision to seconds, but we assume that the subseconds will not be affected by a
  // conversion to system_clock or local time.
  auto sys_tp = time_point_cast<system_clock::duration>(tp);
  auto tt = system_clock::to_time_t(sys_tp);
  auto tm = *std::localtime(&tt);

  // Extract milliseconds and microseconds. Again, we assume that these subseconds would be the same
  // values on the system_clock and in local time.
  auto duration = duration_cast<microseconds>(sys_tp.time_since_epoch());
  auto millis = (duration.count() / 1000) % 1000;
  auto micros = duration.count() % 1000;

  return std::format("{:%Y%m%d_%H%M%S}.{:03}{:03}", tm, millis, micros);
}

#else

template <typename ClockType>
std::string format_timestamp(const std::chrono::time_point<ClockType>& tp) {
  using namespace std::chrono;

  // Convert to system_clock for conversion to time_t followed by a conversion to local time. time_t
  // only has a precision to seconds, but we assume that the subseconds will not be affected by a
  // conversion to system_clock or local time.
  auto sys_tp = time_point_cast<system_clock::duration>(tp);
  auto tt = system_clock::to_time_t(sys_tp);
  auto tm = *std::localtime(&tt);

  // Extract milliseconds and microseconds. Again, we assume that these subseconds would be the same
  // values on the system_clock and in local time.
  auto duration = duration_cast<microseconds>(sys_tp.time_since_epoch());
  auto millis = (duration.count() / 1000) % 1000;
  auto micros = duration.count() % 1000;

  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y%m%d_%H%M%S") << '.' << std::setw(3) << std::setfill('0') << millis
      << std::setw(3) << std::setfill('0') << micros;

  return oss.str();
}

#endif

template <typename ClockType, typename Duration>
std::string to_string(std::chrono::time_point<ClockType, Duration> t) noexcept {
  return format_timestamp(t);
}

}  // namespace tvsc::time
