#include <chrono>
#include <cstdint>

#include "base/initializer.h"
#include "bringup/blink.h"
#include "bringup/can_tx.h"
#include "bringup/flash_target.h"
#include "bringup/read_board_id.h"
#include "hal/board/board.h"
#include "hal/board_identification/board_ids.h"
#include "hal/mcu/mcu.h"
#include "message/announce.h"
#include "message/message.h"
#include "scheduler/scheduler.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

using namespace tvsc::bringup;
using namespace tvsc::scheduler;
using namespace std::chrono_literals;

extern "C" {
alignas(uint32_t)  //
    __attribute__((section(".status.value"))) tvsc::hal::board_identification::BoardId board_id{};

__attribute__((section(".status.value"))) tvsc::hal::mcu::McuId mcu_id{};

__attribute__((section(".status.value"))) uint8_t hashed_mcu_id{};
__attribute__((section(".status.value"))) tvsc::message::CanBusMessage announce_msg{};
}

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  auto& board{BoardType::board()};
  auto& clock{ClockType::clock()};

  {
    auto& gpio_id_power_peripheral{board.gpio<BoardType::BOARD_ID_POWER_PORT>()};
    auto& gpio_id_sense_peripheral{board.gpio<BoardType::BOARD_ID_SENSE_PORT>()};
    auto& adc_peripheral{board.adc()};

    board_id =
        read_board_id(gpio_id_power_peripheral, BoardType::BOARD_ID_POWER_PIN,
                      gpio_id_sense_peripheral, BoardType::BOARD_ID_SENSE_PIN, adc_peripheral);

    board.mcu().read_id(mcu_id);
    hashed_mcu_id = board.mcu().hashed_id();

    tvsc::message::create_announce_message(announce_msg, hashed_mcu_id, board_id);
  }

  static constexpr size_t QUEUE_SIZE{4};
  Scheduler<ClockType, QUEUE_SIZE> scheduler{board.rcc()};

  scheduler.add_task(flash_target<ClockType>(
      board.programmer(), board.gpio<BoardType::DEBUG_LED_PORT>(), BoardType::DEBUG_LED_PIN));

  scheduler.add_task(periodic_transmit<ClockType>(board.can1(), 10s, announce_msg));

  scheduler.add_task(
      blink(clock, board.gpio<BoardType::DEBUG_LED_PORT>(), BoardType::DEBUG_LED_PIN));

  scheduler.start();
}
