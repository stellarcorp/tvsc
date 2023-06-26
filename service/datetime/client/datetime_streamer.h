#pragma once

#include <string>

#include "grpcpp/support/client_callback.h"
#include "pubsub/streamer.h"
#include "service/datetime/common/datetime.grpc.pb.h"
#include "service/datetime/common/datetime.pb.h"

namespace tvsc::service::datetime {

/**
 * Streaming publisher of the Datetime::stream_datetime() method.
 */
class DatetimeStreamer final
    : public tvsc::pubsub::GrpcStreamer<Datetime, DatetimeRequest, DatetimeReply> {
 private:
  static DatetimeRequest construct_default_request() { return DatetimeRequest{}; }

 protected:
  void call_rpc_method(Datetime::StubInterface::async_interface& async_stub,
                       grpc::ClientContext& context, const DatetimeRequest& request,
                       grpc::ClientReadReactor<DatetimeReply>& reactor) override {
    async_stub.stream_datetime(&context, &request, &reactor);
  }

 public:
  static constexpr const char TOPIC_NAME[] = "tvsc.service.datetime.Datetime.stream_datetime";

  DatetimeStreamer() { request() = construct_default_request(); }

  DatetimeStreamer(const std::string& bind_addr) : GrpcStreamer(bind_addr) {
    request() = construct_default_request();
  }
};

}  // namespace tvsc::service::datetime
