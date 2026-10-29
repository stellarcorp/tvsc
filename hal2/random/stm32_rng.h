#pragma once

#include <cstdint>

#include "hal2/random/rng.h"
#include "hal2/rcc/oscillator.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal2::random {

class Stm32Rng final : public RngPeripheral<uint32_t> {
 private:
  rcc::Oscillator* oscillator_;
  // Create an activation instance, but it is invalid by default.
  rcc::OscillatorDriver hsi48_driver_{};

  RNG_HandleTypeDef rng_{.Instance = RNG};

  // Turn on power and clock to this peripheral.
  void enable() noexcept override;
  void disable() noexcept override;

 public:
  Stm32Rng(rcc::Oscillator& oscillator) noexcept : oscillator_(&oscillator) {}

  [[nodiscard]] uint32_t generate() noexcept override;
};

}  // namespace tvsc::hal2::random
