#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>

#include "hal/board_identification/resistors.h"

namespace tvsc::hal::board_identification {

template <uint8_t ADC_RESOLUTION_BITS, uint8_t BOARD_ID_RESOLUTION_BITS,
          ResistorTolerance TOLERANCE>
class VoltageDivider final {
 private:
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
    const float max_high_resistor_value{high_resistor_value_ * (1.f + tolerance(TOLERANCE))};
    const float min_low_resistor_value{low_resistor_value_ * (1.f - tolerance(TOLERANCE))};
    return expected_adc_measurement_value(max_high_resistor_value, min_low_resistor_value);
  }

  constexpr uint16_t max_adc_measurement_value() const {
    const float min_high_resistor_value{high_resistor_value_ * (1.f - tolerance(TOLERANCE))};
    const float max_low_resistor_value{low_resistor_value_ * (1.f + tolerance(TOLERANCE))};
    return expected_adc_measurement_value(min_high_resistor_value, max_low_resistor_value);
  }

  constexpr uint16_t ideal_adc_measurement_value() const {
    return expected_adc_measurement_value(high_resistor_value_, low_resistor_value_);
  }

  constexpr uint8_t id() const {
    return static_cast<uint8_t>(ideal_adc_measurement_value() >>
                                (ADC_RESOLUTION_BITS - BOARD_ID_RESOLUTION_BITS));
  }

  constexpr float high_resistor_value() const { return high_resistor_value_; }
  constexpr float low_resistor_value() const { return low_resistor_value_; }
};

namespace internal {

template <size_t NUM_CONFIGURATIONS, typename ResistorSeriesType, typename VoltageDividerType>
constexpr void generate_voltage_divider_configurations_iterative(
    float sum, std::array<VoltageDividerType, NUM_CONFIGURATIONS>& result, int level,
    size_t& index) {
  const float interval{sum / (1 << (level + 1))};

  for (int i = 1; i < (1 << (level + 1)) && index < result.size(); i += 2) {
    float low_side_resistor_value = i * interval;
    float high_side_resistor_value = sum - low_side_resistor_value;
    auto voltage_divider{
        VoltageDividerType{ResistorSeriesType::find_closest_resistor(high_side_resistor_value),
                           ResistorSeriesType::find_closest_resistor(low_side_resistor_value)}};
    const auto end_pos{result.begin() + index};
    if (std::find(result.begin(), end_pos, voltage_divider) == end_pos) {
      result[index++] = voltage_divider;
    }
  }
}

}  // namespace internal

template <size_t NUM_CONFIGURATIONS, uint8_t ADC_RESOLUTION_BITS, uint8_t BOARD_ID_RESOLUTION_BITS,
          ResistorTolerance TOLERANCE>
constexpr std::array<VoltageDivider<ADC_RESOLUTION_BITS, BOARD_ID_RESOLUTION_BITS, TOLERANCE>,
                     NUM_CONFIGURATIONS>
generate_voltage_divider_configurations(float sum) {
  using VoltageDividerType =
      VoltageDivider<ADC_RESOLUTION_BITS, BOARD_ID_RESOLUTION_BITS, TOLERANCE>;
  using ResistorSeriesType = ResistorSeries<TOLERANCE>;

  std::array<VoltageDividerType, NUM_CONFIGURATIONS> result{};
  int level{0};
  size_t index{0};
  while (index < NUM_CONFIGURATIONS) {
    internal::generate_voltage_divider_configurations_iterative<
        NUM_CONFIGURATIONS, ResistorSeriesType, VoltageDividerType>(sum, result, level, index);
    ++level;
  }

  return result;
}

static constexpr uint8_t BOARD_ID_RESOLUTION_BITS{8};
static constexpr uint8_t NUM_BOARD_IDS{64};
static constexpr uint8_t BOARD_ID_ADC_RESOLUTION_BITS{8};
static constexpr ResistorTolerance BOARD_ID_RESISTOR_TOLERANCE{ResistorTolerance::E96_1};
static constexpr float VOLTAGE_DIVIDER_TOTAL_RESISTANCE{25'000.f};

static constexpr auto BOARD_ID_VOLTAGE_DIVIDER_CONFIGURATIONS =
    generate_voltage_divider_configurations<NUM_BOARD_IDS, BOARD_ID_ADC_RESOLUTION_BITS,
                                            BOARD_ID_RESOLUTION_BITS, BOARD_ID_RESISTOR_TOLERANCE>(
        VOLTAGE_DIVIDER_TOTAL_RESISTANCE);

}  // namespace tvsc::hal::board_identification
