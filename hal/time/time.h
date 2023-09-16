/**
 * This file contains various functions that provide compatibility with the core Arduino libraries,
 * but use different names and possibly different semantics.
 */
#pragma once

namespace tvsc::hal::time {

/**
 * Sleep this thread for the given duration.
 */
void delay_ms(uint32_t milliseconds);
void delay_us(uint32_t microseconds);

/**
 * Current time in milliseconds. Note that this may be the number of milliseconds since the epoch,
 * or it may be the number of milliseconds since the last boot of the system. It is guaranteed
 * to be monotonically increasing up to an overflow condition. Most Arduino platforms overflow this
 * time after about 50 days. On systems where it is available, we use std::chrono::steady_clock.
 */
uint64_t time_millis();

/**
 * Current time in milliseconds. Note that this may be the number of milliseconds since the epoch,
 * or it may be the number of milliseconds since the last boot of the system. It is guaranteed
 * to be monotonically increasing up to an overflow condition. Most Arduino platforms overflow this
 * time after about 50 days. On systems where it is available, we use std::chrono::steady_clock.
 */
uint64_t time_micros();

}  // namespace tvsc::hal::time
