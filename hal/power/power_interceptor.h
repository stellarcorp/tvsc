#pragma once

#include "hal/interceptor.h"
#include "hal/power/power.h"

namespace tvsc::hal::power {

class PowerInterceptor final : public Interceptor<Power> {
 public:
  PowerInterceptor(Power& power) : Interceptor(power) {}

  void enter_sleep_mode() override;
  void enter_stop_mode() override;
};

}  // namespace tvsc::hal::power
