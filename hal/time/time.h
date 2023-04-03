/**
 * This file contains various functions that provide compatibility with the core Arduino libraries,
 * but use different names and possibly different semantics.
 */
#pragma once

namespace tvsc::hal::time {

void delay_ms(uint32_t milliseconds);
void delay_us(uint32_t microseconds);

}  // namespace tvsc::hal::time
