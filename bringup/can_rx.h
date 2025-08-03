#pragma once

#include <chrono>

#include "hal/can_bus/can_bus.h"
#include "message/message.h"
#include "message/queue.h"
#include "scheduler/task.h"

namespace tvsc::bringup {

template <typename ClockType, size_t QUEUE_SIZE, size_t NUM_HANDLERS>
tvsc::scheduler::Task<ClockType> can_bus_receive(
    tvsc::hal::can_bus::CanBusPeripheral& can_peripheral,
    tvsc::message::CanBusMessageQueue<QUEUE_SIZE, NUM_HANDLERS>& queue,
    tvsc::hal::gpio::GpioPeripheral& gpio_peripheral, tvsc::hal::gpio::Pin pin) {
  using namespace std::chrono_literals;
  using namespace tvsc::hal::can_bus;

  {
    // Turn on the LED GPIO.
    tvsc::hal::gpio::Gpio led{gpio_peripheral.access()};

    led.set_pin_mode(pin, tvsc::hal::gpio::PinMode::OUTPUT_PUSH_PULL);

    for (int i = 0; i < 5; ++i) {
      led.write_pin(pin, 1);
      co_yield 25ms;
      led.write_pin(pin, 0);
      co_yield 25ms;
    }
  }

  // Turn on clock for the CAN bus.
  auto can{can_peripheral.access()};
  while (true) {
    while (can.available_message_count(RxFifo::FIFO_ZERO) > 0) {
      // Turn on the LED GPIO.
      tvsc::hal::gpio::Gpio led{gpio_peripheral.access()};

      led.set_pin_mode(pin, tvsc::hal::gpio::PinMode::OUTPUT_PUSH_PULL);

      tvsc::message::CanBusMessage message{};
      if (can.receive(RxFifo::FIFO_ZERO, message) && queue.enqueue(message)) {
        led.write_pin(pin, 1);
        co_yield 100ms;
        led.write_pin(pin, 0);
      } else {
        for (int i = 0; i < 5; ++i) {
          led.write_pin(pin, 1);
          co_yield 25ms;
          led.write_pin(pin, 0);
          co_yield 25ms;
        }
      }
    }
    co_yield 50ms;
  }
}

}  // namespace tvsc::bringup
