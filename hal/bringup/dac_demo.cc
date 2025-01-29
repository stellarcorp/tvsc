#include <cstring>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"

using BoardType = tvsc::hal::board::Board;

template <uint8_t DAC_CHANNEL>
void run_demo(BoardType& board) {
  uint32_t dac_8bit_values[] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256};
  uint32_t dac_12bit_values[] = {0, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
  uint32_t dac_16bit_values[] = {0, 256, 512, 1024, 2048, 4096, 8192, 16636, 32768, 65536};

  auto& dac{board.dac()};
  auto& clock{board.clock()};

  {
    auto& dac_out_gpio{board.gpio<BoardType::DAC_PORTS[DAC_CHANNEL]>()};
    dac_out_gpio.set_pin_mode(BoardType::DAC_PINS[DAC_CHANNEL], tvsc::hal::gpio::PinMode::ANALOG);
  }

  dac.set_resolution(8, DAC_CHANNEL);
  for (const auto& v : dac_8bit_values) {
    dac.set_value(v, DAC_CHANNEL);
    clock.sleep_ms(100);
  }

  dac.clear_value(DAC_CHANNEL);
  clock.sleep_ms(500);

  dac.set_resolution(12, DAC_CHANNEL);
  for (const auto& v : dac_12bit_values) {
    dac.set_value(v, DAC_CHANNEL);
    clock.sleep_ms(100);
  }

  dac.clear_value(DAC_CHANNEL);
  clock.sleep_ms(500);

  dac.set_resolution(16, DAC_CHANNEL);
  for (const auto& v : dac_16bit_values) {
    dac.set_value(v, DAC_CHANNEL);
    clock.sleep_ms(100);
  }

  dac.clear_value(DAC_CHANNEL);
  clock.sleep_ms(500);
}

int main() {
  BoardType board{};

  // Turn on clocks for the peripherals that we want.
  board.rcc().enable_dac_clock();

  if constexpr (BoardType::NUM_DAC_CHANNELS == 1) {
    while (true) {
      run_demo<0>(board);
    }
  } else if constexpr (BoardType::NUM_DAC_CHANNELS == 2) {
    while (true) {
      run_demo<0>(board);
      run_demo<1>(board);
    }
  }
}
