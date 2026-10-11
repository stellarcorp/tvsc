#pragma once

#include <algorithm>
#include <array>
#include <cstdint>

#include "base/enums.h"
#include "hal/board_identification/board_ids.h"
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

  for (uint64_t i = 1; i < (1UL << (level + 1)) and index < result.size(); i += 2) {
    float low_side_resistor_value = i * interval;
    float high_side_resistor_value = sum - low_side_resistor_value;

    auto try_add_divider = [&](float high_side_value, float low_side_value) {
      if (index >= result.size()) return;

      auto voltage_divider{
          VoltageDividerType{ResistorSeriesType::find_closest_resistor(high_side_value),
                             ResistorSeriesType::find_closest_resistor(low_side_value)}};

      const auto voltage_divider_id{voltage_divider.id()};
      const auto end_pos{result.begin() + index};

      if (voltage_divider_id != cast_to_underlying_type(CanonicalBoardIds::UNKNOWN) and
          std::find_if(result.begin(), end_pos, [voltage_divider_id](const auto& existing) {
            return existing.id() == voltage_divider_id;
          }) == end_pos) {
        result[index++] = voltage_divider;
      }
    };

    // Add the primary configuration
    try_add_divider(high_side_resistor_value, low_side_resistor_value);

    // Add the complementary configuration with the high- and low-side resistors exchanged.
    // Note that if the resistors have the same value, the divider will generate the same id as the
    // primary and this variant won't be added. An explicit check to see if they are the same would
    // involve comparing floats for equality. As this code gets run once every few months, we ignore
    // the performance hit.
    try_add_divider(low_side_resistor_value, high_side_resistor_value);
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
  while (index < NUM_CONFIGURATIONS and prev_index != index) {
    prev_index = index;
    internal::generate_voltage_divider_configurations_iterative<
        NUM_CONFIGURATIONS, ResistorSeriesType, VoltageDividerType>(sum, result, level, index);
    ++level;
  }

  return result;
}

}  // namespace tvsc::hal::board_identification
