#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

#include "glog/logging.h"
#include "grpcpp/support/async_stream.h"
#include "grpcpp/support/client_callback.h"
#include "grpcpp/support/status.h"

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
class PubSubService final : grpc::ClientReadReactor<ResponseT> {
 public:
  using RpcMethodCallerT = std::function<void(grpc::ClientContext* context, const RequestT* request,
                                              grpc::ClientReadReactor<ResponseT>* reactor)>;

 private:
  Topic<ResponseT>* web_socket_topic_;
  // std::unique_ptr<grpc::ClientAsyncReaderInterface<ResponseT>> grpc_reader_{};

  RpcMethodCallerT call_rpc_fn_;

  grpc::ClientContext context_{};
  RequestT request_{};

  ResponseT response_{};
  grpc::Status status_{};

  mutable std::mutex mu_{};
  mutable std::condition_variable cv_;

  bool is_running_{false};

  void finalize(const grpc::Status& status) { status_ = status; }

  /**
   * Helper method to build a tag that can be used to tag entries in the gRPC completion queue.
   * Note that only the address of the tag is significant, not its value.
   */
  // void* tag() { return reinterpret_cast<void*>(this); }

 public:
  PubSubService(Topic<ResponseT>& topic, RpcMethodCallerT call_rpc_fn)
      : web_socket_topic_(&topic), call_rpc_fn_(std::move(call_rpc_fn)) {}

  PubSubService(Topic<ResponseT>& topic, RpcMethodCallerT call_rpc_fn, const RequestT& request)
      : web_socket_topic_(&topic), call_rpc_fn_(std::move(call_rpc_fn)), request_(request) {}

  template <typename Clock = std::chrono::system_clock>

  void start() {
    DLOG(INFO) << "PubSubService::start()";
    std::unique_lock<std::mutex> l(mu_);
    if (!is_running_) {
      LOG(INFO) << "PubSubService::start() -- initiating RPC";
      call_rpc_fn_(&context_, &request_, this);

      is_running_ = true;

      // Immediately request a read. These calls do not block.
      this->StartRead(&response_);
      this->StartCall();
    }
  }

  void OnReadDone(bool ok) override {
    DLOG(INFO) << "PubSubService::OnReadDone()";
    std::unique_lock<std::mutex> l(mu_);
    if (ok) {
      // Publish the response.
      web_socket_topic_->publish(response_);

      // Initiate a new read.
      this->StartRead(&response_);
    }
  }

  void OnDone(const grpc::Status& s) override {
    DLOG(INFO) << "PubSubService::OnDone()";
    std::unique_lock<std::mutex> l(mu_);
    status_ = s;
    is_running_ = false;
    cv_.notify_one();
  }

  void stop() {
    DLOG(INFO) << "PubSubService::stop()";
    std::unique_lock<std::mutex> l(mu_);
    context_.TryCancel();
  }

  const grpc::Status& await() const {
    DLOG(INFO) << "PubSubService::await()";
    std::unique_lock<std::mutex> l(mu_);
    cv_.wait(l, [this] { return !is_running_; });
    return status_;
  }

  const grpc::Status& status() const {
    std::unique_lock<std::mutex> l(mu_);
    return status_;
  }

  bool is_running() const {
    std::unique_lock<std::mutex> l(mu_);
    return is_running_;
  }
};

}  // namespace tvsc::services
