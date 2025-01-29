#include <cstring>
#include <limits>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "hal/scheduler/scheduler.h"
#include "hal/scheduler/task.h"
#include "hal/time/clock.h"

using BoardType = tvsc::hal::board::Board;

using namespace tvsc::hal::gpio;
using namespace tvsc::hal::scheduler;
using namespace tvsc::hal::time;

static constexpr size_t QUEUE_SIZE{4};

template <uint64_t DELAY_MS = 500,
          uint64_t DURATION_MS = 10UL * 1000 * 365 * 24 * 60 * 60 *
                                 1000 /* ten thousand years in milliseconds */>
Task blink(Clock& clock, Gpio& gpio, Pin pin) {
  gpio.set_pin_mode(pin, PinMode::OUTPUT_PUSH_PULL);
  const uint64_t stop_time_ms{clock.current_time_millis() + DURATION_MS};

  gpio.write_pin(pin, 0);
  while (clock.current_time_millis() < stop_time_ms) {
    gpio.toggle_pin(pin);
    co_yield 1000 * (clock.current_time_millis() + DELAY_MS);
  }
  gpio.write_pin(pin, 0);
  co_return;
}

int main() {
  BoardType board{};

  auto& clock{board.clock()};

  Scheduler<QUEUE_SIZE> scheduler{clock};
  if constexpr (BoardType::NUM_USER_LEDS == 1) {
    board.rcc().enable_gpio_port_clock(BoardType::GREEN_LED_PORT);
    auto& gpio{board.gpio<BoardType::GREEN_LED_PORT>()};
    Task task{blink(clock, gpio, BoardType::GREEN_LED_PIN)};
    scheduler.add_task(task);

    scheduler.start();
  } else if constexpr (BoardType::NUM_USER_LEDS == 3) {
    static constexpr uint64_t BASE_DURATION_MS{200};
    board.rcc().enable_gpio_port_clock(BoardType::GREEN_LED_PORT);
    auto& gpio{board.gpio<BoardType::GREEN_LED_PORT>()};
    Task task{blink<4 * BASE_DURATION_MS>(clock, gpio, BoardType::GREEN_LED_PIN)};
    scheduler.add_task(task);

    board.rcc().enable_gpio_port_clock(BoardType::YELLOW_LED_PORT);
    auto& gpio2{board.gpio<BoardType::YELLOW_LED_PORT>()};
    Task task2{blink<3 * BASE_DURATION_MS>(clock, gpio2, BoardType::YELLOW_LED_PIN)};
    scheduler.add_task(task2);

    board.rcc().enable_gpio_port_clock(BoardType::RED_LED_PORT);
    auto& gpio3{board.gpio<BoardType::RED_LED_PORT>()};
    Task task3{blink<2 * BASE_DURATION_MS>(clock, gpio3, BoardType::RED_LED_PIN)};
    scheduler.add_task(task3);

    scheduler.start();
  }
}
