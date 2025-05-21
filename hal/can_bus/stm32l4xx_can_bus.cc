#include "hal/can_bus/stm32l4xx_can_bus.h"

#include "base/enums.h"
#include "hal/error.h"
#include "hal/gpio/gpio.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::can_bus {

void CanBusStm32l4xx::enable() {
  using namespace tvsc::hal::gpio;

  gpio_ = gpio_peripheral_->access();
  gpio_.set_pin_mode(tx_pin_, PinMode::ALTERNATE_FUNCTION_PUSH_PULL, PinSpeed::MEDIUM, GPIO_AF9_CAN1);
  gpio_.set_pin_mode(rx_pin_, PinMode::ALTERNATE_FUNCTION_PUSH_PULL, PinSpeed::MEDIUM, GPIO_AF9_CAN1);

  gpio_.set_pin_mode(shutdown_pin_, PinMode::OUTPUT_PUSH_PULL_WITH_PULL_DOWN);
  gpio_.set_pin_mode(silent_pin_, PinMode::OUTPUT_PUSH_PULL_WITH_PULL_DOWN);

  __HAL_RCC_CAN1_CLK_ENABLE();
  __HAL_RCC_CAN1_CLK_SLEEP_ENABLE();  // Keep CAN1 clock active in Stop mode

  gpio_.write_pin(shutdown_pin_, 0);
  gpio_.write_pin(silent_pin_, 0);

  /*
   * ------------------------------
   * CAN bit time = Sync_Seg + Tseg1 + Tseg2
   *   - Sync_Seg is always 1 time quantum (tq)
   *   - Tseg1 = TimeSeg1
   *   - Tseg2 = TimeSeg2
   *
   * Bit rate = tq_clock / (1 + TimeSeg1 + TimeSeg2)
   * tq_clock = PCLK1 / Prescaler
   * ------------------------------
   * Example below achieves:
   *   - tq_clock = 16 MHz / 1 = 16 MHz
   *   - Bit time = 1 + 12 + 3 = 16 tq
   *   - Bit rate = 16 MHz / 16 = 1 Mpbs
   */

  can_bus_.Init.Prescaler = 1;           // Divides PCLK1 (e.g. 32 MHz) to get tq clock (e.g. 8 MHz)
  can_bus_.Init.Mode = CAN_MODE_NORMAL;  // Normal operating mode (not loopback or silent)

  can_bus_.Init.SyncJumpWidth = CAN_SJW_1TQ;
  // Sync Jump Width: allowed resync adjustment range, in tqs
  // 1 tq is usually sufficient unless your oscillator is unstable

  can_bus_.Init.TimeSeg1 = CAN_BS1_12TQ;
  // Tseg1: includes Prop_Seg + Phase_Seg1 (12 tq total here)
  // Longer Tseg1 helps tolerate propagation delays on longer buses

  can_bus_.Init.TimeSeg2 = CAN_BS2_3TQ;
  // Tseg2: Phase_Seg2 (3 tq here), must be >= SJW
  // Allows timing corrections at end of bit

  // Optional features (safe defaults shown)
  can_bus_.Init.TimeTriggeredMode = DISABLE;
  can_bus_.Init.AutoBusOff = ENABLE;  // Automatically enter bus-off on error
  can_bus_.Init.AutoWakeUp = ENABLE;  // Wakeup via CAN bus activity (not needed unless using sleep)
  can_bus_.Init.AutoRetransmission = ENABLE;    // Retry transmission on error (recommended)
  can_bus_.Init.ReceiveFifoLocked = DISABLE;    // Allow overwriting old messages if FIFO is full
  can_bus_.Init.TransmitFifoPriority = ENABLE;  // Send lower-ID messages first

  if (HAL_CAN_Init(&can_bus_) != HAL_OK) {
    error();
  }

  CAN_FilterTypeDef filter{};
  filter.FilterActivation = ENABLE;
  filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  filter.FilterBank = 0;         // first of 14 available
  filter.FilterIdHigh = 0x0000;  // ID[10:0] << 5
  filter.FilterIdLow = 0x0000;
  filter.FilterMaskIdHigh = 0x0000;  // Mask 0 => accept all
  filter.FilterMaskIdLow = 0x0000;
  filter.FilterMode = CAN_FILTERMODE_IDMASK;
  filter.FilterScale = CAN_FILTERSCALE_32BIT;
  filter.SlaveStartFilterBank = 14;

  if (HAL_CAN_ConfigFilter(&can_bus_, &filter) != HAL_OK) {
    error();
  }

  if (HAL_CAN_Start(&can_bus_) != HAL_OK) {
    error();
  }

  // TODO(james): Expose the notification configuration in the API.
  uint32_t notification_configuration{};

  // RX interrupts.
  notification_configuration |= CAN_IT_RX_FIFO0_MSG_PENDING;
  notification_configuration |= CAN_IT_RX_FIFO1_MSG_PENDING;

  // TX interrupts.
  // notification_configuration |= CAN_IT_TX_MAILBOX_EMPTY;

  // Error interrupts.
  notification_configuration |= CAN_IT_ERROR_WARNING;
  notification_configuration |= CAN_IT_ERROR_PASSIVE;
  notification_configuration |= CAN_IT_BUSOFF;
  notification_configuration |= CAN_IT_LAST_ERROR_CODE;

  // Sleep and wake interrupts.
  // notification_configuration |= CAN_IT_WAKEUP;
  // notification_configuration |= CAN_IT_SLEEP;

  // if (HAL_CAN_ActivateNotification(&can_bus_, notification_configuration) != HAL_OK) {
  //   error();
  // }
}

