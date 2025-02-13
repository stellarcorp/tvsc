#include "hal/random/stm32xxxx_rng.h"

#include <cstdint>

#include "hal/enable_lock.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::random {

void disable() { __HAL_RCC_RNG_CLK_DISABLE(); }

// Turn on power and clock to this peripheral.
EnableLock RngStm32xxxx::enable() {
  __HAL_RCC_RNG_CLK_ENABLE();
  // TODO(james): Rework this code. The HAL function below can block for up to 2ms.
  HAL_RNG_Init(&rng_);

  return EnableLock([]() { disable(); });
}

uint32_t RngStm32xxxx::operator()() {
  uint32_t result;

  // TODO(james): Rework this code. The HAL function below can block for up to 2ms.
  while (HAL_RNG_GenerateRandomNumber(&rng_, &result) != HAL_OK) {
    // co_yield;
  }
  return result;
}

}  // namespace tvsc::hal::random
