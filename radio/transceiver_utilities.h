/**
 * Collection of utility functions to assist in sending and receiving.
 */
#pragma once

#include <cstdint>
#include <string>

#include "hal/output/output.h"
#include "hal/time/time.h"
#include "radio/fragment.h"
#include "radio/transceiver.h"
#include "radio/yield.h"

namespace tvsc::radio {

template <size_t MTU>
bool recv(HalfDuplexTransceiver<MTU>& transceiver, Fragment<MTU>& fragment) {
  return transceiver.receive_fragment(fragment, 100);
}

template <size_t MTU, uint16_t TIMEOUT_MS = 50>
bool send(HalfDuplexTransceiver<MTU>& transceiver, const Fragment<MTU>& msg) {
  bool result;

  result = block_until_channel_activity_clear(transceiver, TIMEOUT_MS);
  if (!result) {
    tvsc::hal::output::println("utilities.h send() -- Failed due to channel activity.");
    return false;
  }

  result = block_until_transmission_complete(transceiver, TIMEOUT_MS);
  if (!result) {
    tvsc::hal::output::println("utilities.h send() -- Failed due to ongoing transmission.");
    return false;
  }

  result = transceiver.transmit_fragment(msg, TIMEOUT_MS);
  if (result) {
    // Note that we ignore the return value here. The PACKETSENT interrupt is not triggered. That
    // means that we have to wait and assume the packet got transmitted rather than actually
    // knowing.
    // TODO(james): Fix the interrupts in RF69HCW so that we can use this return value.
    result = block_until_transmission_complete(transceiver, TIMEOUT_MS);
    if (!result) {
      tvsc::hal::output::println(
          "utilities.h send() -- Failed due to block_until_transmission_complete() timeout.");
    }
  } else {
    tvsc::hal::output::println("utilities.h send() -- Failed in transmit_fragment().");
  }

  return result;
}

template <size_t MTU, uint16_t POLL_DELAY_MS = 1>
bool block_until_fragment_available(HalfDuplexTransceiver<MTU>& transceiver, uint16_t timeout_ms) {
  const uint64_t start_time{tvsc::hal::time::time_millis()};
  while (!transceiver.has_fragment_available()) {
    if (tvsc::hal::time::time_millis() - start_time > timeout_ms) {
      return false;
    }
    if constexpr (POLL_DELAY_MS > 0) {
      tvsc::hal::time::delay_ms(POLL_DELAY_MS);
    } else {
      YIELD;
    }
  }
  return true;
}

template <size_t MTU, uint16_t POLL_DELAY_MS = 1>
bool block_until_channel_activity_clear(HalfDuplexTransceiver<MTU>& transceiver,
                                        uint16_t timeout_ms) {
  const uint64_t start_time{tvsc::hal::time::time_millis()};
  while (transceiver.channel_activity_detected()) {
    if (tvsc::hal::time::time_millis() - start_time > timeout_ms) {
      return false;
    }
    if constexpr (POLL_DELAY_MS > 0) {
      tvsc::hal::time::delay_ms(POLL_DELAY_MS);
    } else {
      YIELD;
    }
  }
  return true;
}

template <size_t MTU, uint16_t POLL_DELAY_MS = 1>
bool block_until_transmission_complete(HalfDuplexTransceiver<MTU>& transceiver,
                                       uint16_t timeout_ms) {
  const uint64_t start_time{tvsc::hal::time::time_millis()};
  while (transceiver.is_transmitting_fragment()) {
    if (tvsc::hal::time::time_millis() - start_time > timeout_ms) {
      return false;
    }
    if constexpr (POLL_DELAY_MS > 0) {
      tvsc::hal::time::delay_ms(POLL_DELAY_MS);
    } else {
      YIELD;
    }
  }
  return true;
}

}  // namespace tvsc::radio
