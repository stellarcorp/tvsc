/**
 * This file contains various functions that provide compatibility with the core Arduino libraries,
 * but use different names and possibly different semantics.
 */
// TODO(james): Move this file to a different package.

#pragma once

namespace tvsc::bus::gpio {

void delay_ms(uint32_t milliseconds);
void delay_us(uint32_t microseconds);

}  // namespace tvsc::bus::gpio
