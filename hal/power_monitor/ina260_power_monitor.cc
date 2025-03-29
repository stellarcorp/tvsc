#include "hal/power_monitor/ina260_power_monitor.h"

#include "hal/error.h"

namespace tvsc::hal::power_monitor {

void Ina260PowerMonitor::enable() {
  i2c_ = i2c_peripheral_->access();
  require(i2c_.is_device_ready(addr_));
}

void Ina260PowerMonitor::disable() { i2c_.invalidate(); }

bool Ina260PowerMonitor::read_id(uint16_t* result) {
  uint8_t bytes[2]{};
  bool success{i2c_.read(addr_, ID_REGISTER, reinterpret_cast<uint8_t*>(bytes), sizeof(bytes))};
  if (success) {
    *result = (static_cast<uint16_t>(bytes[0]) << 8) + bytes[1];
  }
  return success;
}

}  // namespace tvsc::hal::power_monitor
