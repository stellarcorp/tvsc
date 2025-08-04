#pragma once

#include <cstddef>
#include <cstdint>

#include "message/message.h"

namespace tvsc::message {

template <size_t PAYLOAD_SIZE>
void create_led_command(Message<PAYLOAD_SIZE>& result, bool turn_on) {
  static constexpr uint8_t ON_VALUE{0x01};
  static constexpr uint8_t OFF_VALUE{0x00};
  static constexpr Subsystem LED_SUBSYSTEM{Subsystem::LED};
  result.set_type(Type::COMMAND);
  result.clear_payload();
  result.append_payload(sizeof(std::underlying_type_t<Subsystem>),
                        reinterpret_cast<const uint8_t*>(&LED_SUBSYSTEM));
  if (turn_on) {
    result.append_payload(1, &ON_VALUE);
  } else {
    result.append_payload(1, &OFF_VALUE);
  }
}

template <size_t PAYLOAD_SIZE>
Message<PAYLOAD_SIZE> led_on_command() {
  Message<PAYLOAD_SIZE> result{};
  create_led_command(result, true);
  return result;
}

template <size_t PAYLOAD_SIZE>
Message<PAYLOAD_SIZE> led_off_command() {
  Message<PAYLOAD_SIZE> result{};
  create_led_command(result, false);
  return result;
}

}  // namespace tvsc::message
