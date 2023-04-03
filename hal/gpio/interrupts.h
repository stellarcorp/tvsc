#pragma once

#include <cstdint>
#include <functional>

namespace tvsc::bus::gpio {

void attach_interrupt(uint8_t pin, std::function<void()> fn);

}  // namespace tvsc::bus::gpio
