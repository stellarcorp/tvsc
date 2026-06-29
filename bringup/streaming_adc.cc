#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>

#include "base/initializer.h"
#include "bringup/dac_demo.h"
#include "hal/gpio/gpio.h"
#include "system/system.h"

extern "C" {

__attribute__((section(".status.value"))) uint32_t current_output_value{};
__attribute__((section(".status.value"))) std::array<uint32_t, 4> values_read{};
__attribute__((section(".status.value"))) volatile bool dma_complete{};
__attribute__((section(".status.value"))) volatile bool dma_error{};

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adc) { dma_complete = true; }

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* adc) { dma_error = true; }

}  // extern "C"

namespace tvsc::bringup {

using namespace std::chrono_literals;
static constexpr std::chrono::microseconds PERIOD_US{500ms};

/**
 * NOTE: This bringup script is NOT fully functional.
 *
 * We still need to add support for internal timers and use a timer as the trigger for the ADC
 * conversion. We might also want to use the same timer, if possible, to trigger changing the DAC
 * value.
 */
template <uint8_t DAC_CHANNEL = 0>
tvsc::system::System::Task run_adc_demo() {
  using Pinout = tvsc::system::System::PinoutType;

  auto& mcu{tvsc::system::System::mcu()};
  auto& board{tvsc::system::System::board()};
  auto& led_peripheral{board.debug_led()};
  auto& adc_peripheral{mcu.adc()};
  auto& dac_peripheral{mcu.dac()};
  auto& timer_peripheral{mcu.timer2()};

  // Turn on clocks for the peripherals that we want.
  auto dac{dac_peripheral.access()};
  auto led{led_peripheral.access()};
  auto adc{adc_peripheral.access()};
  auto timer{timer_peripheral.access()};
  auto dac_out_gpio{mcu.create_peripheral(Pinout::DAC_CHANNEL_PINS[DAC_CHANNEL]).access()};

  dac_out_gpio.set_pin_mode(tvsc::hal::gpio::PinMode::ANALOG);

  static constexpr uint8_t RESOLUTION{8};
  static constexpr uint8_t RESOLUTION_SHIFT{RESOLUTION - 8};

  dac.set_resolution(RESOLUTION);
  adc.set_resolution(RESOLUTION);

  uint32_t iteration_counter{0};
  while (true) {
    // Recalibrate after a certain number of conversions.
    static constexpr uint32_t CALIBRATION_FREQUENCY{1024};
    if ((iteration_counter % CALIBRATION_FREQUENCY) == 0) {
      // Turn on LED while calibrating.
      led.on();

      adc.calibrate_single_ended_input();

      led.off();
    }

    timer.start(PERIOD_US.count());
    adc.start_conversion_stream(dac_out_gpio.ref(), values_read.data(), values_read.size() * 2,
                                timer);

    dma_complete = false;
    dma_error = false;

    for (auto v : {256, 0, 1, 2, 4, 8, 16, 32, 64, 128}) {
      current_output_value = (v << RESOLUTION_SHIFT);
      dac.set_value(current_output_value);

      // Hold the DAC at this value. The ADC runs in the background and will measure this value
      // asynchronously.
      co_yield PERIOD_US;
    }

    // Stop the ADC.
    adc.reset_after_conversion();

    // Clear the state, DAC, and LED and pause between iterations.
    current_output_value = 0;
    dac.set_value(current_output_value);
    // Hold the DAC at this value.
    co_yield PERIOD_US;
    dac.clear_value();
    ++iteration_counter;
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
