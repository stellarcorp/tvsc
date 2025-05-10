#pragma once

#include <cstdint>

#include "hal/board_identification/tolerance.h"

namespace tvsc::hal::board_identification {

using BoardId = uint8_t;

template <uint8_t ADC_RESOLUTION_BITS, ResistorTolerance TOLERANCE>
class VoltageDivider final {
 private:
  static constexpr float TOLERANCE_FACTOR{5.f};
  static constexpr uint8_t BOARD_ID_RESOLUTION_BITS{8 * sizeof(BoardId)};

  static constexpr float compute_voltage_division(float high_resistor_value,
                                                  float low_resistor_value) {
    return low_resistor_value / (high_resistor_value + low_resistor_value);
  }

  static constexpr uint16_t expected_adc_measurement_value(float high_resistor_value,
                                                           float low_resistor_value) {
    constexpr uint16_t MAX_VALUE{static_cast<uint16_t>(1U << ADC_RESOLUTION_BITS)};
    return static_cast<uint16_t>(compute_voltage_division(high_resistor_value, low_resistor_value) *
                                 MAX_VALUE);
  }

  float high_resistor_value_{};
  float low_resistor_value_{};

 public:
  constexpr VoltageDivider() = default;
  constexpr VoltageDivider(float high_resistor_value, float low_resistor_value)
      : high_resistor_value_(high_resistor_value), low_resistor_value_(low_resistor_value) {}

  constexpr VoltageDivider(const VoltageDivider& rhs)
      : high_resistor_value_(rhs.high_resistor_value_),
        low_resistor_value_(rhs.low_resistor_value_) {}

  constexpr VoltageDivider& operator=(const VoltageDivider& rhs) {
    high_resistor_value_ = rhs.high_resistor_value_;
    low_resistor_value_ = rhs.low_resistor_value_;
    return *this;
  }

  constexpr bool operator==(const VoltageDivider& rhs) const { return id() == rhs.id(); }

  constexpr uint16_t min_adc_measurement_value() const {
    const float max_high_resistor_value{high_resistor_value_ *
                                        (1.f + TOLERANCE_FACTOR * tolerance(TOLERANCE))};
    const float min_low_resistor_value{low_resistor_value_ *
                                       (1.f - TOLERANCE_FACTOR * tolerance(TOLERANCE))};
    return expected_adc_measurement_value(max_high_resistor_value, min_low_resistor_value);
  }

  constexpr uint16_t max_adc_measurement_value() const {
    const float min_high_resistor_value{high_resistor_value_ *
                                        (1.f - TOLERANCE_FACTOR * tolerance(TOLERANCE))};
    const float max_low_resistor_value{low_resistor_value_ *
                                       (1.f + TOLERANCE_FACTOR * tolerance(TOLERANCE))};
    return expected_adc_measurement_value(min_high_resistor_value, max_low_resistor_value);
  }

  constexpr uint16_t ideal_adc_measurement_value() const {
    return expected_adc_measurement_value(high_resistor_value_, low_resistor_value_);
  }

  constexpr BoardId id() const {
    if constexpr (ADC_RESOLUTION_BITS > BOARD_ID_RESOLUTION_BITS) {
      return static_cast<BoardId>(ideal_adc_measurement_value() >>
                                  (ADC_RESOLUTION_BITS - BOARD_ID_RESOLUTION_BITS));
    } else {
      return static_cast<BoardId>(ideal_adc_measurement_value());
    }
  }

  constexpr float high_resistor_value() const { return high_resistor_value_; }
  constexpr float low_resistor_value() const { return low_resistor_value_; }

  constexpr float source_impedance() const {
    return 1.f / (1.f / high_resistor_value_ + 1.f / low_resistor_value_);
  }
};

}  // namespace tvsc::hal::board_identification
