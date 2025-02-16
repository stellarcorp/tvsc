#pragma once

#include <cstdint>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "hal/scheduler/task.h"

namespace tvsc::hal::bringup {

template <uint8_t DAC_CHANNEL = 0>
scheduler::Task run_dac_demo(board::Board& board, uint32_t& output_value,
                             uint64_t initial_delay_ms = 0) {
  static constexpr uint32_t dac_8bit_values[] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256};
  static constexpr uint32_t dac_12bit_values[] = {0, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
  static constexpr uint32_t dac_16bit_values[] = {0,    256,  512,   1024,  2048,
                                                  4096, 8192, 16636, 32768, 65536};

  using BoardType = board::Board;

  auto& dac_peripheral{board.dac()};
  auto& clock{board.clock()};
  auto& dac_gpio_peripheral{board.gpio<BoardType::DAC_PORTS[DAC_CHANNEL]>()};

  if (initial_delay_ms > 0) {
    co_yield 1000 * (initial_delay_ms + clock.current_time_millis());
  }

  // Turn on clocks for the peripherals that we want.
  auto dac_out_gpio{dac_gpio_peripheral.access()};

  dac_out_gpio.set_pin_mode(BoardType::DAC_PINS[DAC_CHANNEL], gpio::PinMode::ANALOG);

  while (true) {
    for (auto& v : dac_8bit_values) {
      auto dac{dac_peripheral.access()};
      output_value = v;
      dac.set_resolution(8, DAC_CHANNEL);
      dac.set_value(v, DAC_CHANNEL);
      co_yield 50'000 + clock.current_time_micros();
    }

    co_yield 1000 * (250 + clock.current_time_millis());

    for (auto& v : dac_12bit_values) {
      auto dac{dac_peripheral.access()};
      dac.set_resolution(12, DAC_CHANNEL);
      dac.set_value(v, DAC_CHANNEL);
      co_yield 50'000 + clock.current_time_micros();
    }

    co_yield 1000 * (250 + clock.current_time_millis());

    for (auto& v : dac_16bit_values) {
      auto dac{dac_peripheral.access()};
      dac.set_resolution(16, DAC_CHANNEL);
      dac.set_value(v, DAC_CHANNEL);
      co_yield 50'000 + clock.current_time_micros();
    }

    co_yield 1000 * (500 + clock.current_time_millis());
  }
}

}  // namespace tvsc::hal::bringup
