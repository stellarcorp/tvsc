#include "comms/tdma/frame.h"

#include <cstdint>

#include "base/except.h"

namespace tvsc::comms::tdma {

void pack_frame_times(Frame& frame) {
  uint32_t next_slot_start_us = 0;
  frame.frame_duration_us = 0;
  for (TimeSlot& slot : frame.time_slots) {
    slot.start_us = next_slot_start_us;
    next_slot_start_us += slot.duration_us;
    frame.frame_duration_us += slot.duration_us;
  }
}

FrameBuilder::FrameBuilder(uint64_t frame_start_time_us) {
  frame_.frame_start_time_us = frame_start_time_us;
}

void FrameBuilder::set_base_station_id(uint64_t id) { frame_.base_station_id = id; }

void FrameBuilder::add_guard_interval(uint32_t duration_us) {
  TimeSlot slot{};
  slot.start_us = 0;
  slot.duration_us = duration_us;
  slot.role = TimeSlot::Role::GUARD_INTERVAL;
  frame_.time_slots.emplace_back(slot);
}

void FrameBuilder::add_node_tx_slot(uint32_t duration_us, uint64_t owner_id) {
  TimeSlot slot{};
  slot.start_us = 0;
  slot.duration_us = duration_us;
  slot.role = TimeSlot::Role::NODE_TX;
  slot.slot_owner_id = owner_id;
  frame_.time_slots.emplace_back(slot);
}

void FrameBuilder::add_association_slot(uint32_t duration_us) {
  TimeSlot slot{};
  slot.start_us = 0;
  slot.duration_us = duration_us;
  slot.role = TimeSlot::Role::ASSOCIATION;
  frame_.time_slots.emplace_back(slot);
}

void FrameBuilder::add_blackout_slot(uint32_t duration_us) {
  TimeSlot slot{};
  slot.start_us = 0;
  slot.duration_us = duration_us;
  slot.role = TimeSlot::Role::BLACKOUT;
  frame_.time_slots.emplace_back(slot);
}

Frame FrameBuilder::build() {
  if (frame_.time_slots.empty()) {
    except<std::logic_error>("Cannot create frame without time slots");
  }
  pack_frame_times(frame_);
  return std::move(frame_);
}

Frame FrameBuilder::create_default_node_frame() {
  FrameBuilder frame{};
  frame.add_guard_interval(100'000 /* The exact duration here does not matter, as long as it is long enough to complete a fragment transmission. */);
  return frame.build();
}

Frame FrameBuilder::create_default_base_station_frame(uint64_t base_station_id) {
  FrameBuilder frame{};
  frame.set_base_station_id(base_station_id);

  // Allow the base station to transmit.
  frame.add_node_tx_slot(100'000, base_station_id);

  // Provide for time skew.
  frame.add_guard_interval(10'000);

  // Provide for node association broadcasts.
  frame.add_association_slot(100'000);

  // Provide for time skew.
  frame.add_guard_interval(10'000);

  return frame.build();
}

}  // namespace tvsc::comms::tdma
