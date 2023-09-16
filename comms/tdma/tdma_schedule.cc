#include "comms/tdma/tdma_schedule.h"

namespace tvsc::comms::tdma {

bool TdmaSchedule::is_associated(uint64_t id) const {
  for (const TimeSlot& slot : frame_.time_slots) {
    if (slot.role == TimeSlot::Role::NODE_TX && slot.slot_owner_id == id) {
      return true;
    }
  }
  return false;
}

bool TdmaSchedule::can_transmit() const {
  const TimeSlot& slot{current_time_slot()};
  switch (slot.role) {
    case TimeSlot::Role::BLACKOUT:
      return false;
    case TimeSlot::Role::NODE_TX:
      return slot.slot_owner_id == id_;
    case TimeSlot::Role::ASSOCIATION:
      return !is_associated() && !is_base_station();
    case TimeSlot::Role::TIME_SKEW_ALLOWANCE:
      return false;
  }
}

bool TdmaSchedule::should_receive() const {
  const TimeSlot& slot{current_time_slot()};
  switch (slot.role) {
    case TimeSlot::Role::BLACKOUT:
      return false;
    case TimeSlot::Role::NODE_TX:
      return slot.slot_owner_id != id_;
    case TimeSlot::Role::ASSOCIATION:
      return is_base_station();
    case TimeSlot::Role::TIME_SKEW_ALLOWANCE:
      return true;
  }
}

}  // namespace tvsc::comms::tdma
