#pragma once

#include <cstdint>

#include "comms/tdma/tdma_frame.h"

namespace tvsc::comms::tdma {

bool encode(const Frame& frame, uint64_t current_time_us, uint8_t* buf, size_t& size);
bool decode(Frame& frame, uint64_t& current_time_us, const uint8_t* buf, size_t size);

}  // namespace tvsc::comms::tdma