void CanBusStm32l4xx::disable() {
  HAL_CAN_Stop(&can_bus_);
  HAL_CAN_DeInit(&can_bus_);
  __HAL_RCC_CAN1_CLK_DISABLE();
  gpio_.invalidate();
}

uint32_t CanBusStm32l4xx::available_message_count(RxFifo fifo) {
  return HAL_CAN_GetRxFifoFillLevel(&can_bus_, CAN_RX_FIFO0 + cast_to_underlying_type(fifo));
}

bool CanBusStm32l4xx::receive(RxFifo fifo, uint32_t& identifier, std::array<uint8_t, 8>& data) {
  HAL_StatusTypeDef status;
  CAN_RxHeaderTypeDef header{};
  status = HAL_CAN_GetRxMessage(&can_bus_, CAN_RX_FIFO0 + cast_to_underlying_type(fifo), &header,
                                data.data());
  // Note that we only support transmitting standard ids.
  require(header.IDE == CAN_ID_STD);
  identifier = header.StdId;

  // TODO(james): Change the std::array<uint8_t, 8> to a real buffer type and uncomment this line to
  // get the actual size of the received data.
  // data.set_size(header.DLC);
  return status == HAL_OK;
}

bool CanBusStm32l4xx::transmit(uint32_t identifier, const std::array<uint8_t, 8>& data) {
  CAN_TxHeaderTypeDef tx_header = {};

  tx_header.StdId = identifier & 0x7ff;    // Standard 11-bit identifier
  tx_header.ExtId = 0;                     // Not used for standard ID
  tx_header.IDE = CAN_ID_STD;              // Standard frame
  tx_header.RTR = CAN_RTR_DATA;            // Data frame (not remote request)
  tx_header.DLC = data.size();             // Data length (max 8 for bxCAN)
  tx_header.TransmitGlobalTime = DISABLE;  // Not using timestamp

  uint32_t tx_mailbox;
  if (HAL_CAN_AddTxMessage(&can_bus_, &tx_header, const_cast<uint8_t*>(data.data()), &tx_mailbox) ==
      HAL_OK) {
    return true;
  } else {
    // All 3 TX mailboxes are likely full (HAL_BUSY).
    // Other errors can occur, but they are most likely due to misconfiguration.
    return false;
  }
}

void CanBusStm32l4xx::handle_interrupt() { HAL_CAN_IRQHandler(&can_bus_); }

uint32_t CanBusStm32l4xx::error_code() const { return can_bus_.ErrorCode; }

}  // namespace tvsc::hal::can_bus
