#pragma once

#include "hal2/device.h"
#include "hal2/random/rng.h"
#include "hal2/random/stm32_rng.h"
#include "hal2/rcc/stm32l4_rcc_config.h"

namespace tvsc::hal2::random {

static constexpr size_t HSI48_OSCILLATOR_INDEX{2};

template <typename System, typename DeviceRegistry>
class Stm32RngAccessor {
 private:
  static constexpr DeviceId hsi48_id{Facility::OSCILLATOR_FACILITY,
                                                 HSI48_OSCILLATOR_INDEX};

  using Hsi48OscillatorAccessor = config::accessor_type<System, DeviceRegistry, hsi48_id>;

  Stm32Rng rng_{static_cast<Hsi48OscillatorAccessor&>(static_cast<System&>(*this)).get()};

 public:
  [[nodiscard]] RngDevice<uint32_t>& get() noexcept { return rng_; }
  [[nodiscard]] auto& random_device() noexcept { return get(); }
};

}  // namespace tvsc::hal2::random
