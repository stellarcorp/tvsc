#include <cstring>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "hal/scheduler/scheduler.h"
#include "hal/scheduler/task.h"
#include "hal/time/clock.h"

using BoardType = tvsc::hal::board::Board;

using namespace tvsc::hal::gpio;
using namespace tvsc::hal::scheduler;
using namespace tvsc::hal::time;

template <uint64_t DELAY_MS = 500>
Task blink(Clock& clock, Gpio& gpio, Pin pin) {
  while (true) {
    gpio.toggle_pin(pin);
    co_yield(clock.current_time_millis() + DELAY_MS) * 1000;
  }
}

int main() {
  BoardType board{};

  // Turn on clocks for the GPIO ports that we want.
  board.rcc().enable_gpio_port_clock(BoardType::GREEN_LED_PORT);

  auto& gpio{board.gpio<BoardType::GREEN_LED_PORT>()};
  gpio.set_pin_mode(BoardType::GREEN_LED_PIN, PinMode::OUTPUT_PUSH_PULL);
  auto& clock{board.clock()};

  Scheduler<4> scheduler{clock};
  Task task{blink(clock, gpio, BoardType::GREEN_LED_PIN)};
  scheduler.add_task(task);
  scheduler.start();
}
