#pragma once

#include "hal/interceptor.h"
#include "hal/rcc/rcc.h"

namespace tvsc::hal::rcc {

template <typename ClockType>
class RccInterceptor final : public Interceptor<Rcc, ClockType> {
 public:
  RccInterceptor(Rcc& rcc) : Interceptor<Rcc, ClockType>(rcc) {}

  void set_clock_to_max_speed() override {
    LOG_FN();
    return this->call(&Rcc::set_clock_to_max_speed);
  }

  void set_clock_to_min_speed() override {
    LOG_FN();
    return this->call(&Rcc::set_clock_to_min_speed);
  }

  void set_clock_to_energy_efficient_speed() override {
    LOG_FN();
    return this->call(&Rcc::set_clock_to_energy_efficient_speed);
  }

  void restore_clock_speed() override {
    LOG_FN();
    return this->call(&Rcc::restore_clock_speed);
  }
};

}  // namespace tvsc::hal::rcc
