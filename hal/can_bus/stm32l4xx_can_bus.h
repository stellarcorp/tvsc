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
  gpio::GpioPeripheral* gpio_peripheral_;
  gpio::Gpio gpio_{};
  gpio::Pin tx_pin_;
  gpio::Pin rx_pin_;
  gpio::Pin shutdown_pin_;
  gpio::Pin silent_pin_;

  void enable() override;
  void disable() override;

  uint32_t available_message_count(RxFifo fifo) override;

  bool receive(RxFifo fifo, uint32_t& identifier, std::array<uint8_t, 8>& data) override;

  bool transmit(uint32_t identifier, const std::array<uint8_t, 8>& data) override;

  uint32_t error_code() const override;

 public:
  CanBusStm32l4xx(CAN_TypeDef* can_bus_instance, gpio::GpioPeripheral& gpio_peripheral,
                  gpio::Pin tx_pin, gpio::Pin rx_pin, gpio::Pin shutdown_pin, gpio::Pin silent_pin)
      : gpio_peripheral_(&gpio_peripheral),
        tx_pin_(tx_pin),
        rx_pin_(rx_pin),
        shutdown_pin_(shutdown_pin),
        silent_pin_(silent_pin) {
    can_bus_.Instance = can_bus_instance;
  }

  void handle_interrupt() override;
};

}  // namespace tvsc::hal::can_bus
