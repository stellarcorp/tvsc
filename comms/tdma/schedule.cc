#include "comms/tdma/schedule.h"

namespace tvsc::comms::tdma {

bool Schedule::is_associated(uint64_t id) const {
  for (const TimeSlot& slot : frame_.time_slots) {
    if (slot.role == TimeSlot::Role::NODE_TX && slot.slot_owner_id == id) {
      return true;
    }
  }
  return false;
}

bool Schedule::can_transmit() const {
  const TimeSlot& slot{current_time_slot()};
  switch (slot.role) {
    case TimeSlot::Role::BLACKOUT:
      return false;
    case TimeSlot::Role::NODE_TX:
      return slot.slot_owner_id == id_;
    case TimeSlot::Role::ASSOCIATION:
      return !is_associated() && !is_base_station();
    case TimeSlot::Role::GUARD_INTERVAL:
      return false;
  }
}

bool Schedule::should_receive() const {
  const TimeSlot& slot{current_time_slot()};
  switch (slot.role) {
    case TimeSlot::Role::BLACKOUT:
      return false;
    case TimeSlot::Role::NODE_TX:
      return slot.slot_owner_id != id_;
    case TimeSlot::Role::ASSOCIATION:
      return is_base_station();
    case TimeSlot::Role::GUARD_INTERVAL:
      return true;
  }
}

}  // namespace tvsc::comms::tdma