#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

#include "base/initializer.h"
#include "bits/bits.h"
#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"
#include "time/embedded_clock.h"

extern "C" {

alignas(uint32_t) __attribute__((section(".status.value"))) volatile uint8_t board_id{};
alignas(uint32_t) __attribute__((section(".status.value"))) volatile int16_t id_error{};
__attribute__((section(".status.value"))) std::array<uint32_t, 1> buffer{};
__attribute__((section(".status.value"))) volatile bool dma_complete{};
__attribute__((section(".status.value"))) volatile bool dma_error{};

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adc) { dma_complete = true; }

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* adc) { dma_error = true; }
}

namespace tvsc::bringup {

template <typename T, size_t SIZE, T start = T{}, T delta = T{1}>
inline static constexpr std::array<T, SIZE> create_array_of_incremented_values() {
  std::array<T, SIZE> result{};
  for (size_t i = 0; i < SIZE; ++i) {
    result[i] = start + i * delta;
  }
  return result;
}

template <uint8_t ADC_RESOLUTION>
[[nodiscard]] std::tuple<uint8_t, int16_t> find_closest_board_id(uint16_t measured_value) {
  static constexpr uint8_t MIN_ID{1};
  static constexpr uint8_t MAX_ID{6};
  static constexpr uint8_t NUM_IDS{MAX_ID + 1 - MIN_ID};
  static constexpr uint8_t NUM_ID_BITS{tvsc::bits::bit_width(MAX_ID)};
  static constexpr std::array<uint8_t, NUM_IDS> BOARD_IDS{
      create_array_of_incremented_values<uint8_t, NUM_IDS, MIN_ID>()};

  uint8_t result{};
  int16_t difference{std::numeric_limits<int16_t>::max()};
  const int16_t signed_adc_value{static_cast<int16_t>(measured_value)};
  for (auto id : BOARD_IDS) {
    auto current_difference =
        std::abs(signed_adc_value - (static_cast<uint16_t>(id) << (ADC_RESOLUTION - NUM_ID_BITS)));
    if (current_difference < difference) {
      result = id;
      difference = current_difference;
    }
  }
  return std::make_tuple(result, difference);
}

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

template <typename ClockType>
tvsc::scheduler::Task<ClockType> read_board_id(BoardType& board) {
  using namespace std::chrono_literals;
  auto& gpio_id_power_peripheral{board.gpio<BoardType::BOARD_ID_POWER_PORT>()};
  auto& gpio_id_sense_peripheral{board.gpio<BoardType::BOARD_ID_SENSE_PORT>()};
  auto& gpio_debug_led_peripheral{board.gpio<BoardType::DEBUG_LED_PORT>()};
  auto& adc_peripheral{board.adc()};

  // Turn on clocks for the peripherals that we want.
  auto gpio_id_power{gpio_id_power_peripheral.access()};
  auto gpio_id_sense{gpio_id_sense_peripheral.access()};
  auto gpio_debug_led{gpio_debug_led_peripheral.access()};
  auto adc{adc_peripheral.access()};

  gpio_id_power.set_pin_mode(BoardType::BOARD_ID_POWER_PIN,
                             tvsc::hal::gpio::PinMode::OUTPUT_PUSH_PULL,
                             tvsc::hal::gpio::PinSpeed::LOW);

  gpio_id_sense.set_pin_mode(BoardType::BOARD_ID_SENSE_PIN, tvsc::hal::gpio::PinMode::ANALOG);

  gpio_debug_led.set_pin_mode(BoardType::DEBUG_LED_PIN, tvsc::hal::gpio::PinMode::OUTPUT_PUSH_PULL,
                              tvsc::hal::gpio::PinSpeed::LOW);

  static constexpr uint8_t RESOLUTION{8};
  adc.set_resolution(RESOLUTION);
  adc.use_data_align_left();

  uint32_t iteration_counter{0};
  while (true) {
    // Recalibrate after a certain number of conversions.
    static constexpr uint32_t CALIBRATION_FREQUENCY{1024};
    if ((iteration_counter % CALIBRATION_FREQUENCY) == 0) {
      adc.calibrate_single_ended_input();

      // Flash slowly after calibration.
      gpio_debug_led.write_pin(BoardType::DEBUG_LED_PIN, 1);
      co_yield 500ms;
      gpio_debug_led.write_pin(BoardType::DEBUG_LED_PIN, 0);
      co_yield 500ms;
      gpio_debug_led.write_pin(BoardType::DEBUG_LED_PIN, 1);
      co_yield 500ms;
      gpio_debug_led.write_pin(BoardType::DEBUG_LED_PIN, 0);
      co_yield 500ms;
    }

    // Turn on the power to the board id circuitry.
    gpio_id_power.write_pin(BoardType::BOARD_ID_POWER_PIN, 1);
    // Let the circuit settle.
    co_yield 1ms;

    // Set the values in the buffer to a known pattern to check for buffer overrun issues.
    for (size_t i = 0; i < buffer.size(); ++i) {
      buffer[i] = 0xfefefefe;
    }

    dma_complete = false;
    dma_error = false;

    adc.start_single_conversion({BoardType::BOARD_ID_SENSE_PORT, BoardType::BOARD_ID_SENSE_PIN},
                                buffer.data(), buffer.size());

    while (!dma_complete) {
      // Yield while we take the measurement.
      co_yield 5ms;
    }
    adc.reset_after_conversion();

    // Turn off the power to the board id circuitry.
    gpio_id_power.write_pin(BoardType::BOARD_ID_POWER_PIN, 0);

    uint16_t value_read = buffer[0];
    std::tie(board_id, id_error) = find_closest_board_id<RESOLUTION>(value_read);

    // Clear the LED and pause between iterations.
    if (id_error < 16) {
      for (uint8_t i = 0; i < 2; ++i) {
        gpio_debug_led.toggle_pin(BoardType::DEBUG_LED_PIN);
        co_yield 250ms;
        gpio_debug_led.toggle_pin(BoardType::DEBUG_LED_PIN);
        co_yield 250ms;
      }
    } else {
      for (uint8_t i = 0; i < 8; ++i) {
        gpio_debug_led.toggle_pin(BoardType::DEBUG_LED_PIN);
        co_yield 50ms;
        gpio_debug_led.toggle_pin(BoardType::DEBUG_LED_PIN);
        co_yield 50ms;
      }
    }

    gpio_debug_led.write_pin(BoardType::DEBUG_LED_PIN, 0);
    co_yield 1000ms;
    ++iteration_counter;
  }
}

}  // namespace tvsc::bringup

using namespace tvsc::bringup;
using namespace tvsc::scheduler;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  BoardType& board{BoardType::board()};

  Scheduler<ClockType, 4 /*QUEUE_SIZE*/> scheduler{board.rcc()};
  scheduler.add_task(read_board_id<ClockType>(board));
  scheduler.start();
}
