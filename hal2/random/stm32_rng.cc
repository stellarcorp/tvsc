#include "hal2/random/stm32_rng.h"

#include <cstdint>

#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal2::random {

// Turn on power and clock to this peripheral.
void Stm32Rng::enable() noexcept {
  // Turn on the HSI48 oscillator.
  hsi48_driver_ = oscillator_->access();

  __HAL_RCC_RNG_CLK_ENABLE();

  // TODO(james): Rework this code. The HAL function below can block for up to 2ms.
  HAL_RNG_Init(&rng_);
}

void Stm32Rng::disable() noexcept {
  __HAL_RCC_RNG_CLK_DISABLE();

  // Invalidate this activation instance, likely turning off the HSI48 oscillator.
  hsi48_driver_.invalidate();
}

[[nodiscard]] uint32_t Stm32Rng::generate() noexcept {
  uint32_t result;

  // TODO(james): Rework this code. The HAL function below can block for up to 2ms.
  HAL_RNG_GenerateRandomNumber(&rng_, &result);
  return result;
}

}  // namespace tvsc::hal2::random
