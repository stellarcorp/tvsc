#pragma once

#include <chrono>
#include <cstdint>

#include "hal/gpio/gpio.h"
#include "system/system.h"

namespace tvsc::bringup {

template <uint8_t DAC_CHANNEL = 0>
tvsc::system::System::Task run_dac_demo(uint32_t& output_value, uint64_t initial_delay_ms = 0) {
  using Pinout = tvsc::system::System::PinoutType;
  using namespace std::chrono_literals;

  static constexpr uint32_t dac_8bit_values[] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256};
  static constexpr uint32_t dac_12bit_values[] = {0, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
  static constexpr uint32_t dac_16bit_values[] = {0,    256,  512,   1024,  2048,
                                                  4096, 8192, 16636, 32768, 65536};

  auto& mcu{tvsc::system::System::mcu()};
  auto& dac_peripheral{mcu.dac()};
  auto dac_out_peripheral{mcu.as_peripheral(Pinout::DAC_CHANNEL_PINS[DAC_CHANNEL])};

  if (initial_delay_ms > 0) {
    co_yield std::chrono::milliseconds{initial_delay_ms};
  }

  // Turn on clocks for the peripherals that we want.
  auto dac_out{dac_out_peripheral.access()};

  dac_out.set_pin_mode(tvsc::hal::gpio::PinMode::ANALOG);

  while (true) {
    for (auto& v : dac_8bit_values) {
      auto dac{dac_peripheral.access()};
      output_value = v;
      dac.set_resolution(8, DAC_CHANNEL);
      dac.set_value(v, DAC_CHANNEL);
      co_yield 50ms;
    }

    co_yield 250ms;

    for (auto& v : dac_12bit_values) {
      auto dac{dac_peripheral.access()};
      dac.set_resolution(12, DAC_CHANNEL);
      dac.set_value(v, DAC_CHANNEL);
      co_yield 50ms;
    }

    co_yield 250ms;

    for (auto& v : dac_16bit_values) {
      auto dac{dac_peripheral.access()};
      dac.set_resolution(16, DAC_CHANNEL);
      dac.set_value(v, DAC_CHANNEL);
      co_yield 50ms;
    }

    co_yield 500ms;
  }
}

}  // namespace tvsc::bringup
