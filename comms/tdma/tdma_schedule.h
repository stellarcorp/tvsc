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
 * Schedule containing all of the time slots for activity in a TDMA cell.
 */
class TdmaSchedule final {
 private:
  Frame frame_{};
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

  const TimeSlot* current_time_slot() const {
    const uint32_t offset_us{frame_offset_us()};
    for (const TimeSlot& slot : frame_.time_slots) {
      if (slot.start_us <= offset_us && slot.start_us + slot.duration_us > offset_us) {
        return &slot;
      }
    }
    return nullptr;
  }

 public:
  TdmaSchedule(tvsc::hal::time::Clock& local_clock) : cell_clock_(local_clock) {}
  TdmaSchedule(tvsc::hal::time::Clock& local_clock, uint64_t id)
      : cell_clock_(local_clock), id_(id) {}

  /**
   * Get/set the id of the radio we use to receive and transmit. Time slots with this id refer to
   * us.
   */
  uint64_t id() const { return id_; }
  void set_id(uint64_t id) { id_ = id; }

  bool is_base_station() const { return id_ == frame_.base_station_id; }

  bool is_associated(uint64_t id) const {
    for (const TimeSlot& slot : frame_.time_slots) {
      if (slot.role == TimeSlot::Role::NODE_TX && slot.slot_owner_id == id) {
        return true;
      }
    }
    return false;
  }

  bool is_associated() const { return is_associated(id_); }

  uint64_t cell_time_us() const { return cell_clock_.current_time_micros(); }

  void mark_cell_time(uint64_t current_cell_time_us) {
    cell_clock_.mark_remote_time_micros(current_cell_time_us);
  }

  bool can_transmit() const {
    const TimeSlot* slot{current_time_slot()};
    if (slot != nullptr) {
      switch (slot->role) {
        case TimeSlot::Role::BLACKOUT:
          return false;
          break;
        case TimeSlot::Role::NODE_TX:
          return slot->slot_owner_id == id_;
          break;
        case TimeSlot::Role::ASSOCIATION:
          return !is_associated() && !is_base_station();
          break;
        case TimeSlot::Role::TIME_SKEW_ALLOWANCE:
          return false;
          break;
      }
    } else {
      switch (frame_.default_role) {
        case TimeSlot::Role::BLACKOUT:
        case TimeSlot::Role::TIME_SKEW_ALLOWANCE:
          return false;

        default:
          except<std::logic_error>("Invalid role to be used as a default role in a TDMA frame");
	  return false;
      }
    }
  }

  bool should_receive() const {
    const TimeSlot* slot{current_time_slot()};
    if (slot != nullptr) {
      switch (slot->role) {
        case TimeSlot::Role::BLACKOUT:
          return false;
          break;
        case TimeSlot::Role::NODE_TX:
          return slot->slot_owner_id != id_;
          break;
        case TimeSlot::Role::ASSOCIATION:
          return is_base_station();
          break;
        case TimeSlot::Role::TIME_SKEW_ALLOWANCE:
          return true;
          break;
      }
    } else {
      switch (frame_.default_role) {
        case TimeSlot::Role::BLACKOUT:
          return false;

        case TimeSlot::Role::TIME_SKEW_ALLOWANCE:
          return true;

        default:
          except<std::logic_error>("Invalid role to be used as a default role in a TDMA frame");
          return true;
      }
    }
  }

  uint32_t frame_offset_us() const {
    if (frame_.frame_size_us > 0) {
      return time_after_frame_start_us() % frame_.frame_size_us;
    } else {
      return 0;
    }
  }

  uint32_t time_slot_offset_us() const {
    const TimeSlot* slot{current_time_slot()};
    if (slot != nullptr) {
      return frame_offset_us() - slot->start_us;
    } else {
      if (!frame_.time_slots.empty()) {
        const TimeSlot& last_slot{frame_.time_slots.back()};
        return frame_offset_us() - (last_slot.start_us + last_slot.duration_us);
      } else {
        return 0;
      }
    }
  }

  uint32_t frame_duration_us() const { return frame_.frame_size_us; }

  uint32_t time_slot_duration_us() const {
    const TimeSlot* slot{current_time_slot()};
    if (slot != nullptr) {
      return slot->duration_us;
    } else {
      if (!frame_.time_slots.empty()) {
        const TimeSlot& last_slot{frame_.time_slots.back()};
        return frame_.frame_size_us - (last_slot.start_us + last_slot.duration_us);
      } else {
        return 0;
      }
    }
  }

  uint32_t frame_duration_remaining_us() const { return frame_duration_us() - frame_offset_us(); }

  uint32_t time_slot_duration_remaining_us() const {
    return time_slot_duration_us() - time_slot_offset_us();
  }

  TimeSlot::Role time_slot_role() const {
    const TimeSlot* slot{current_time_slot()};
    if (slot != nullptr) {
      return slot->role;
    } else {
      return frame_.default_role;
    }
  }

  uint64_t time_slot_owner() const {
    const TimeSlot* slot{current_time_slot()};
    if (slot != nullptr) {
      return slot->slot_owner_id;
    } else {
      return 0;
    }
  }

  void set_frame(const Frame& frame) { frame_ = frame; }
  Frame& frame() { return frame_; }
  const Frame& frame() const { return frame_; }
};

}  // namespace tvsc::comms::tdma
