#pragma once

#include "hal2/peripheral.h"
#include "hal2/random/rng.h"
#include "hal2/random/stm32_rng.h"
#include "hal2/rcc/stm32l4_rcc_config.h"

namespace tvsc::hal2::random {

static constexpr size_t HSI48_OSCILLATOR_INDEX{2};

template <typename System, typename PeripheralRegistry>
class Stm32RngAccessor {
 private:
  static constexpr PeripheralId hsi48_id{Facility::OSCILLATOR_FACILITY,
                                                 HSI48_OSCILLATOR_INDEX};

  using Hsi48OscillatorAccessor = config::accessor_type<System, PeripheralRegistry, hsi48_id>;

  Stm32Rng rng_{static_cast<Hsi48OscillatorAccessor&>(static_cast<System&>(*this)).get()};

 public:
  [[nodiscard]] RngPeripheral<uint32_t>& get() noexcept { return rng_; }
  [[nodiscard]] auto& random_device() noexcept { return get(); }
};

}  // namespace tvsc::hal2::random
