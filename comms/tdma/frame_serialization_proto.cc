#include <cstdint>

#include "comms/tdma/frame.h"
#include "comms/tdma/frame_serialization.h"
#include "comms/tdma/proto/frame.pb.h"

namespace tvsc::comms::tdma {

template <typename T, typename U>
U convert_enum(T in) {
  return static_cast<U>(in);
}

bool encode(const Frame& frame, uint64_t current_time_us, uint8_t* buf, size_t& size) {
  tvsc::comms::tdma::proto::Frame proto{};

  proto.set_time_announced_us(current_time_us);

  proto.set_frame_start_time_us(frame.frame_start_time_us);
  proto.set_base_station_id(frame.base_station_id);

  for (const TimeSlot& slot : frame.time_slots) {
    tvsc::comms::tdma::proto::TimeSlot* proto_slot = proto.add_time_slots();
    proto_slot->set_duration_us(slot.duration_us);
    proto_slot->set_role(
        convert_enum<TimeSlot::Role, tvsc::comms::tdma::proto::TimeSlot::Role>(slot.role));
    proto_slot->set_slot_owner_id(slot.slot_owner_id);
  }

  const size_t proto_byte_size{proto.ByteSizeLong()};
  if (proto_byte_size <= size) {
    proto.SerializeWithCachedSizesToArray(buf);
    size = proto_byte_size;
    return true;
  } else {
    return false;
  }
}

bool decode(Frame& frame, uint64_t& current_time_us, const uint8_t* buf, size_t size) {
  tvsc::comms::tdma::proto::Frame proto{};

  bool result{proto.ParseFromArray(buf, size)};

  if (result) {
    current_time_us = proto.time_announced_us();

    frame.frame_start_time_us = proto.frame_start_time_us();
    frame.base_station_id = proto.base_station_id();

    for (const tvsc::comms::tdma::proto::TimeSlot& proto_slot : proto.time_slots()) {
      TimeSlot slot{};
      slot.duration_us = proto_slot.duration_us();
      slot.role =
          convert_enum<tvsc::comms::tdma::proto::TimeSlot::Role, TimeSlot::Role>(proto_slot.role());
      slot.slot_owner_id = proto_slot.slot_owner_id();
      frame.time_slots.emplace_back(std::move(slot));
    }

    pack_frame_times(frame);
  }

  return result;
}

}  // namespace tvsc::comms::tdma
