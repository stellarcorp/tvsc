#pragma once

#include <string>

#include "grpcpp/support/client_callback.h"
#include "pubsub/streamer.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/common/communications.pb.h"

namespace tvsc::service::communications {

/**
 * Streaming publisher of the CommunicationsService::receive() method.
 */
class RxStreamer final
    : public tvsc::pubsub::GrpcStreamer<CommunicationsService, EmptyMessage, Message> {
 protected:
  void call_rpc_method(CommunicationsService::StubInterface::async_interface& async_stub,
                       grpc::ClientContext& context, const EmptyMessage& request,
                       grpc::ClientReadReactor<Message>& reactor) override {
    async_stub.receive(&context, &request, &reactor);
  }

 public:
  static constexpr const char TOPIC_NAME[] =
      "tvsc.service.communications.CommunicationsService.receive";

  RxStreamer() {}

  RxStreamer(const std::string& bind_addr) : GrpcStreamer(bind_addr) {}
};

}  // namespace tvsc::service::communications
