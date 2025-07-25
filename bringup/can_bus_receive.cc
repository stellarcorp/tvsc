#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

#include "base/initializer.h"
#include "hal/board/board.h"
#include "hal/can_bus/can_bus.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"
#include "time/embedded_clock.h"

extern "C" {
__attribute__((section(".status.value"))) volatile uint32_t rx_count{};
__attribute__((section(".status.value"))) volatile uint32_t rx_error_count{};
__attribute__((section(".status.value"))) volatile uint32_t error_code{};
__attribute__((section(".status.value"))) uint32_t identifier{};
__attribute__((section(".status.value"))) std::array<uint8_t, 8> message{};

__attribute__((section(".status.value"))) volatile bool can_tx_complete_flag{false};
__attribute__((section(".status.value"))) volatile bool can_rx_fifo0_msg_pending_flag{false};
__attribute__((section(".status.value"))) volatile bool can_rx_fifo1_msg_pending_flag{false};

__attribute__((section(".status.value"))) volatile bool can_sleep_flag{false};
__attribute__((section(".status.value"))) volatile bool can_wakeup_flag{false};

__attribute__((section(".status.value"))) volatile bool can_unknown_error_flag{false};

__attribute__((section(".status.value"))) volatile bool can_error_passive_flag{false};
__attribute__((section(".status.value"))) volatile bool can_bus_off_flag{false};
__attribute__((section(".status.value"))) volatile bool can_error_warning_flag{false};
__attribute__((section(".status.value"))) volatile bool can_last_error_code_updated{false};

void clear_debug_flags() {
  can_tx_complete_flag = false;
  can_rx_fifo0_msg_pending_flag = false;
  can_rx_fifo1_msg_pending_flag = false;

  can_unknown_error_flag = false;

  can_sleep_flag = false;
  can_wakeup_flag = false;

  can_error_passive_flag = false;
  can_bus_off_flag = false;
  can_error_warning_flag = false;
  can_last_error_code_updated = false;
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *) { can_tx_complete_flag = true; }

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *) { can_tx_complete_flag = true; }

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *) { can_tx_complete_flag = true; }

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *) {
  can_rx_fifo0_msg_pending_flag = true;
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *) {
  can_rx_fifo1_msg_pending_flag = true;
}

void HAL_CAN_SleepCallback(CAN_HandleTypeDef *) { can_sleep_flag = true; }

void HAL_CAN_WakeUpFromRxMsgCallback(CAN_HandleTypeDef *) { can_wakeup_flag = true; }

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *) {
  can_unknown_error_flag = true;

  // if (hcan->ErrorCode & HAL_CAN_ERROR_EWG) {
  //   can_error_warning_flag = true;
  // }
  // if (hcan->ErrorCode & HAL_CAN_ERROR_EPV) {
  //   can_error_passive_flag = true;
  // }
  // if (hcan->ErrorCode & HAL_CAN_ERROR_BOF) {
  //   can_bus_off_flag = true;
  // }
  // if (hcan->ErrorCode & HAL_CAN_ERROR_LEC) {
  //   can_last_error_code_updated = true;
  // }
}
}

namespace tvsc::bringup {

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

template <typename ClockType>
tvsc::scheduler::Task<ClockType> echo_server(BoardType &board) {
  using namespace std::chrono_literals;
  using namespace tvsc::hal::can_bus;
  using namespace tvsc::hal::gpio;

  auto &debug_led_peripheral{board.gpio<BoardType::DEBUG_LED_PORT>()};
  auto &can1_peripheral{board.can1()};

  // Turn on clocks for the peripherals that we want.
  auto debug_led{debug_led_peripheral.access()};
  auto can1{can1_peripheral.access()};

  debug_led.set_pin_mode(BoardType::DEBUG_LED_PIN, PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);

  while (true) {
    error_code = can1.error_code();
    while (can1.available_message_count(RxFifo::FIFO_ZERO) > 0) {
      if (can1.receive(RxFifo::FIFO_ZERO, identifier, message)) {
        ++rx_count;
        debug_led.write_pin(BoardType::DEBUG_LED_PIN, 1);
        co_yield 2ms;
        debug_led.write_pin(BoardType::DEBUG_LED_PIN, 0);
      } else {
        ++rx_error_count;
      }
    }
    clear_debug_flags();

    co_yield 1ms;
  }
}

}  // namespace tvsc::bringup

using namespace tvsc::bringup;
using namespace tvsc::scheduler;

int main(int argc, char *argv[]) {
  tvsc::initialize(&argc, &argv);

  BoardType &board{BoardType::board()};

  Scheduler<ClockType, 1 /*QUEUE_SIZE*/> scheduler{board.rcc()};
  scheduler.add_task(echo_server<ClockType>(board));
  scheduler.start();
}
