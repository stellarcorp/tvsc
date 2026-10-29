#pragma once

#include <cstdint>

#include "hal2/random/rng.h"
#include "hal2/rcc/oscillator.h"
#include "hal2/register.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal2::random {

class RngRegisterBank final {
 public:
  // Control Register used to enable the RNG, manage interrupt enablement,
  // and control clock error detection.
  // Offset 0x00
  volatile Register CR;

  // Status Register used to check data-ready (DRDY), clock error (CECS),
  // and seed error (SECS) flags.
  // Offset 0x04
  volatile Register SR;

  // Data Register used to read the generated 32-bit random value.
  // Reading this register automatically clears the DRDY flag.
  // Offset 0x08
  volatile Register DR;
};

class Stm32Rng final : public RngPeripheral<uint32_t> {
 private:
  RngRegisterBank* registers_;

  rcc::Oscillator* oscillator_;
  // Create an activation instance, but it is invalid by default.
  rcc::OscillatorDriver hsi48_driver_{};

  // Turn on power and clock to this peripheral.
  void enable() noexcept override;
  void disable() noexcept override;

 public:
  Stm32Rng(rcc::Oscillator& oscillator) noexcept
      : registers_(reinterpret_cast<RngRegisterBank*>(RNG)), oscillator_(&oscillator) {}

  [[nodiscard]] uint32_t generate() noexcept override;
};

}  // namespace tvsc::hal2::random
