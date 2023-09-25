#pragma once

namespace tvsc::comms::route {

template <typename PacketT>
class PacketRouter {
 public:
  virtual ~PacketRouter() = default;

  /**
   * Route a packet.
   *
   * Returns true if this router is the final handler for this packet; false if it should be passed
   * to other routers.
   */
  virtual bool route(const PacketT& packet) = 0;
};

}  // namespace tvsc::comms::route
