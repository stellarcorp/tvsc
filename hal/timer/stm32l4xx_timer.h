#pragma once

#include <cstdint>

#include "hal/enable_lock.h"
#include "hal/peripheral_id.h"
#include "hal/stm32_peripheral_ids.h"
#include "hal/timer/timer.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::timer {

class TimerStm32l4xx final : public Timer {
 private:
  TIM_HandleTypeDef timer_{};
  PeripheralId id_;
  uint32_t enable_counter_{0};

 public:
  TimerStm32l4xx(PeripheralId id, TIM_TypeDef* timer_instance) : id_(id) {
    timer_.Instance = timer_instance;
  }

  PeripheralId id() override;

  void start(uint32_t interval_us, bool high_precision) override;
  void stop() override;

  bool is_running() override;

  // Turn on power and clock to this peripheral.
  EnableLock enable() override;

  void handle_interrupt() override;
};

}  // namespace tvsc::hal::timer
