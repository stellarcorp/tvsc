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
#include "hal/mcu_identification/mcu_identification.h"
#include "message/announce.h"
#include "message/leds.h"
#include "message/message.h"
#include "message/processor.h"
#include "message/queue.h"
#include "message/ring_buffer.h"
#include "system/scheduler.h"
#include "system/system.h"
#include "time/embedded_clock.h"

using namespace std::chrono_literals;
using namespace tvsc::bringup;
using namespace tvsc::hal::board_identification;
using namespace tvsc::system;

extern "C" {
alignas(uint32_t)  //
    __attribute__((section(".status.value"))) tvsc::hal::board_identification::BoardId board_id{};

__attribute__((section(".status.value"))) tvsc::hal::mcu_identification::McuId mcu_id{};

__attribute__((section(".status.value"))) uint8_t hashed_mcu_id{};
__attribute__((section(".status.value"))) tvsc::message::CanBusMessage announce_msg{};

__attribute__((section(".status.value")))
tvsc::message::RingBuffer<tvsc::message::CanBusMessage, 16, /*PRIORITIZE_EXISTING_ELEMENTS*/ false>
    latest_can_bus_messages{};
}

class CanBusSniffer final : public tvsc::message::Processor<tvsc::message::CanBusMessage> {
 public:
  bool process(const tvsc::message::CanBusMessage& msg) override {
    (void)latest_can_bus_messages.push(msg);
    return false;
  }
};

class LedControl final : public tvsc::message::Processor<tvsc::message::CanBusMessage> {
 private:
  tvsc::hal::gpio::PinPeripheral led_peripheral_;
  tvsc::hal::gpio::Pin led_{};

 public:
  LedControl(tvsc::hal::gpio::PinPeripheral led_peripheral)
      : led_peripheral_(std::move(led_peripheral)) {}

  bool process(const tvsc::message::CanBusMessage& msg) override {
    const tvsc::message::Type type{msg.retrieve_type()};
    if (type == tvsc::message::Type::COMMAND) {
      const tvsc::message::Subsystem subsystem{
          static_cast<tvsc::message::Subsystem>(msg.payload()[0])};
      if (subsystem == tvsc::message::Subsystem::LED) {
        if (msg.payload()[1] == 0x01) {
          if (!led_.is_valid()) {
            led_ = led_peripheral_.access();
            led_.set_pin_mode(tvsc::hal::gpio::PinMode::OUTPUT_PUSH_PULL);
          }
          led_.write_pin(/* ON */ 1);
        } else if (msg.payload()[1] == 0x00) {
          if (led_.is_valid()) {
            led_.write_pin(/* OFF */ 0);
          }
        }
        return true;
      }
    }
    return false;
  }
};

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);
  using Pinout = System::PinoutType;
  auto& system{System::get()};

  {
    auto id_power_peripheral{system.mcu().as_peripheral(Pinout::BOARD_ID_POWER_PIN)};
    auto id_sense_peripheral{system.mcu().as_peripheral(Pinout::BOARD_ID_SENSE_PIN)};
    auto& adc_peripheral{system.mcu().adc()};

    board_id = read_board_id(system.clock(), std::move(id_power_peripheral),
                             std::move(id_sense_peripheral), adc_peripheral);

    system.mcu().mcu_identification().read_id(mcu_id);
    hashed_mcu_id = system.mcu().mcu_identification().hashed_id();

    tvsc::message::create_announce_message(announce_msg, hashed_mcu_id, board_id);
  }

  static constexpr size_t QUEUE_SIZE{5};
  static constexpr size_t NUM_PROCESSORS{2};
  tvsc::message::CanBusMessageQueue<QUEUE_SIZE, NUM_PROCESSORS> can_bus_message_queue{};
  CanBusSniffer can_bus_sniffer{};
  (void)can_bus_message_queue.attach_processor(can_bus_sniffer);

  system.scheduler().add_task(
      flash_target(system.board().programmer(), system.board().debug_led()));

  system.scheduler().add_task(periodic_transmit(system.mcu().can<0>(), 10s, announce_msg));

  system.scheduler().add_task(
      can_bus_receive(system.mcu().can<0>(), can_bus_message_queue, system.board().debug_led()));

  system.scheduler().add_task(process_messages(can_bus_message_queue));

  if (board_id == static_cast<tvsc::hal::board_identification::BoardId>(
                      tvsc::hal::board_identification::CanonicalBoardIds::COMMS_BOARD_1)) {
    system.scheduler().add_task(
        periodic_transmit(system.mcu().can<0>(), 500ms, 100ms,
                          tvsc::message::led_on_command<tvsc::message::CanBusMessage::mtu()>(),
                          tvsc::message::led_off_command<tvsc::message::CanBusMessage::mtu()>()));
  }

  system.scheduler().start();
}
