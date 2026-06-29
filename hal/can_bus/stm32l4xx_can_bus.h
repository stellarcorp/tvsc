#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>

#include "hal/can_bus/can_bus.h"
#include "hal/gpio/gpio.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::can_bus {

class CanBusStm32l4xx final : public CanBusPeripheral {
 private:
  CAN_HandleTypeDef can_bus_{};
  gpio::PinPeripheral tx_pin_peripheral_;
  gpio::PinPeripheral rx_pin_peripheral_;
  gpio::PinPeripheral shutdown_pin_peripheral_;
  gpio::PinPeripheral silent_pin_peripheral_;
  gpio::Pin tx_pin_{};
  gpio::Pin rx_pin_{};
  gpio::Pin shutdown_pin_{};
  gpio::Pin silent_pin_{};

  void enable() override;
  void disable() override;

  uint32_t available_message_count(RxFifo fifo) override;

  bool receive(RxFifo fifo, uint32_t& identifier, std::array<uint8_t, 8>& data) override;

  bool transmit(uint32_t identifier, const std::array<uint8_t, 8>& data) override;

  uint32_t error_code() const override;

 public:
  CanBusStm32l4xx(CAN_TypeDef* can_bus_instance, gpio::PinPeripheral tx_pin,
                  gpio::PinPeripheral rx_pin, gpio::PinPeripheral shutdown_pin,
                  gpio::PinPeripheral silent_pin)
      : tx_pin_peripheral_(std::move(tx_pin)),
        rx_pin_peripheral_(std::move(rx_pin)),
        shutdown_pin_peripheral_(std::move(shutdown_pin)),
        silent_pin_peripheral_(std::move(silent_pin)) {
    can_bus_.Instance = can_bus_instance;
  }

  void handle_interrupt() override;
};

}  // namespace tvsc::hal::can_bus
