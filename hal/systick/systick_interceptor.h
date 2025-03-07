#pragma once

#include "hal/interceptor.h"
#include "hal/systick/systick.h"
#include "hal/time_type.h"

namespace tvsc::hal::systick {

template <typename ClockType>
class SysTickInterceptor final : public Interceptor<SysTickType, ClockType> {
 public:
  SysTickInterceptor(SysTickType& systick) : Interceptor<SysTickType, ClockType>(systick) {}

  TimeType current_time_micros() override {
    LOG_FN();
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
