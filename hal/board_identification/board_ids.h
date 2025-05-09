#pragma once

#include <cstdint>

#include "hal/board_identification/resistors.h"

namespace tvsc::hal::board_identification {

static constexpr uint8_t BOARD_ID_RESOLUTION_BITS{8};
static constexpr uint8_t NUM_BOARD_IDS{16};
static constexpr uint8_t BOARD_ID_ADC_RESOLUTION_BITS{8};
static constexpr ResistorTolerance BOARD_ID_RESISTOR_TOLERANCE{ResistorTolerance::E96_1};
static constexpr float VOLTAGE_DIVIDER_TOTAL_RESISTANCE{25'000.f};

enum class BoardIds : uint8_t {
  POWER_BOARD = 128,
  MAGNETORQUER_CONTROL_BOARD_ZENITH = 63,
  MAGNETORQUER_CONTROL_BOARD_NADIR = 192,
  MAGNETORQUER_CONTROL_BOARD_PORT = 32,
  MAGNETORQUER_CONTROL_BOARD_STARBOARD = 94,
  COMMS_BOARD_1 = 161,
  COMMS_BOARD_2 = 223,
};

}  // namespace tvsc::hal::board_identification
