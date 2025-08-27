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

__attribute__((section(".status.value"))) tvsc::hal::mcu::McuId mcu_id{};

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
  tvsc::hal::gpio::GpioPeripheral* led_gpio_peripheral_;
  tvsc::hal::gpio::PinNumber led_pin_;
  tvsc::hal::gpio::Gpio led_gpio_{};

 public:
  LedControl(tvsc::hal::gpio::GpioPeripheral& led_gpio_peripheral, tvsc::hal::gpio::PinNumber led_pin)
      : led_gpio_peripheral_(&led_gpio_peripheral), led_pin_(led_pin) {}

  bool process(const tvsc::message::CanBusMessage& msg) override {
    const tvsc::message::Type type{msg.retrieve_type()};
    if (type == tvsc::message::Type::COMMAND) {
      const tvsc::message::Subsystem subsystem{
          static_cast<tvsc::message::Subsystem>(msg.payload()[0])};
      if (subsystem == tvsc::message::Subsystem::LED) {
        if (msg.payload()[1] == 0x01) {
          if (!led_gpio_.is_valid()) {
            led_gpio_ = led_gpio_peripheral_->access();
            led_gpio_.set_pin_mode(led_pin_, tvsc::hal::gpio::PinMode::OUTPUT_PUSH_PULL);
          }
          led_gpio_.write_pin(led_pin_, 1);
        } else if (msg.payload()[1] == 0x00) {
          if (led_gpio_.is_valid()) {
            led_gpio_.write_pin(led_pin_, 0);
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

  auto& system{System::get()};

  {
    auto& gpio_id_power_peripheral{system.board().gpio<System::BoardType::BOARD_ID_POWER_PORT>()};
    auto& gpio_id_sense_peripheral{system.board().gpio<System::BoardType::BOARD_ID_SENSE_PORT>()};
    auto& adc_peripheral{system.board().adc()};

    board_id = read_board_id(system.clock(), gpio_id_power_peripheral,
                             System::BoardType::BOARD_ID_POWER_PIN, gpio_id_sense_peripheral,
                             System::BoardType::BOARD_ID_SENSE_PIN, adc_peripheral);

    system.board().mcu().read_id(mcu_id);
    hashed_mcu_id = system.board().mcu().hashed_id();

    tvsc::message::create_announce_message(announce_msg, hashed_mcu_id, board_id);
  }

  static constexpr size_t QUEUE_SIZE{5};
  static constexpr size_t NUM_PROCESSORS{2};
  tvsc::message::CanBusMessageQueue<QUEUE_SIZE, NUM_PROCESSORS> can_bus_message_queue{};
  CanBusSniffer can_bus_sniffer{};
  (void)can_bus_message_queue.attach_processor(can_bus_sniffer);

  system.scheduler().add_task(flash_target(system.board().programmer(),
                                           system.board().gpio<System::BoardType::DEBUG_LED_PORT>(),
                                           System::BoardType::DEBUG_LED_PIN));

  system.scheduler().add_task(periodic_transmit(system.board().can1(), 10s, announce_msg));

  system.scheduler().add_task(can_bus_receive(
      system.board().can1(), can_bus_message_queue,
      system.board().gpio<System::BoardType::DEBUG_LED_PORT>(), System::BoardType::DEBUG_LED_PIN));

  system.scheduler().add_task(process_messages(can_bus_message_queue));

  if (board_id == static_cast<tvsc::hal::board_identification::BoardId>(
                      tvsc::hal::board_identification::CanonicalBoardIds::COMMS_BOARD_1)) {
    system.scheduler().add_task(
        periodic_transmit(system.board().can1(), 500ms, 100ms,
                          tvsc::message::led_on_command<tvsc::message::CanBusMessage::mtu()>(),
                          tvsc::message::led_off_command<tvsc::message::CanBusMessage::mtu()>()));
  }

  system.scheduler().start();
}
