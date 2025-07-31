#pragma once

#include <cstddef>
#include <cstdint>

#include "message/message.h"

namespace tvsc::message {

template <size_t PAYLOAD_SIZE>
void create_announce_message(Message<PAYLOAD_SIZE>& result, uint8_t mcu_id,
                             tvsc::hal::board_identification::BoardId board_id) {
  result.set_type(Type::ANNOUNCE);
  result.clear_payload();
  result.append_payload(sizeof(mcu_id), reinterpret_cast<const uint8_t*>(&mcu_id));
  result.append_payload(sizeof(board_id), reinterpret_cast<const uint8_t*>(&board_id));
}

}  // namespace tvsc::message
