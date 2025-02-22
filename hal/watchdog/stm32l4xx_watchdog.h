#pragma once

#include "hal/rcc/rcc.h"
#include "hal/watchdog/watchdog.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::watchdog {

class WatchdogStm32l4xx final : public WatchdogPeripheral {
 private:
  IWDG_HandleTypeDef watchdog_{};
  rcc::LsiOscillator* oscillator_;
  // Create an activation instance, but it is invalid by default.
  rcc::LsiActivation lsi_active_{};

  void enable() override;
  void disable() override;

  void feed() override;

 public:
  WatchdogStm32l4xx(IWDG_TypeDef* watchdog_instance, rcc::LsiOscillator& oscillator)
      : oscillator_(&oscillator) {
    watchdog_.Instance = watchdog_instance;
  }

  [[nodiscard]] std::chrono::milliseconds reset_interval() override;
};

}  // namespace tvsc::hal::watchdog
