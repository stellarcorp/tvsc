#pragma once

#include <cstdint>

#include "hal/enable_lock.h"
#include "hal/random/rng.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::random {

class RngStm32xxxx final : public Rng {
 private:
  RNG_HandleTypeDef rng_{.Instance = RNG};
  uint32_t enable_counter_{};

 public:
  // Turn on power and clock to this peripheral.
  EnableLock enable() override;

  uint32_t operator()() override;
};

}  // namespace tvsc::hal::random
