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

namespace internal {
constexpr std::array<uint16_t, 12> E12{100, 120, 150, 180, 220, 270, 330, 390, 470, 560, 680, 820};

constexpr std::array<uint16_t, 24> E24{100, 110, 120, 130, 150, 160, 180, 200, 220, 240,  //
                                       270, 300, 330, 360, 390, 430, 470, 510, 560, 620,  //
                                       680, 750, 820, 910};

constexpr std::array<uint16_t, 48> E48{100, 105, 110, 115, 121, 127, 133, 140, 147, 154,  //
                                       162, 169, 178, 187, 196, 205, 215, 226, 237, 249,  //
                                       261, 274, 287, 301, 316, 332, 348, 365, 383, 402,  //
                                       422, 442, 464, 487, 511, 536, 562, 590, 619, 649,  //
                                       681, 715, 750, 787, 825, 865, 909, 953};

constexpr std::array<uint16_t, 96> E96{100, 102, 105, 107, 110, 113, 115, 118, 121, 124,  //
                                       127, 130, 133, 137, 140, 143, 147, 150, 154, 158,  //
                                       162, 165, 169, 174, 178, 182, 187, 191, 196, 200,  //
                                       205, 210, 215, 221, 226, 232, 237, 243, 249, 255,  //
                                       261, 267, 274, 280, 287, 294, 301, 309, 316, 324,  //
                                       332, 340, 348, 357, 365, 374, 383, 392, 402, 412,  //
                                       422, 432, 442, 453, 464, 475, 487, 499, 511, 523,  //
                                       536, 549, 562, 576, 590, 604, 619, 634, 649, 665,  //
                                       681, 698, 715, 732, 750, 768, 787, 806, 825, 845,  //
                                       866, 887, 909, 931, 953, 976};

constexpr std::array<uint16_t, 192> E192{100, 101, 102, 104, 105, 106, 107, 109, 110, 111,  //
                                         113, 114, 115, 117, 118, 120, 121, 123, 124, 126,  //
                                         127, 129, 130, 132, 133, 135, 137, 138, 140, 142,  //
                                         143, 145, 147, 149, 150, 152, 154, 156, 158, 160,  //
                                         162, 164, 165, 167, 169, 172, 174, 176, 178, 180,  //
                                         182, 184, 187, 189, 191, 193, 196, 198, 200, 203,  //
                                         205, 208, 210, 213, 215, 218, 221, 223, 226, 229,  //
                                         232, 234, 237, 240, 243, 246, 249, 252, 255, 258,  //
                                         261, 264, 267, 271, 274, 277, 280, 284, 287, 291,  //
                                         294, 298, 301, 305, 309, 312, 316, 320, 324, 328,  //
                                         332, 336, 340, 344, 348, 352, 357, 361, 365, 370,  //
                                         374, 379, 383, 388, 392, 397, 402, 407, 412, 417,  //
                                         422, 427, 432, 437, 442, 448, 453, 459, 464, 470,  //
                                         475, 481, 487, 493, 499, 505, 511, 517, 523, 530,  //
                                         536, 542, 549, 556, 562, 569, 576, 583, 590, 597,  //
                                         604, 612, 619, 627, 634, 642, 649, 657, 665, 673,  //
                                         681, 690, 698, 706, 715, 723, 732, 741, 750, 759,  //
                                         768, 777, 787, 796, 806, 816, 825, 835, 845, 856,  //
                                         866, 876, 887, 898, 909, 920, 931, 942, 953, 965,  //
                                         976, 988};

template <ResistorTolerance TOLERANCE>
constexpr const auto& base_series();

template <>
constexpr const auto& base_series<ResistorTolerance::E12_10>() {
  return E12;
}

template <>
constexpr const auto& base_series<ResistorTolerance::E24_5>() {
  return E24;
}

template <>
constexpr const auto& base_series<ResistorTolerance::E48_2>() {
  return E48;
}

template <>
constexpr const auto& base_series<ResistorTolerance::E96_1>() {
  return E96;
}

template <>
constexpr const auto& base_series<ResistorTolerance::E96_0_5>() {
  return E96;
}

template <>
constexpr const auto& base_series<ResistorTolerance::E192_0_1>() {
  return E192;
}

}  // namespace internal

template <ResistorTolerance TOLERANCE>
class ResistorSeries final {
 private:
  static constexpr auto generate_series() {
    constexpr int MIN_DECADE = -2;
    constexpr int MAX_DECADE = 6;
    constexpr auto base_values{internal::base_series<TOLERANCE>()};
    std::array<float, base_values.size() * (MAX_DECADE - MIN_DECADE + 1)> result{};

    size_t index{0};
    for (int decade = MIN_DECADE; decade <= MAX_DECADE; ++decade) {
      float scale = std::pow(10.0f, static_cast<float>(decade));
      for (uint16_t value : base_values) {
        result[index++] = value * scale / 100.0f;
      }
    }

    return result;
  }

  static constexpr auto SERIES_VALUES{generate_series()};

 public:
  constexpr ResistorSeries() = default;

  static constexpr bool series_contains_resistor(float resistor_value) {
    return std::find(SERIES_VALUES.begin(), SERIES_VALUES.end(), resistor_value) !=
           SERIES_VALUES.end();
  }

  static constexpr float find_closest_resistor(float target) {
    size_t low = 0;
    size_t high = SERIES_VALUES.size();

    // Binary search to find the closest value or insertion point
    while (low < high) {
      size_t mid = low + (high - low) / 2;
      if (SERIES_VALUES[mid] < target) {
        low = mid + 1;
      } else {
        high = mid;
      }
    }

    // Now, low is the first element >= target
    if (low == 0) {
      return SERIES_VALUES[0];
    } else if (low == SERIES_VALUES.size()) {
      return SERIES_VALUES.back();
    } else {
      float below = SERIES_VALUES[low - 1];
      float above = SERIES_VALUES[low];
      return (target - below < above - target) ? below : above;
    }
  }
};

}  // namespace tvsc::hal::board_identification
