#pragma once

#include <cstdint>
#include <vector>

namespace tvsc::comms::tdma {

/**
 * Description of a single time slot describing allowed actions in a TDMA cell.
 */
struct TimeSlot final {
  enum class Role : uint8_t {
    // The guard interval time slots allow for a small amount of error in the time measured by
    // the clocks in the cell. During this period, all nodes should be receiving, but no node should
    // transmit. The expectation here is that a node may begin or finish transmitting during this
    // period if its clocks are slightly skewed compared to other nodes in the cell. This provides a
    // form of immunity to clock skew and related errors.
    GUARD_INTERVAL,
    // A particular node may transmit. All other nodes should be listening.
    NODE_TX,
    // An association time slot allows for unassociated nodes to announce themselves. Those nodes
    // will later be assigned NODE_TX time slot during which they can transmit.
    ASSOCIATION,
    // Blackout of all transmissions; no transmission should occur. This allows nodes to measure
    // background noise levels and perform other processing that might prevent them from receiving
    // (or transmitting) signals.
    BLACKOUT,
  };

  uint32_t start_us{};
  uint32_t duration_us{};
  Role role{Role::BLACKOUT};

  /**
   * Id of the node that can transmit during this slot, if any.
   */
  uint64_t slot_owner_id{};

  bool operator==(const TimeSlot& rhs) const {
    return start_us == rhs.start_us &&        //
           duration_us == rhs.duration_us &&  //
           role == rhs.role &&                //
           slot_owner_id == rhs.slot_owner_id;
  }
};

/**
 * All of the time slots for a TDMA cell, as well as information about the length of a frame and
 * the role of unallocated time slots. These time slots are dense, meaning there should be no gaps
 * in time. The start time plus the duration of a slot should equal the start time of the next time
 * slot. Any unallocated time at the end of the frame will be assumed to have the default role.
 */
struct Frame final {
  /**
   * When this frame began or will begin. This time is measured from the perspective of the clock on
   * the base station, the clock of reference for the cell. The Schedule class manages approximating
   * the current time at that remote clock.
   */
  uint64_t frame_start_time_us{};

  /**
   * Duration of the frame in microseconds.
   */
  uint32_t frame_duration_us{};

  /**
   * All of the time slots in the frame to indicate which nodes may transmit and which must be
   * receiving. The slots may have different durations.
   */
  std::vector<TimeSlot> time_slots{};

  /**
   * Id of the base station of the cell, since some rules in the TDMA schedule are different for
   * base stations and nodes.
   */
  uint64_t base_station_id;

  bool operator==(const Frame& rhs) const {
    return frame_start_time_us == rhs.frame_start_time_us &&  //
           frame_duration_us == rhs.frame_duration_us &&      //
           time_slots == rhs.time_slots &&                    //
           base_station_id == rhs.base_station_id;
  }
};

void pack_frame_times(Frame& frame);

/**
 * Frame construction and configuration functions.
 */
class FrameBuilder final {
 private:
  Frame frame_{};

 public:
  FrameBuilder(uint64_t frame_start_time_us = 0);

  void set_base_station_id(uint64_t id);

  void add_guard_interval(uint32_t duration_us);
  void add_node_tx_slot(uint32_t duration_us, uint64_t owner_id);
  void add_association_slot(uint32_t duration_us);
  void add_blackout_slot(uint32_t duration_us);

  Frame build();

  /**
   * Create a default frame for a node. This frame will put the node in a mode to listen for frame
   * announcements.
   */
  static Frame create_default_node_frame();

  /**
   * Create a default frame for a base station. This frame will establish time for the base station
   * to broadcast frame announcements and listen for association requests.
   */
  static Frame create_default_base_station_frame(uint64_t base_station_id);
};

}  // namespace tvsc::comms::tdma
