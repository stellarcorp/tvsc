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
      can.transmit(cast_to_underlying_type(msg.type()), msg.payload());
    }
    co_yield period;
  }
}

}  // namespace tvsc::bringup
