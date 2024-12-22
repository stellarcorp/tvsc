#pragma once

#include <cstdint>

namespace tvsc::hal::boards {

enum class Boards : uint8_t {
  LINUX_X86_64_GENERAL,
  NUCLEO_STM32H743ZI,
  TEENSY_40,
  TEENSY_41,
};

}
