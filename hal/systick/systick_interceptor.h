#pragma once

#include "hal/simulation/interceptor.h"
#include "hal/simulation/logger.h"
#include "hal/systick/systick.h"
#include "hal/time_type.h"

namespace tvsc::hal::systick {

template <typename ClockType>
class SysTickInterceptor final : public simulation::Interceptor<SysTickType, ClockType> {
 public:
  SysTickInterceptor(SysTickType& systick, simulation::Logger<ClockType>& logger)
      : simulation::Interceptor<SysTickType, ClockType>(systick, logger) {}

  TimeType current_time_micros() override {
    // We disable logging of this function. It is called often, creating a lot of noise in the log
    // file, and we generally do not care about these calls.
    // LOG_FN();
    return this->call(&SysTickType::current_time_micros);
  }

  void increment_micros(TimeType us) override {
    LOG_FN();
    return this->call(&SysTickType::increment_micros, us);
  }

  void handle_interrupt() override {
    LOG_FN();
    return this->call(&SysTickType::handle_interrupt);
  }
};

}  // namespace tvsc::hal::systick
