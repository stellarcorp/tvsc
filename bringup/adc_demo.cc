#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>

#include "base/initializer.h"
#include "bringup/dac_demo.h"
#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "system/scheduler.h"
#include "system/task.h"
#include "time/embedded_clock.h"

extern "C" {

__attribute__((section(".status.value"))) uint32_t current_output_value{};
__attribute__((section(".status.value"))) std::array<uint32_t, 1> buffer{};
__attribute__((section(".status.value"))) float absolute_difference{};
__attribute__((section(".status.value"))) float relative_difference{};
__attribute__((section(".status.value"))) volatile bool dma_complete{};
__attribute__((section(".status.value"))) volatile bool dma_error{};

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adc) { dma_complete = true; }

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* adc) { dma_error = true; }
}

namespace tvsc::bringup {

template <uint8_t DAC_CHANNEL = 0>
tvsc::system::System::Task run_adc_demo() {
  using Pinout = tvsc::system::System::PinoutType;
  using namespace std::chrono_literals;

  auto& mcu{tvsc::system::System::mcu()};
  auto& board{tvsc::system::System::board()};
  auto& led_peripheral{board.debug_led()};
  auto& adc_peripheral{mcu.adc()};
  auto& dac_peripheral{mcu.dac()};
  auto dac_gpio_peripheral{mcu.as_peripheral(Pinout::DAC_CHANNEL_PINS[DAC_CHANNEL])};

  // Turn on clocks for the peripherals that we want.
  auto dac{dac_peripheral.access()};
  auto led{led_peripheral.access()};
  auto adc{adc_peripheral.access()};
  auto dac_gpio{dac_gpio_peripheral.access()};

  dac_gpio.set_pin_mode(tvsc::hal::gpio::PinMode::ANALOG);

  led.set_pin_mode(tvsc::hal::gpio::PinMode::OUTPUT_PUSH_PULL, tvsc::hal::gpio::PinSpeed::LOW);

  static constexpr uint8_t RESOLUTION{12};
  static constexpr uint8_t RESOLUTION_SHIFT{RESOLUTION - 8};

  dac.set_resolution(RESOLUTION);
  adc.set_resolution(RESOLUTION);

  uint32_t iteration_counter{0};
  while (true) {
    // Recalibrate after a certain number of conversions.
    static constexpr uint32_t CALIBRATION_FREQUENCY{16};
    if ((iteration_counter % CALIBRATION_FREQUENCY) == 0) {
      adc.calibrate_single_ended_input();

      // Flash slowly after calibration.
      led.write_pin(/* ON */ 1);
      co_yield 500ms;
      led.write_pin(/* OFF */ 0);
      co_yield 500ms;
      led.write_pin(/* ON */ 1);
      co_yield 500ms;
      led.write_pin(/* OFF */ 0);
      co_yield 500ms;
    }

    for (auto v : {256, 0, 1, 2, 4, 8, 16, 32, 64, 128, 256}) {
      current_output_value = (v << RESOLUTION_SHIFT);
      dac.set_value(current_output_value);

      // Set the values in the buffer to a known pattern to check for buffer overrun issues.
      for (size_t i = 0; i < buffer.size(); ++i) {
        buffer[i] = 0xfefefefe;
      }

      dma_complete = false;
      dma_error = false;

      adc.start_single_conversion(dac_gpio.ref(), buffer.data(), buffer.size());

      while (!dma_complete) {
        // Yield while we take the measurement.
        co_yield 5ms;
      }
      adc.reset_after_conversion();

      uint16_t value_read = buffer[0];
      static constexpr float epsilon{0.01};
      relative_difference = std::abs(static_cast<float>(current_output_value) - value_read) /
                            (current_output_value + epsilon);
      absolute_difference = std::abs(static_cast<float>(current_output_value) - value_read);

      if (relative_difference < 0.25f || absolute_difference < (3 << RESOLUTION_SHIFT)) {
        // Success. Short solid.
        led.write_pin(/* ON */ 1);
        co_yield 400ms;
        led.write_pin(/* OFF */ 0);
        co_yield 100ms;
      } else {
        // Failure. Flash frenetically.
        for (int i = 0; i < 2; ++i) {
          led.write_pin(/* ON */ 1);
          co_yield 50ms;
          led.write_pin(/* OFF */ 0);
          co_yield 50ms;
        }
      }
    }
    ++iteration_counter;

    // Clear the state, DAC, and LED and pause between iterations.
    current_output_value = 0;
    absolute_difference = 0.f;
    relative_difference = 0.f;
    dac.set_value(current_output_value);
    dac.clear_value();
    led.write_pin(/* OFF */ 0);
    co_yield 500ms;
  }
}

}  // namespace tvsc::bringup

using namespace tvsc::bringup;
using namespace tvsc::system;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  System::scheduler().add_task(run_adc_demo());
  System::scheduler().start();
}
