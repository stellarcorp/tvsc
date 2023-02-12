#pragma once

#include <memory>
#include <string>

#include "App.h"
#include "discovery/service_types.h"
#include "grpcpp/grpcpp.h"
#include "service/datetime/client/client.h"
#include "service/datetime/common/datetime.pb.h"
#include "services/pub_sub_service.h"
#include "services/web_socket_topic.h"

namespace tvsc::service::datetime {

/**
 * Integration of the Datetime::stream_datetime() RPC service method, the PubSubService, and the
 * Topic where the messages are pushed to clients.
 */
// TODO(james): Rename this class, probably to DatetimeStreamer, while implementing refactoring(s)
// indicated below.
template <bool SSL>
class StreamingDatetimeClient {
 private:
  static DatetimeRequest construct_request() {
    DatetimeRequest request{};
    request.set_precision(TimeUnit::MILLISECOND);
    request.set_period_count(90);
    request.set_period_unit(TimeUnit::MILLISECOND);
    return request;
  }

 public:
  static constexpr const char* const TOPIC_NAME = Datetime::service_full_name();

  StreamingDatetimeClient(uWS::TemplatedApp<SSL>& app)
      : StreamingDatetimeClient(app, tvsc::discovery::service_url<Datetime>()) {}

  StreamingDatetimeClient(uWS::TemplatedApp<SSL>& app, const std::string& bind_addr)
      : app_(&app),
        topic_(std::make_unique<tvsc::services::WebSocketTopic<DatetimeReply, 1>>(TOPIC_NAME)),
        client_(std::make_unique<DatetimeClient>(bind_addr)),
        pub_sub_(std::make_unique<tvsc::services::PubSubService<DatetimeRequest, DatetimeReply>>(
            *topic_,
            [this](grpc::ClientContext* context, const DatetimeRequest* request,
                   grpc::ClientReadReactor<DatetimeReply>* reactor) {
              return this->client_->stream(context, request, reactor);
            },
            construct_request())) {}

  /**
   * All of the web socket message sending and publishing must occur on the thread(s) allocated to
   * the uWS::App instance's Loop. Trying to publish from other threads to the same Loop instance
   * tends to cause deadlock/hanging issues.
   */
  // TODO(james): This is an artifact of uWS's design. We should limit the effect of that library by
  // moving this and related concepts into the WebSocketTopic class. This class would then take a
  // reference to a Topic instance, instead of owning the instance of WebSocketTopic.
  void register_publishing_handler(uWS::Loop& loop) {
    loop.addPreHandler(
        this, [this](uWS::Loop* /*unused*/) { this->topic_->transfer_messages(*this->app_); });
  }

  /**
   * Start the gRPC stream.
   */
  void start() { pub_sub_->start(); }

 private:
  uWS::TemplatedApp<SSL>* app_;
  std::unique_ptr<tvsc::services::WebSocketTopic<DatetimeReply, 1>> topic_;
  std::unique_ptr<DatetimeClient> client_;
  std::unique_ptr<tvsc::services::PubSubService<DatetimeRequest, DatetimeReply>> pub_sub_;
};

}  // namespace tvsc::service::datetime
