#pragma once

#include <chrono>

#include "hal/can_bus/can_bus.h"
#include "hal/gpio/gpio.h"
#include "hal/led/led.h"
#include "message/message.h"
#include "message/queue.h"
#include "system/system.h"

namespace tvsc::bringup {

template <size_t QUEUE_SIZE, size_t NUM_HANDLERS>
tvsc::system::System::Task can_bus_receive(
    tvsc::hal::can_bus::CanBusPeripheral& can_peripheral,
    tvsc::message::CanBusMessageQueue<QUEUE_SIZE, NUM_HANDLERS>& queue,
    tvsc::hal::led::LedPeripheral& led_peripheral) {
  using namespace std::chrono_literals;
  using namespace tvsc::hal::can_bus;
  using namespace tvsc::hal::gpio;

  {
    // Turn on the LED GPIO.
    auto led{led_peripheral.access()};

    for (int i = 0; i < 5; ++i) {
      led.on();
      co_yield 25ms;
      led.off();
      co_yield 25ms;
    }
  }

  // Turn on clock for the CAN bus.
  auto can{can_peripheral.access()};
  while (true) {
    while (can.available_message_count(RxFifo::FIFO_ZERO) > 0) {
      // Turn on the LED GPIO.
      auto led{led_peripheral.access()};

      tvsc::message::CanBusMessage message{};
      if (can.receive(RxFifo::FIFO_ZERO, message) && queue.enqueue(message)) {
        led.on();
        co_yield 50ms;
        led.off();
      } else {
        for (int i = 0; i < 5; ++i) {
          led.on();
          co_yield 25ms;
          led.off();
          co_yield 25ms;
        }
      }
    }
    co_yield 10ms;
  }
}

}  // namespace tvsc::bringup
