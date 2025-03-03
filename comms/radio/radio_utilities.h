/**
 * Collection of utility functions to assist in sending and receiving.
 */
#pragma once

#include <cstdint>
#include <string>

#include "comms/radio/fragment.h"
#include "comms/radio/half_duplex_radio.h"
#include "comms/radio/yield.h"
#include "hal/output/output.h"
#include "time/time.h"

namespace tvsc::comms::radio {

template <size_t MTU, uint16_t TIMEOUT_MS = 150>
bool recv(HalfDuplexRadio<MTU>& transceiver, Fragment<MTU>& fragment) {
  bool result = block_until_fragment_available(transceiver, TIMEOUT_MS);
  if (result) {
    transceiver.read_received_fragment(fragment);
  } else {
    tvsc::hal::output::println(
        "transceiver_utilities.h recv() -- Receive timed out. No fragments available.");
  }
  return result;
}

template <size_t MTU, uint16_t TIMEOUT_MS = 150>
bool send(HalfDuplexRadio<MTU>& transceiver, const Fragment<MTU>& msg) {
  bool result;

  result = block_until_channel_activity_clear(transceiver, TIMEOUT_MS);
  if (!result) {
    tvsc::hal::output::println("transceiver_utilities.h send() -- Failed due to channel activity.");
    return false;
  }

  result = block_until_transmission_complete(transceiver, TIMEOUT_MS);
  if (!result) {
    tvsc::hal::output::println(
        "transceiver_utilities.h send() -- Failed due to ongoing transmission.");
    return false;
  }

  result = transceiver.transmit_fragment(msg);
  if (result) {
    result = block_until_transmission_complete(transceiver, TIMEOUT_MS);
    if (!result) {
      tvsc::hal::output::println(
          "transceiver_utilities.h send() -- Failed due to block_until_transmission_complete() "
          "timeout.");
    }
  } else {
    tvsc::hal::output::println("transceiver_utilities.h send() -- Failed in transmit_fragment().");
  }

  return result;
}

template <size_t MTU>
bool send(HalfDuplexRadio<MTU>& transceiver, const Fragment<MTU>& msg, uint16_t timeout_ms) {
  bool result;

  result = block_until_channel_activity_clear(transceiver, timeout_ms);
  if (!result) {
    tvsc::hal::output::println("transceiver_utilities.h send() -- Failed due to channel activity.");
    return false;
  }

  result = block_until_transmission_complete(transceiver, timeout_ms);
  if (!result) {
    tvsc::hal::output::println(
        "transceiver_utilities.h send() -- Failed due to ongoing transmission.");
    return false;
  }

  result = transceiver.transmit_fragment(msg);
  if (result) {
    result = block_until_transmission_complete(transceiver, timeout_ms);
    if (!result) {
      tvsc::hal::output::println(
          "transceiver_utilities.h send() -- Failed due to block_until_transmission_complete() "
          "timeout.");
    }
  } else {
    tvsc::hal::output::println("transceiver_utilities.h send() -- Failed in transmit_fragment().");
  }

  return result;
}

template <size_t MTU, uint16_t POLL_DELAY_MS = 1>
bool block_until_fragment_available(HalfDuplexRadio<MTU>& transceiver, uint16_t timeout_ms) {
  const uint64_t start_time{tvsc::time::time_millis()};
  while (!transceiver.has_fragment_available()) {
    if (tvsc::time::time_millis() - start_time > timeout_ms) {
      return false;
    }
    if constexpr (POLL_DELAY_MS > 0) {
      tvsc::time::delay_ms(POLL_DELAY_MS);
    } else {
      YIELD;
    }
  }
  return true;
}

template <size_t MTU, uint16_t POLL_DELAY_MS = 1>
bool block_until_channel_activity_clear(HalfDuplexRadio<MTU>& transceiver, uint16_t timeout_ms) {
  const uint64_t start_time{tvsc::time::time_millis()};
  while (transceiver.channel_activity_detected()) {
    if (tvsc::time::time_millis() - start_time > timeout_ms) {
      return false;
    }
    if constexpr (POLL_DELAY_MS > 0) {
      tvsc::time::delay_ms(POLL_DELAY_MS);
    } else {
      YIELD;
    }
  }
  return true;
}

template <size_t MTU, uint16_t POLL_DELAY_MS = 1>
bool block_until_transmission_complete(HalfDuplexRadio<MTU>& transceiver, uint16_t timeout_ms) {
  const uint64_t start_time{tvsc::time::time_millis()};
  while (transceiver.is_transmitting_fragment()) {
    if (tvsc::time::time_millis() - start_time > timeout_ms) {
      return false;
    }
    if constexpr (POLL_DELAY_MS > 0) {
      tvsc::time::delay_ms(POLL_DELAY_MS);
    } else {
      YIELD;
    }
  }
  return true;
}

}  // namespace tvsc::comms::radio
