#include "hal/random/stm32xxxx_rng.h"

#include <cstdint>

#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::random {

// Turn on power and clock to this peripheral.
void RngStm32xxxx::enable() {
  // Turn on the HSI48 oscillator.
  hsi48_active_ = oscillator_->access();

  __HAL_RCC_RNG_CLK_ENABLE();

  // TODO(james): Rework this code. The HAL function below can block for up to 2ms.
  HAL_RNG_Init(&rng_);
}

void RngStm32xxxx::disable() {
  __HAL_RCC_RNG_CLK_DISABLE();

  // Invalidate this activation instance, likely turning off the HSI48 oscillator.
  hsi48_active_.invalidate();
}

uint32_t RngStm32xxxx::generate() {
  uint32_t result;

  // TODO(james): Rework this code. The HAL function below can block for up to 2ms.
  HAL_RNG_GenerateRandomNumber(&rng_, &result);
  return result;
}

}  // namespace tvsc::hal::random
