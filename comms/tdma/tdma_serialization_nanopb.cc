#include <cstdint>

#include "comms/tdma/nanopb_proto/tdma_frame.pb.h"
#include "comms/tdma/tdma_frame.h"
#include "comms/tdma/tdma_serialization.h"
#include "pb_decode.h"
#include "pb_encode.h"

namespace tvsc::comms::tdma {

template <typename T, typename U>
U convert_enum(T in) {
  return static_cast<U>(in);
}

bool encode(const Frame& frame, uint64_t current_time_us, uint8_t* buf, size_t& size) {
  tvsc_comms_tdma_nano_Frame proto{};

  proto.time_announced_us = current_time_us;

  proto.frame_start_time_us = frame.frame_start_time_us;
  proto.base_station_id = frame.base_station_id;

  for (const TimeSlot& slot : frame.time_slots) {
    tvsc_comms_tdma_nano_TimeSlot& proto_slot = proto.time_slots[proto.time_slots_count++];
    proto_slot.duration_us = slot.duration_us;
    proto_slot.role = convert_enum<TimeSlot::Role, tvsc_comms_tdma_nano_TimeSlot_Role>(slot.role);
    proto_slot.slot_owner_id = slot.slot_owner_id;
  }

  pb_ostream_t ostream = pb_ostream_from_buffer(buf, size);
  if (pb_encode(&ostream, nanopb::MessageDescriptor<tvsc_comms_tdma_nano_Frame>::fields(),
                &proto)) {
    size = ostream.bytes_written;
    return true;
  } else {
    return false;
  }
}

bool decode(Frame& frame, uint64_t& current_time_us, const uint8_t* buf, size_t size) {
  tvsc_comms_tdma_nano_Frame proto{};

  pb_istream_t istream = pb_istream_from_buffer(buf, size);
  if (!pb_decode(&istream, nanopb::MessageDescriptor<tvsc_comms_tdma_nano_Frame>::fields(),
                 &proto)) {
    return false;
  }

  current_time_us = proto.time_announced_us;

  frame.frame_start_time_us = proto.frame_start_time_us;
  frame.base_station_id = proto.base_station_id;

  for (pb_size_t slot_index = 0; slot_index < proto.time_slots_count; ++slot_index) {
    const tvsc_comms_tdma_nano_TimeSlot& proto_slot = proto.time_slots[slot_index];
    TimeSlot slot{};
    slot.duration_us = proto_slot.duration_us;
    slot.role = convert_enum<tvsc_comms_tdma_nano_TimeSlot_Role, TimeSlot::Role>(proto_slot.role);
    slot.slot_owner_id = proto_slot.slot_owner_id;
    frame.time_slots.emplace_back(std::move(slot));
  }

  pack_frame_times(frame);
  return true;
}

}  // namespace tvsc::comms::tdma
