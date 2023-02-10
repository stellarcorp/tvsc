#include "service/datetime/common/datetime_utils.h"

#include <chrono>

#include "glog/logging.h"
#include "service/datetime/common/datetime.pb.h"

namespace tvsc::service::datetime {

std::chrono::nanoseconds as_duration(int64_t count, TimeUnit precision) {
  using namespace std::literals::chrono_literals;

  std::chrono::nanoseconds nanos{};
  switch (precision) {
    case TimeUnit::NANOSECOND:
      nanos = count * 1ns;
      break;

    case TimeUnit::MICROSECOND:
      nanos = count * 1us;
      break;

    case TimeUnit::MILLISECOND:
      nanos = count * 1ms;
      break;

    case TimeUnit::SECOND:
      nanos = count * 1s;
      break;

    case TimeUnit::MINUTE:
      nanos = count * 1min;
      break;

    case TimeUnit::HOUR:
      nanos = count * 1h;
      break;

    case TimeUnit::DAY:
      nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(DayDuration{count});
      break;

    case TimeUnit::WEEK:
      nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(WeekDuration{count});
      break;

    case TimeUnit::YEAR:
      nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(YearDuration{count});
      break;

    case TimeUnit::MONTH:
      nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(MonthDuration{count});
      break;

    default:
      LOG(WARNING) << "Unrecognized precision value in request";
      throw std::domain_error("Unrecognized precision value in request");
  }

  return nanos;
}

}  // namespace tvsc::service::datetime
