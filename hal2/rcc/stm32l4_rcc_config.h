#pragma once

#include <array>

#include "hal2/config/facility.h"
#include "hal2/config/system_builder.h"
#include "hal2/config/vector_table.h"
#include "hal2/rcc/rcc.h"
#include "hal2/rcc/stm32l4xx_rcc.h"

namespace tvsc::hal2::rcc {

template <typename System>
class Stm32L4RccAccessor {
 private:
  RccStm32L4xx rcc_{};

 public:
  [[nodiscard]] Rcc& get() noexcept { return rcc_; }
  [[nodiscard]] Rcc& rcc() noexcept { return get(); }
};

template <typename System>
class Stm32L4LsiOscillatorAccessor {
 private:
  LsiOscillatorStm32L4xx lsi_{};

 public:
  [[nodiscard]] Oscillator& get() noexcept { return lsi_; }
  [[nodiscard]] Oscillator& oscillator1() noexcept { return get(); }
  [[nodiscard]] Oscillator& low_power_oscillator() noexcept { return get(); }
};

template <typename System>
class Stm32L4Hsi48OscillatorAccessor {
 private:
  Hsi48OscillatorStm32L4xx hsi48_{};

 public:
  [[nodiscard]] Oscillator& get() noexcept { return hsi48_; }
  [[nodiscard]] Oscillator& oscillator2() noexcept { return get(); }
};

}  // namespace tvsc::hal2::rcc
