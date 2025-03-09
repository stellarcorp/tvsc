#pragma once

#include "hal/simulation/interceptor.h"
#include "hal/simulation/irq_generator.h"
#include "hal/simulation/logger.h"
#include "hal/systick/systick.h"
#include "hal/time_type.h"

namespace tvsc::hal::systick {

template <typename ClockType>
class SysTickInterceptor final : public simulation::Interceptor<SysTickType, ClockType>,
                                 public simulation::IrqGenerator<ClockType> {
 public:
  using ReactorType = simulation::Reactor<ClockType>;

  SysTickInterceptor(SysTickType& systick, simulation::Logger<ClockType>& logger)
      : simulation::Interceptor<SysTickType, ClockType>(systick, logger),
        simulation::IrqGenerator<ClockType>(logger) {}

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

  void handle_interrupt() noexcept override {
    LOG_FN();
    return this->call(&SysTickType::handle_interrupt);
  }

  int irq() const noexcept override { return /* SysTick_IRQn */ -1; }
  const char* irq_name() const noexcept override { return "SysTick_IRQ"; }

  ClockType::duration next_interrupt_in(ClockType::time_point /*now*/) const noexcept override {
    using namespace std::chrono_literals;
    return 1000us;
  }
};

}  // namespace tvsc::hal::systick
