#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <type_traits>

#include "hal/error.h"

namespace tvsc::hal::board_identification {

enum class ResistorTolerance : uint8_t {
  E12_10,    // 10% tolerance
  E24_5,     // 5% tolerance
  E48_2,     // 2% tolerance
  E96_1,     // 1% tolerance
  E96_0_5,   // 0.5% tolerance
  E192_0_1,  // 0.1% tolerance
};

inline constexpr float tolerance(ResistorTolerance tolerance) {
  switch (tolerance) {
    case ResistorTolerance::E12_10:
      return 0.1;
    case ResistorTolerance::E24_5:
      return 0.05;
    case ResistorTolerance::E48_2:
      return 0.02;
    case ResistorTolerance::E96_1:
      return 0.01;
    case ResistorTolerance::E96_0_5:
      return 0.005;
    case ResistorTolerance::E192_0_1:
      return 0.001;
    default:
      error();
  }
}

}  // namespace tvsc::hal::board_identification
