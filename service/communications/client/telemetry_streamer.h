#pragma once

#include <string>

#include "comms/radio/proto/settings.pb.h"
#include "grpcpp/support/client_callback.h"
#include "pubsub/streamer.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/common/communications.pb.h"

namespace tvsc::service::communications {

/**
 * Streaming publisher of the CommunicationsService::monitor() method.
 */
class TelemetryStreamer final
    : public tvsc::pubsub::GrpcStreamer<CommunicationsService, EmptyMessage,
                                        tvsc::comms::radio::proto::TelemetryEvent> {
 protected:
  void call_rpc_method(
      CommunicationsService::StubInterface::async_interface& async_stub,
      grpc::ClientContext& context, const EmptyMessage& request,
      grpc::ClientReadReactor<tvsc::comms::radio::proto::TelemetryEvent>& reactor) override {
    async_stub.monitor(&context, &request, &reactor);
  }

 public:
  static constexpr const char TOPIC_NAME[] =
      "tvsc.service.communications.CommunicationsService.monitor";

  TelemetryStreamer() {}

  TelemetryStreamer(const std::string& bind_addr) : GrpcStreamer(bind_addr) {}
};

}  // namespace tvsc::service::communications
