#pragma once

#include <array>
#include <cstdint>

#include "hash/hash_combine.h"
#include "hash/integer_hash.h"

namespace tvsc::hal::mcu {

using McuId = std::array<uint32_t, 3>;

class Mcu {
 public:
  virtual ~Mcu() = default;

  virtual void read_id(McuId& id) = 0;

  uint8_t hashed_id() {
    McuId id;
    read_id(id);
    const auto hashed_id{hash::hash_combine(0, hash::integer_hash(id[0]), hash::integer_hash(id[1]),
                                            hash::integer_hash(id[2]))};

    return static_cast<uint8_t>(hashed_id);
  }
};

}  // namespace tvsc::hal::mcu
