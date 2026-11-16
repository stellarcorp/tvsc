#pragma once

#include <cmath>
#include <cstdint>

#include "hal2/board_identification/resistor_series.h"

namespace tvsc::hal2::board_identification {

using BoardId = uint8_t;

template <uint8_t ADC_RESOLUTION_BITS, ResistorTolerance TOLERANCE, float GUARD_BAND_FACTOR = 5.f>
class VoltageDivider final {
 private:
  static constexpr uint8_t BOARD_ID_RESOLUTION_BITS{8 * sizeof(BoardId)};

  [[nodiscard]] static constexpr float compute_voltage_division(float high_resistor_value,
                                                                float low_resistor_value) noexcept {
    return low_resistor_value / (high_resistor_value + low_resistor_value);
  }

  [[nodiscard]] static constexpr uint16_t expected_adc_measurement_value(
      float high_resistor_value, float low_resistor_value) noexcept {
    constexpr uint32_t MAX_VALUE{(1U << ADC_RESOLUTION_BITS) - 1};
    const float voltage_ratio{compute_voltage_division(high_resistor_value, low_resistor_value)};
    return static_cast<uint16_t>(std::round(voltage_ratio * MAX_VALUE));
  }

  float high_resistor_value_{};
  float low_resistor_value_{};

 public:
  constexpr VoltageDivider() noexcept = default;
  constexpr VoltageDivider(float high_resistor_value, float low_resistor_value) noexcept
      : high_resistor_value_(high_resistor_value), low_resistor_value_(low_resistor_value) {}

  constexpr VoltageDivider(const VoltageDivider& rhs) noexcept
      : high_resistor_value_(rhs.high_resistor_value_),
        low_resistor_value_(rhs.low_resistor_value_) {}

  constexpr VoltageDivider& operator=(const VoltageDivider& rhs) noexcept {
    high_resistor_value_ = rhs.high_resistor_value_;
    low_resistor_value_ = rhs.low_resistor_value_;
    return *this;
  }

  [[nodiscard]] constexpr uint16_t min_adc_measurement_value() const noexcept {
    const float max_high_resistor_value{high_resistor_value_ *
                                        (1.f + GUARD_BAND_FACTOR * tolerance(TOLERANCE))};
    const float min_low_resistor_value{low_resistor_value_ *
                                       (1.f - GUARD_BAND_FACTOR * tolerance(TOLERANCE))};
    return expected_adc_measurement_value(max_high_resistor_value, min_low_resistor_value);
  }

  [[nodiscard]] constexpr uint16_t max_adc_measurement_value() const noexcept {
    const float min_high_resistor_value{high_resistor_value_ *
                                        (1.f - GUARD_BAND_FACTOR * tolerance(TOLERANCE))};
    const float max_low_resistor_value{low_resistor_value_ *
                                       (1.f + GUARD_BAND_FACTOR * tolerance(TOLERANCE))};
    return expected_adc_measurement_value(min_high_resistor_value, max_low_resistor_value);
  }

  [[nodiscard]] constexpr uint16_t ideal_adc_measurement_value() const noexcept {
    return expected_adc_measurement_value(high_resistor_value_, low_resistor_value_);
  }

  [[nodiscard]] constexpr bool is_match(uint16_t adc_measurement_value) const noexcept {
    return min_adc_measurement_value() <= adc_measurement_value and
           adc_measurement_value <= max_adc_measurement_value();
  }

  [[nodiscard]] constexpr BoardId id() const noexcept {
    if constexpr (ADC_RESOLUTION_BITS > BOARD_ID_RESOLUTION_BITS) {
      return static_cast<BoardId>(ideal_adc_measurement_value() >>
                                  (ADC_RESOLUTION_BITS - BOARD_ID_RESOLUTION_BITS));
    } else {
      return static_cast<BoardId>(ideal_adc_measurement_value());
    }
  }

  [[nodiscard]] constexpr float high_resistor_value() const noexcept {
    return high_resistor_value_;
  }
  [[nodiscard]] constexpr float low_resistor_value() const noexcept { return low_resistor_value_; }

  [[nodiscard]] constexpr float source_impedance() const noexcept {
    return 1.f / (1.f / high_resistor_value_ + 1.f / low_resistor_value_);
  }
};

}  // namespace tvsc::hal2::board_identification
