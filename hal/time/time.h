/**
 * This file contains various functions that provide compatibility with the core Arduino libraries,
 * but use different names and possibly different semantics.
 */
#pragma once

namespace tvsc::hal::time {

void delay_ms(uint32_t milliseconds);
void delay_us(uint32_t microseconds);

/**
 * Current time in milliseconds. Note that this may be the number of milliseconds since the epoch,
 * or it may be the number of milliseconds since the last boot of the system. It is not guaranteed
 * to be monotonically increasing, since most Arduino platforms overflow this time after about 50
 * days, though we use std::chrono::steady_clock when it is available.
 */
uint64_t time_millis();

}  // namespace tvsc::hal::time
