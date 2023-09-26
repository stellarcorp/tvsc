#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "base/except.h"
#include "comms/tdma/tdma_frame.h"
#include "hal/time/clock.h"
#include "hal/time/remote_clock.h"

namespace tvsc::comms::tdma {

/**
 * Schedule controlling the activity of nodes in a TDMA cell according to the current time of the
 * cell.
 */
class Schedule final {
 private:
  Frame frame_{FrameBuilder::create_default_node_frame()};

  // Clock to approximate the time as measured by the clock on the base station. That clock
  // represents the definitive time for the cell.
  mutable tvsc::hal::time::RemoteClock cell_clock_;
  uint64_t id_{};

  int64_t time_after_frame_start_us() const {
    int64_t difference{};

    difference = cell_clock_.current_time_micros();
    difference -= frame_.frame_start_time_us;

    return difference;
  }

  const TimeSlot& current_time_slot() const {
    const uint32_t offset_us{frame_offset_us()};
    for (const TimeSlot& slot : frame_.time_slots) {
      if (slot.start_us <= offset_us && slot.start_us + slot.duration_us > offset_us) {
        return slot;
      }
    }
  }

 public:
  Schedule(tvsc::hal::time::Clock& local_clock) : cell_clock_(local_clock) {}
  Schedule(tvsc::hal::time::Clock& local_clock, uint64_t id) : cell_clock_(local_clock), id_(id) {}

  /**
   * Get/set the id of the radio we use to receive and transmit. Time slots with this id refer to
   * us.
   */
  uint64_t id() const { return id_; }
  void set_id(uint64_t id) { id_ = id; }

  bool is_base_station() const { return id_ == frame_.base_station_id; }

  bool is_associated(uint64_t id) const;
  bool is_associated() const { return is_associated(id_); }

  uint64_t cell_time_us() const { return cell_clock_.current_time_micros(); }

  void mark_cell_time(uint64_t current_cell_time_us) {
    cell_clock_.mark_remote_time_micros(current_cell_time_us);
  }

  bool can_transmit() const;
  bool should_receive() const;

  uint32_t frame_offset_us() const {
    return time_after_frame_start_us() % frame_.frame_duration_us;
  }

  uint32_t time_slot_offset_us() const {
    const TimeSlot& slot{current_time_slot()};
    return frame_offset_us() - slot.start_us;
  }

  uint32_t frame_duration_us() const { return frame_.frame_duration_us; }

  uint32_t time_slot_duration_us() const {
    const TimeSlot& slot{current_time_slot()};
    return slot.duration_us;
  }

  uint32_t frame_duration_remaining_us() const { return frame_duration_us() - frame_offset_us(); }

  uint32_t time_slot_duration_remaining_us() const {
    return time_slot_duration_us() - time_slot_offset_us();
  }

  TimeSlot::Role time_slot_role() const {
    const TimeSlot& slot{current_time_slot()};
    return slot.role;
  }

  uint64_t time_slot_owner() const {
    const TimeSlot& slot{current_time_slot()};
    return slot.slot_owner_id;
  }

  void set_frame(const Frame& frame) { frame_ = frame; }
  Frame& frame() { return frame_; }
  const Frame& frame() const { return frame_; }
};

}  // namespace tvsc::comms::tdma
