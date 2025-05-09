#pragma once

#include <array>
#include <cstdint>

#include "hal/board_identification/tolerance.h"
#include "hal/board_identification/voltage_divider.h"

namespace tvsc::hal::board_identification {

static constexpr BoardId NUM_BOARD_IDS{15};
static constexpr uint8_t BOARD_ID_ADC_RESOLUTION_BITS{12};
static constexpr ResistorTolerance BOARD_ID_RESISTOR_TOLERANCE{ResistorTolerance::E96_1};
static constexpr float VOLTAGE_DIVIDER_TOTAL_RESISTANCE{25'000.f};

enum class CanonicalBoardIds : BoardId {
  UNKNOWN = 0,
  POWER_BOARD = 128,
  MAGNETORQUER_CONTROL_BOARD_ZENITH = 63,
  MAGNETORQUER_CONTROL_BOARD_NADIR = 192,
  MAGNETORQUER_CONTROL_BOARD_PORT = 32,
  MAGNETORQUER_CONTROL_BOARD_STARBOARD = 94,
  COMMS_BOARD_1 = 161,
  COMMS_BOARD_2 = 223,
};

const std::array<VoltageDivider<BOARD_ID_ADC_RESOLUTION_BITS, BOARD_ID_RESISTOR_TOLERANCE>,
                 NUM_BOARD_IDS>&
voltage_divider_configurations();

BoardId determine_board_id(uint16_t adc_measurement);

}  // namespace tvsc::hal::board_identification
