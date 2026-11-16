#pragma once

#include <array>
#include <cstdint>

#include "base/enums.h"
#include "hal2/board_identification/resistor_series.h"
#include "hal2/board_identification/voltage_divider.h"
#include "hal2/board_identification/voltage_divider_configurations.h"

namespace tvsc::hal2::board_identification {

static constexpr BoardId NUM_BOARD_IDS{17};
static constexpr uint8_t BOARD_ID_ADC_RESOLUTION_BITS{12};
static constexpr ResistorTolerance BOARD_ID_RESISTOR_TOLERANCE{ResistorTolerance::E96_1};
static constexpr float VOLTAGE_DIVIDER_TOTAL_RESISTANCE{25'000.f};

enum class CanonicalBoardIds : BoardId {
  UNKNOWN = UNKNOWN_BOARD_ID,
  POWER_BOARD = 0x80,
  MAGNETORQUER_CONTROL_BOARD_1 = 0x3f,
  MAGNETORQUER_CONTROL_BOARD_2 = 0xc0,
  MAGNETORQUER_CONTROL_BOARD_3 = 0x5e,
  MAGNETORQUER_CONTROL_BOARD_4 = 0xa1,
  COMMS_BOARD_1 = 0x20,
  COMMS_BOARD_2 = 0xdf,
  BATTERY_BOARD_1 = 0x10,
  BATTERY_BOARD_2 = 0xef,
};
[[maybe_unused]] static constexpr std::array VALID_BOARD_IDS{
    // The UNKNOWN board id is excluded from the list. These are the valid board ids.
    CanonicalBoardIds::POWER_BOARD,
    CanonicalBoardIds::MAGNETORQUER_CONTROL_BOARD_1,
    CanonicalBoardIds::MAGNETORQUER_CONTROL_BOARD_2,
    CanonicalBoardIds::MAGNETORQUER_CONTROL_BOARD_3,
    CanonicalBoardIds::MAGNETORQUER_CONTROL_BOARD_4,
    CanonicalBoardIds::COMMS_BOARD_1,
    CanonicalBoardIds::COMMS_BOARD_2,
    CanonicalBoardIds::BATTERY_BOARD_1,
    CanonicalBoardIds::BATTERY_BOARD_2,
};

/**
 * Helper method that bakes in all of the settings when generating the voltage divider
 * configurations used to set the board ids.
 */
[[nodiscard]] inline constexpr auto board_id_voltage_dividers() noexcept {
  return generate_voltage_divider_configurations<NUM_BOARD_IDS, BOARD_ID_ADC_RESOLUTION_BITS,
                                                 BOARD_ID_RESISTOR_TOLERANCE>(
      VOLTAGE_DIVIDER_TOTAL_RESISTANCE);
}

[[nodiscard]] inline constexpr BoardId determine_board_id(uint16_t adc_measurement) noexcept {
  BoardId result{cast_to_underlying_type(CanonicalBoardIds::UNKNOWN)};

  for (const auto& configuration : board_id_voltage_dividers()) {
    if (configuration.is_match(adc_measurement)) {
      result = configuration.id();
      break;
    }
  }

  // Due to a mistake in the board design, the comms board in Cheerful Coffee has 221k ohm
  // resistors in the board identification voltage dividers. It should have had 22k1 ohm resistors
  // instead. The section of code here works around this issue and allows the boards to correctly
  // identify themselves.
#ifdef CHEERFUL_COFFEE
  if (result == cast_to_underlying_type(CanonicalBoardIds::UNKNOWN)) {
    constexpr VoltageDivider<BOARD_ID_ADC_RESOLUTION_BITS, BOARD_ID_RESISTOR_TOLERANCE>
        ERRANT_COMMS1_BOARD{221'000.f, 3'160.f};
    constexpr VoltageDivider<BOARD_ID_ADC_RESOLUTION_BITS, BOARD_ID_RESISTOR_TOLERANCE>
        ERRANT_COMMS2_BOARD{3'160.f, 221'000.f};

    if (ERRANT_COMMS1_BOARD.is_match(adc_measurement)) {
      result = cast_to_underlying_type(CanonicalBoardIds::COMMS_BOARD_1);
    } else if (ERRANT_COMMS2_BOARD.is_match(adc_measurement)) {
      result = cast_to_underlying_type(CanonicalBoardIds::COMMS_BOARD_2);
    }
  }
#endif

  return result;
}

}  // namespace tvsc::hal2::board_identification
