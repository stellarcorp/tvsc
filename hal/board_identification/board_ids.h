#pragma once

#include <array>
#include <cstdint>

#include "hal/board_identification/tolerance.h"
#include "hal/board_identification/voltage_divider.h"

namespace tvsc::hal::board_identification {

static constexpr BoardId NUM_BOARD_IDS{17};
static constexpr uint8_t BOARD_ID_ADC_RESOLUTION_BITS{12};
static constexpr ResistorTolerance BOARD_ID_RESISTOR_TOLERANCE{ResistorTolerance::E96_1};
static constexpr float VOLTAGE_DIVIDER_TOTAL_RESISTANCE{25'000.f};

enum class CanonicalBoardIds : BoardId {
  UNKNOWN = 0,
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

const std::array<VoltageDivider<BOARD_ID_ADC_RESOLUTION_BITS, BOARD_ID_RESISTOR_TOLERANCE>,
                 NUM_BOARD_IDS>&
voltage_divider_configurations();

BoardId determine_board_id(uint16_t adc_measurement);

}  // namespace tvsc::hal::board_identification
