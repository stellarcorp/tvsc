#pragma once

#include <cstdint>
#include <string>

#include "hal/output/output.h"

namespace tvsc::comms::radio {

/**
 * Identification of a transceiver.
 */
struct TransceiverIdentification final {
  /**
   * The expanded id will be a randomly generated unsigned 64-bit integer value. This value is good
   * for uniqueness, but bad for transmission size. As an alternative, we will use a small integer
   * for the id for most communications. The full id exists so that we can address conflicts. In the
   * event of a conflict, we will need a method for regenerating and re-advertising the short id.
   */
  uint64_t expanded_id{};

  /**
   * The id for the transceiver. This is a shortened form of the expanded_id above. The goal of this
   * id is to save space in any transmissions, especially since these transmissions may be going
   * over radios with high latency and limited bandwidth.
   */
  uint8_t id{};

  /**
   * Create a new TransceiverIdentification and initialize it from either an existing saved
   * identification, or, if no such identification exists, populate it with unique random values and
   * save that identification.
   */
  static TransceiverIdentification initialize();

  /**
   * Direct load and save functions. You will likely want to call the initialize() function instead
   * of calling these directly.
   */
  static bool load_transceiver_identification(TransceiverIdentification& id);
  static void save_transceiver_identification(const TransceiverIdentification& id);
};

inline std::string to_string(const TransceiverIdentification& id) {
  using std::to_string;
  std::string result{};

  result.append("{")
      .append(to_string(id.expanded_id))
      .append(", ")
      .append(to_string(id.id))
      .append("}");

  return result;
}

inline void print_id(const TransceiverIdentification& id) {
  tvsc::hal::output::println(to_string(id));
}

}  // namespace tvsc::comms::radio
