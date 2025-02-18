#pragma once

#include <cstdint>

#include "hal/random/rng.h"
#include "hal/rcc/rcc.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::random {

class RngStm32xxxx final : public RngPeripheral {
 private:
  rcc::Hsi48Oscillator* oscillator_;
  // Create an activation instance, but it is invalid by default.
  rcc::Hsi48Activation hsi48_active_{};

  RNG_HandleTypeDef rng_{.Instance = RNG};

  // Turn on power and clock to this peripheral.
  void enable() override;
  void disable() override;

 public:
  RngStm32xxxx(rcc::Hsi48Oscillator& oscillator) : oscillator_(&oscillator) {}

  uint32_t generate() override;
};

}  // namespace tvsc::hal::random
