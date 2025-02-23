#pragma once

#include <cstdint>

#include "hal/peripheral_id.h"
#include "hal/rcc/rcc.h"
#include "hal/stm32_peripheral_ids.h"
#include "hal/timer/timer.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::timer {

class TimerStm32l4xx final : public TimerPeripheral {
 private:
  TIM_HandleTypeDef timer_{};
  PeripheralId id_;

 public:
  TimerStm32l4xx(PeripheralId id, TIM_TypeDef* timer_instance) : id_(id) {
    timer_.Instance = timer_instance;
  }

  PeripheralId id() override;

  void start(uint32_t interval_us) override;
  void stop() override;

  bool is_running() override;

  // Turn on power and clock to this peripheral.
  void enable() override;
  void disable() override;

  void handle_interrupt() override;
};

class Stm32l4xxLptim final : public TimerPeripheral {
 private:
  LPTIM_HandleTypeDef timer_{};
  PeripheralId id_;
  rcc::LsiOscillator* oscillator_;
  // Create an activation instance, but it is invalid by default.
  rcc::LsiActivation lsi_active_{};
  volatile bool is_running_{};

 public:
  Stm32l4xxLptim(PeripheralId id, LPTIM_TypeDef* timer_instance, rcc::LsiOscillator& oscillator)
      : id_(id), oscillator_(&oscillator) {
    timer_.Instance = timer_instance;
  }

  PeripheralId id() override;

  void start(uint32_t interval_us) override;
  void stop() override;

  bool is_running() override;

  // Turn on power and clock to this peripheral.
  void enable() override;
  void disable() override;

  void handle_interrupt() override;
};

}  // namespace tvsc::hal::timer
