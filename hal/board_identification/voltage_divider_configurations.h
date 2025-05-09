#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>

#include "hal/board_identification/resistor_series.h"
#include "hal/board_identification/tolerance.h"
#include "hal/board_identification/voltage_divider.h"

namespace tvsc::hal::board_identification {

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
    if (voltage_divider.id() != 0 &&
        std::find(result.begin(), end_pos, voltage_divider) == end_pos) {
      result[index++] = voltage_divider;
    }
  }
}

}  // namespace internal

template <size_t NUM_CONFIGURATIONS, uint8_t ADC_RESOLUTION_BITS, ResistorTolerance TOLERANCE>
constexpr std::array<VoltageDivider<ADC_RESOLUTION_BITS, TOLERANCE>, NUM_CONFIGURATIONS>
generate_voltage_divider_configurations(float sum) {
  using VoltageDividerType = VoltageDivider<ADC_RESOLUTION_BITS, TOLERANCE>;
  using ResistorSeriesType = ResistorSeries<TOLERANCE>;

  std::array<VoltageDividerType, NUM_CONFIGURATIONS> result{};
  int level{0};
  size_t index{0};
  size_t prev_index{index - 1};
  while (index < NUM_CONFIGURATIONS && prev_index != index) {
    prev_index = index;
    internal::generate_voltage_divider_configurations_iterative<
        NUM_CONFIGURATIONS, ResistorSeriesType, VoltageDividerType>(sum, result, level, index);
    ++level;
  }

  return result;
}

}  // namespace tvsc::hal::board_identification
