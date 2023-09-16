#pragma once

#include <cstdint>
#include <vector>

namespace tvsc::comms::tdma {

/**
 * Description of a single time slot describing allowed actions in a TDMA cell.
 */
struct TimeSlot final {
  enum class Role : uint8_t {
    // No transmission should occur. This allows nodes to measure background noise levels and
    // perform other processing that might prevent them from receiving (or transmitting) signals.
    BLACKOUT,
    // A particular node may transmit. All other nodes should be listening.
    NODE_TX,
    // An association time slot allows for unassociated nodes to announce themselves. Those nodes
    // will later be assigned NODE_TX time slot during which they can transmit.
    ASSOCIATION,
    // The time skew allowance time slots allow for a small amount of error in the time measured by
    // the clocks in the cell. During this period, all nodes should be receiving, but no node should
    // transmit. The expectation here is that a node may begin or finish transmitting during this
    // period if its clocks are slightly skewed compared to other nodes in the cell. This provides a
    // form of immunity to clock skew and related errors.
    TIME_SKEW_ALLOWANCE,
  };

  uint32_t start_us{};
  uint32_t duration_us{};
  Role role{Role::BLACKOUT};

  /**
   * Id of the node that can transmit during this slot, if any.
   */
  uint64_t slot_owner_id{};
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
   * the base station, the clock of reference for the cell. The TdmaSchedule class manages
   * approximating the current time at that remote clock.
   */
  uint64_t frame_start_time_us{};

  /**
   * Size of a frame in microseconds.
   */
  uint32_t frame_size_us{};

  std::vector<TimeSlot> time_slots{};

  /**
   * The role of any time that is not explicitly accounted for in a frame. This gives the default
   * behavior of the local radio when there aren't explicit instructions on how the time slot should
   * be managed. This default should probably be set at compile time, but all frame announcements
   * from the base station should likely include explicit entries for all of the time slots.
   */
  TimeSlot::Role default_role{TimeSlot::Role::TIME_SKEW_ALLOWANCE};

  uint64_t base_station_id;
};

/**
 * Frame construction and configuration functions.
 */

class FrameBuilder final {
 private:
  Frame frame_{};

  void consolidate_frame_size();

 public:
  FrameBuilder(uint64_t frame_start_time_us);

  void set_base_station_id(uint64_t);
  void add_time_skew_slot(uint32_t duration_us);
  void add_node_tx_slot(uint32_t duration_us, uint64_t owner_id);
  void add_association_slot(uint32_t duration_us);
  void add_blackout_slot(uint32_t duration_us);

  Frame build();
};

}  // namespace tvsc::comms::tdma
