#include <cstdint>

#include "base/initializer.h"
#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "hal/peripheral_id.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

using BoardType = tvsc::hal::board::Board;

using namespace tvsc::hal::gpio;

struct Debug final {
  volatile tvsc::hal::PeripheralId timer_id{};
  volatile uint32_t loop_count{};
  volatile uint32_t interrupt_trigger_count{};
  volatile bool compare_match_triggered{};
  volatile bool timer_is_running{};
};
__attribute__((section(".status.value"))) volatile Debug debug_stats{};

void HAL_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *) {
  debug_stats.compare_match_triggered = true;
  ++debug_stats.interrupt_trigger_count;
}

int main(int argc, char *argv[]) {
  tvsc::initialize(&argc, &argv);

  BoardType &board{BoardType::board()};

  auto led{board.debug_led().access()};

  led.set_pin_mode(PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);
  led.write_pin(/* ON */ 1);

  auto timer{board.mcu().sleep_timer().access()};
  debug_stats.timer_id = timer.id();
  while (true) {
    timer.start(500'000);
    const uint32_t old_interrupt_trigger_count{debug_stats.interrupt_trigger_count};
    while (debug_stats.interrupt_trigger_count == old_interrupt_trigger_count) {
      // Wait for the interrupt to trigger.
      ++debug_stats.loop_count;
      debug_stats.timer_is_running = timer.is_running();

      // Block here until an interrupt occurs. Only break out of the loop if it's the
      // LPTIM_CompareMatchCallback().
      __WFI();
    }
    led.toggle_pin();
  }
}
