#pragma once

#include "hal/simulation/interceptor.h"
#include "hal/power/power.h"

namespace tvsc::hal::power {

template <typename ClockType>
class PowerInterceptor final : public simulation::Interceptor<Power, ClockType> {
 public:
  PowerInterceptor(Power& power) : simulation::Interceptor<Power, ClockType>(power) {}

  void enter_sleep_mode() override {
    LOG_FN();
    return this->call(&Power::enter_sleep_mode);
  }

  void enter_stop_mode() override {
    LOG_FN();
    return this->call(&Power::enter_stop_mode);
  }
};

}  // namespace tvsc::hal::power
