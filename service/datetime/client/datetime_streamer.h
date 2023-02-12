#pragma once

#include <memory>
#include <string>

#include "discovery/service_types.h"
#include "grpcpp/grpcpp.h"
#include "pubsub/pub_sub_service.h"
#include "service/datetime/client/client.h"
#include "service/datetime/common/datetime.pb.h"

namespace tvsc::service::datetime {

/**
 * Integration of the Datetime::stream_datetime() RPC service method, the PubSubService, and the
 * Topic where the messages are pushed to clients.
 */
// TODO(james): The functionality of this class should be rolled into the PubSubService class and/or
// the DatetimeClient.
template <bool SSL>
class DatetimeStreamer {
 private:
  using PubSubT = tvsc::pubsub::PubSubService<DatetimeRequest, DatetimeReply>;

  static DatetimeRequest construct_request() {
    DatetimeRequest request{};
    request.set_precision(TimeUnit::MILLISECOND);
    request.set_period_count(100);
    request.set_period_unit(TimeUnit::MILLISECOND);
    return request;
  }

  PubSubT::RpcMethodCallerT construct_rpc_method_caller() {
    return [this](grpc::ClientContext* context, const DatetimeRequest* request,
                  grpc::ClientReadReactor<DatetimeReply>* reactor) {
      this->client_->stream(context, request, reactor);
    };
  }

 public:
  static constexpr const char* const TOPIC_NAME = Datetime::service_full_name();

  DatetimeStreamer(tvsc::pubsub::Topic<DatetimeReply>& topic)
      : DatetimeStreamer(topic, tvsc::discovery::service_url<Datetime>()) {}

  DatetimeStreamer(tvsc::pubsub::Topic<DatetimeReply>& topic, const std::string& bind_addr)
      : client_(std::make_unique<DatetimeClient>(bind_addr)),
        pub_sub_(
            std::make_unique<PubSubT>(topic, construct_rpc_method_caller(), construct_request())) {}

  /**
   * Start the gRPC stream.
   */
  void start() { pub_sub_->start(); }

 private:
  std::unique_ptr<DatetimeClient> client_;
  std::unique_ptr<PubSubT> pub_sub_;
};

}  // namespace tvsc::service::datetime
