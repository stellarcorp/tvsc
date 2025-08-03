#include <chrono>
#include <cstdint>

#include "base/enums.h"
#include "base/initializer.h"
#include "bringup/blink.h"
#include "bringup/can_rx.h"
#include "bringup/can_tx.h"
#include "bringup/flash_target.h"
#include "bringup/process_messages.h"
#include "bringup/read_board_id.h"
#include "hal/board/board.h"
#include "hal/board_identification/board_ids.h"
#include "hal/mcu/mcu.h"
#include "message/announce.h"
#include "message/handler.h"
#include "message/message.h"
#include "message/queue.h"
#include "message/ring_buffer.h"
#include "scheduler/scheduler.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

using namespace tvsc::bringup;
using namespace tvsc::scheduler;
using namespace std::chrono_literals;
using namespace tvsc::hal::board_identification;

extern "C" {
alignas(uint32_t)  //
    __attribute__((section(".status.value"))) tvsc::hal::board_identification::BoardId board_id{};

__attribute__((section(".status.value"))) tvsc::hal::mcu::McuId mcu_id{};

__attribute__((section(".status.value"))) uint8_t hashed_mcu_id{};
__attribute__((section(".status.value"))) tvsc::message::CanBusMessage announce_msg{};

__attribute__((section(".status.value")))
tvsc::message::RingBuffer<tvsc::message::CanBusMessage, 16, /*PRIORITIZE_EXISTING_ELEMENTS*/ false>
    latest_can_bus_messages{};
}

class CanBusSniffer final
    : public tvsc::message::Handler<tvsc::message::CanBusMessage::PAYLOAD_SIZE> {
 public:
  bool handle(const tvsc::message::CanBusMessage& msg) override {
    (void)latest_can_bus_messages.push(msg);
    return false;
  }
};

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  auto& board{BoardType::board()};
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

  static constexpr size_t NUM_TASKS{4};
  Scheduler<ClockType, NUM_TASKS> scheduler{board.rcc()};

  static constexpr size_t QUEUE_SIZE{5};
  static constexpr size_t NUM_HANDLERS{2};
  tvsc::message::CanBusMessageQueue<QUEUE_SIZE, NUM_HANDLERS> can_bus_message_queue{};
  CanBusSniffer can_bus_sniffer{};
  (void)can_bus_message_queue.attach_handler(can_bus_sniffer);

  scheduler.add_task(flash_target<ClockType>(
      board.programmer(), board.gpio<BoardType::DEBUG_LED_PORT>(), BoardType::DEBUG_LED_PIN));

  scheduler.add_task(periodic_transmit<ClockType>(board.can1(), 1s, announce_msg));

  scheduler.add_task(can_bus_receive<ClockType>(board.can1(), can_bus_message_queue,
                                                board.gpio<BoardType::DEBUG_LED_PORT>(),
                                                BoardType::DEBUG_LED_PIN));

  scheduler.add_task(process_messages<ClockType>(can_bus_message_queue));

  scheduler.start();
}
