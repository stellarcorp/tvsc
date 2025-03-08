#pragma once

#include "hal/power/power.h"
#include "hal/simulation/interceptor.h"
#include "hal/simulation/logger.h"

namespace tvsc::hal::power {

template <typename ClockType>
class PowerInterceptor final : public simulation::Interceptor<Power, ClockType> {
 public:
  PowerInterceptor(Power& power, simulation::Logger<ClockType>& logger)
      : simulation::Interceptor<Power, ClockType>(power, logger) {}

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
