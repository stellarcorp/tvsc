#include "service/datetime/common/datetime_utils.h"

#include <chrono>

#include "glog/logging.h"
#include "service/datetime/common/datetime.pb.h"

namespace tvsc::service::datetime {

std::chrono::nanoseconds as_duration(DatetimeRequest::Precision precision) {
  using namespace std::literals::chrono_literals;

  std::chrono::nanoseconds duration{};
  switch (precision) {
    case DatetimeRequest::NANOSECOND:
      duration = 1ns;
      break;

    case DatetimeRequest::MICROSECOND:
      duration = 1us;
      break;

    case DatetimeRequest::MILLISECOND:
      duration = 1ms;
      break;

    case DatetimeRequest::SECOND:
      duration = 1s;
      break;

    case DatetimeRequest::MINUTE:
      duration = 1min;
      break;

    case DatetimeRequest::HOUR:
      duration = 1h;
      break;

    case DatetimeRequest::DAY:
      duration = 24h;
      break;

    case DatetimeRequest::WEEK:
      duration = 7 * 24h;
      break;

    case DatetimeRequest::YEAR:
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(DAYS_IN_YEAR * 24h);
      break;

    case DatetimeRequest::MONTH:
      duration = std::chrono::duration_cast<std::chrono::nanoseconds>(DAYS_IN_YEAR * 24h / 12);
      break;

    default:
      LOG(WARNING) << "Unrecognized precision value in request";
      throw std::domain_error("Unrecognized precision value in request");
  }
  return duration;
}

}  // namespace tvsc::service::datetime
