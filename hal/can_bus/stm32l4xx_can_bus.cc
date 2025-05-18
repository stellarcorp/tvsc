#include "hal/can_bus/stm32l4xx_can_bus.h"

#include "hal/error.h"
#include "hal/gpio/gpio.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::can_bus {

void CanBusStm32l4xx::enable() {
  using namespace tvsc::hal::gpio;

  gpio_ = gpio_peripheral_->access();
  gpio_.set_pin_mode(tx_pin_, PinMode::ALTERNATE_FUNCTION_OPEN_DRAIN_WITH_PULL_UP,
                     PinSpeed::HIGH, GPIO_AF9_CAN1);
  gpio_.set_pin_mode(rx_pin_, PinMode::ALTERNATE_FUNCTION_OPEN_DRAIN_WITH_PULL_UP,
                     PinSpeed::HIGH, GPIO_AF9_CAN1);



}

void CanBusStm32l4xx::disable() {
  HAL_I2C_DeInit(&can_bus_);

  gpio_.invalidate();
}

bool CanBusStm32l4xx::data_available(RxFifo fifo) {}

void CanBusStm32l4xx::transmit(TxMailbox mailbox, uint32_t identifier,
                               const std::array<uint8_t, 8>& data) {}

void CanBusStm32l4xx::receive(RxFifo fifo, uint32_t& identifier, std::array<uint8_t, 8>& data) {}

void CanBusStm32l4xx::handle_interrupt() {}

}  // namespace tvsc::hal::can_bus
