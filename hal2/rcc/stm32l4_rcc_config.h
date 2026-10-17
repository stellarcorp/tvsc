#pragma once

#include "hal2/config/facility.h"
#include "hal2/config/system_builder.h"
#include "hal2/config/vector_table.h"
#include "hal2/rcc/rcc.h"
#include "hal2/rcc/stm32l4xx_rcc.h"

namespace tvsc::hal2::rcc {

class Stm32L4RccAccessor final {
 private:
  RccStm32L4xx rcc_{};

 public:
  [[nodiscard]] Rcc& get() noexcept { return rcc_; }
  [[nodiscard]] Rcc& rcc() noexcept { return get(); }
};

struct Stm32L4RccConfiguration final {
  using accessor = Stm32L4RccAccessor;

  static constexpr auto facility() noexcept { return config::Facility::RCC; }
  static constexpr size_t index() noexcept { return 0; }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table) noexcept {
    // No ISRs required.
  }
};
static_assert(ValidPeripheralConfiguration<RccConfiguration>);

class Stm32L4LsiOscillatorAccessor final {
 private:
  LsiOscillatorStm32L4xx lsi_{};

 public:
  [[nodiscard]] Oscillator& get() noexcept { return lsi_; }
  [[nodiscard]] Oscillator& oscillator1() noexcept { return get(); }
  [[nodiscard]] Oscillator& low_power_oscillator() noexcept { return get(); }
};

struct Stm32L4LsiOscillatorConfiguration final {
  using accessor = Stm32L4LsiOscillatorAccessor;

  static constexpr auto facility() noexcept { return config::Facility::OSCILLATOR; }
  static constexpr size_t index() noexcept { return 1; }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table) noexcept {
    // No ISRs required.
  }
};
static_assert(ValidPeripheralConfiguration<LsiOscillatorConfiguration>);

class Stm32L4Hsi48OscillatorAccessor final {
 private:
  Hsi48OscillatorStm32L4xx hsi48_{};

 public:
  [[nodiscard]] Oscillator& get() noexcept { return hsi48_; }
  [[nodiscard]] Oscillator& oscillator2() noexcept { return get(); }
};

struct Stm32L4Hsi48OscillatorConfiguration final {
  using accessor = Stm32L4Hsi48OscillatorAccessor;

  static constexpr auto facility() noexcept { return config::Facility::OSCILLATOR; }
  static constexpr size_t index() noexcept { return 2; }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table) noexcept {
    // No ISRs required.
  }
};
static_assert(ValidPeripheralConfiguration<Hsi48OscillatorConfiguration>);

}  // namespace tvsc::hal2::rcc
