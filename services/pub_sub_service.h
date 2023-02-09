#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include "glog/logging.h"
#include "grpcpp/support/status.h"
#include "grpcpp/support/sync_stream.h"

namespace tvsc::services {

/**
 * Should a particular MessageT type get compressed when sending over a socket?
 *
 * This implementation provides a default. Add other implementations as desired to enable/disable
 * compression for each type. Note that submessages within this type are compressed according
 * to this setting when they are sent as part of that top-level type.
 */
template <typename MessageT>
constexpr bool should_compress() {
  return false;
}

template <typename MessageT>
class Topic {
 protected:
  const std::string topic_name_;

  virtual void publish_compressed(std::string_view msg) = 0;
  virtual void publish_uncompressed(std::string_view msg) = 0;

 public:
  Topic(std::string_view topic_name) : topic_name_(topic_name) {}
  virtual ~Topic() = default;

  void publish(const MessageT& msg) {
    std::string serialized{};
    msg.SerializeToString(&serialized);
    if constexpr (should_compress<MessageT>()) {
      publish_compressed(serialized);
    } else {
      publish_uncompressed(serialized);
    }
  }
};

template <typename RequestT, typename ResponseT>
class PubSubService final {
 public:
  using RpcMethodCallerT = std::function<std::unique_ptr<grpc::ClientReaderInterface<ResponseT>>(
      grpc::ClientContext* context, const RequestT& request)>;

 private:
  Topic<ResponseT>* web_socket_topic_;
  std::unique_ptr<grpc::ClientReaderInterface<ResponseT>> grpc_reader_{};

  RpcMethodCallerT call_rpc_fn_;

  grpc::ClientContext context_{};
  RequestT request_{};

  grpc::Status status_{};

  void finalize(const grpc::Status& status) { status_ = status; }

 public:
  PubSubService(Topic<ResponseT>& topic, RpcMethodCallerT call_rpc_fn)
      : web_socket_topic_(&topic), call_rpc_fn_(std::move(call_rpc_fn)) {}

  PubSubService(Topic<ResponseT>& topic, RpcMethodCallerT call_rpc_fn, const RequestT& request)
      : web_socket_topic_(&topic), call_rpc_fn_(std::move(call_rpc_fn)), request_(request) {}

  void loop() {
    using namespace std::literals::chrono_literals;

    grpc_reader_ = call_rpc_fn_(&context_, request_);

    ResponseT response{};
    while (grpc_reader_->Read(&response)) {
      web_socket_topic_->publish(response);
    }
    status_ = grpc_reader_->Finish();
  }

  void stop() {
    context_.TryCancel();
    grpc_reader_.reset();
  }

  const grpc::Status& status() const { return status_; }
};

}  // namespace tvsc::services
