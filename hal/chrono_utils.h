#pragma once

#include <chrono>
#include <string>

namespace tvsc::hal {

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

template <typename ClockType, typename Duration>
std::string to_string(std::chrono::time_point<ClockType, Duration> t) noexcept {
  return to_string(t.time_since_epoch());
}

}  // namespace tvsc::hal
