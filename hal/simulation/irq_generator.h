#pragma once

#include "hal/simulation/logger.h"

namespace tvsc::hal::simulation {

template <typename ClockType>
class IrqGenerator {
 private:
  Logger<ClockType>* logger_{nullptr};

 public:
  IrqGenerator() = default;
  IrqGenerator(Logger<ClockType>& logger) : logger_(&logger) {}
  virtual ~IrqGenerator() = default;

  virtual ClockType::duration next_interrupt_in(ClockType::time_point now) const noexcept = 0;

  virtual int irq() const noexcept = 0;
  virtual const char* irq_name() const noexcept = 0;

  void generate_interrupt(ClockType::time_point now) noexcept {
    if (logger_) {
      logger_->log_irq(irq(), irq_name());
    }
    handle_interrupt();
  }

  virtual void handle_interrupt() noexcept = 0;
};

}  // namespace tvsc::hal::simulation
