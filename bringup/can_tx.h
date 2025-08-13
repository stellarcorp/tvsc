#pragma once

#include <chrono>

#include "base/enums.h"
#include "hal/can_bus/can_bus.h"
#include "message/message.h"
#include "scheduler/task.h"

namespace tvsc::bringup {

/**
 * Transmit the same message periodically. This is useful for heartbeat and announce messages.
 */
template <typename ClockType>
tvsc::scheduler::Task<ClockType> periodic_transmit(
    tvsc::hal::can_bus::CanBusPeripheral& can_peripheral, std::chrono::milliseconds period,
    const message::CanBusMessage& msg) {
  while (true) {
    {
      auto can{can_peripheral.access()};
      can.transmit(msg);
    }
    co_yield period;
  }
}

/**
 * Transmit the two messages periodically, ping-ponging between the two. This is useful for LED
 * control.
 */
template <typename ClockType>
tvsc::scheduler::Task<ClockType> periodic_transmit(
    tvsc::hal::can_bus::CanBusPeripheral& can_peripheral, std::chrono::milliseconds period1, std::chrono::milliseconds period2,
    const message::CanBusMessage& msg1, const message::CanBusMessage& msg2) {
  while (true) {
    {
      auto can{can_peripheral.access()};
      can.transmit(msg1);
    }
    co_yield period1;
    {
      auto can{can_peripheral.access()};
      can.transmit(msg2);
    }
    co_yield period2;
  }
}

}  // namespace tvsc::bringup
