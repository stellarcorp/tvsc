#pragma once

#include <cstdint>

#include "hal/random/rng.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::random {

class RngStm32xxxx final : public RngPeripheral {
 private:
  RNG_HandleTypeDef rng_{.Instance = RNG};

  // Turn on power and clock to this peripheral.
  void enable() override;
  void disable() override;

 public:
  uint32_t generate() override;
};

}  // namespace tvsc::hal::random
