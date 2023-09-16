#include "comms/tdma/tdma_frame.h"

#include <cstdint>

#include "base/except.h"

namespace tvsc::comms::tdma {

FrameBuilder::FrameBuilder(uint64_t frame_start_time_us) {
  frame_.frame_start_time_us = frame_start_time_us;
}

void FrameBuilder::set_base_station_id(uint64_t id) { frame_.base_station_id = id; }

void FrameBuilder::add_time_skew_slot(uint32_t duration_us) {
  TimeSlot slot{};
  slot.start_us = 0;
  if (!frame_.time_slots.empty()) {
    const TimeSlot& last_slot{frame_.time_slots.back()};
    slot.start_us = last_slot.start_us + last_slot.duration_us;
  }
  slot.duration_us = duration_us;
  slot.role = TimeSlot::Role::TIME_SKEW_ALLOWANCE;
  frame_.time_slots.emplace_back(slot);
}

void FrameBuilder::add_node_tx_slot(uint32_t duration_us, uint64_t owner_id) {
  TimeSlot slot{};
  slot.start_us = 0;
  if (!frame_.time_slots.empty()) {
    const TimeSlot& last_slot{frame_.time_slots.back()};
    slot.start_us = last_slot.start_us + last_slot.duration_us;
  }
  slot.duration_us = duration_us;
  slot.role = TimeSlot::Role::NODE_TX;
  slot.slot_owner_id = owner_id;
  frame_.time_slots.emplace_back(slot);
}

void FrameBuilder::add_association_slot(uint32_t duration_us) {
  TimeSlot slot{};
  slot.start_us = 0;
  if (!frame_.time_slots.empty()) {
    const TimeSlot& last_slot{frame_.time_slots.back()};
    slot.start_us = last_slot.start_us + last_slot.duration_us;
  }
  slot.duration_us = duration_us;
  slot.role = TimeSlot::Role::ASSOCIATION;
  frame_.time_slots.emplace_back(slot);
}

void FrameBuilder::add_blackout_slot(uint32_t duration_us) {
  TimeSlot slot{};
  slot.start_us = 0;
  if (!frame_.time_slots.empty()) {
    const TimeSlot& last_slot{frame_.time_slots.back()};
    slot.start_us = last_slot.start_us + last_slot.duration_us;
  }
  slot.duration_us = duration_us;
  slot.role = TimeSlot::Role::BLACKOUT;
  frame_.time_slots.emplace_back(slot);
}

void FrameBuilder::consolidate_frame_size() {
  frame_.frame_duration_us = 0;
  for (const TimeSlot& slot : frame_.time_slots) {
    frame_.frame_duration_us += slot.duration_us;
  }
}

Frame FrameBuilder::build() {
  if (frame_.time_slots.empty()) {
    except<std::logic_error>("Cannot create frame without time slots");
  }
  consolidate_frame_size();
  return std::move(frame_);
}

Frame FrameBuilder::create_default_node_frame() {
  FrameBuilder frame{};
  frame.add_time_skew_slot(100'000 /* The exact duration here does not matter, as long as it is long enough to complete a fragment transmission. */);
  return frame.build();
}

Frame FrameBuilder::create_default_base_station_frame(uint64_t base_station_id) {
  FrameBuilder frame{};
  frame.set_base_station_id(base_station_id);

  // Allow the base station to transmit.
  frame.add_node_tx_slot(100'000, base_station_id);

  // Provide for time skew.
  frame.add_time_skew_slot(10'000);

  // Provide for node association broadcasts.
  frame.add_association_slot(100'000);

  // Provide for time skew.
  frame.add_time_skew_slot(10'000);

  return frame.build();
}

}  // namespace tvsc::comms::tdma
