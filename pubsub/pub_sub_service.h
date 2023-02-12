#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>

#include "glog/logging.h"
#include "grpcpp/support/client_callback.h"
#include "grpcpp/support/status.h"
#include "pubsub/streamer.h"
#include "pubsub/topic.h"

namespace tvsc::pubsub {

/**
 * Service to use streaming methods, such as those in gRPC services, to create a publish-subscribe
 * mechanism.
 *
 * This service wraps a Streamer, likely a streaming method of a gRPC client, and connects it to a
 * pub-sub Topic. That Topic is then responsible for publishing any messages sent to it.
 */
// TODO(james): The structure of this class and the Topic class above need to be rethought. The
// names don't match the expected structure for a pub-sub system. Investigate the design of
// RabbitMQ, ØMQ, Kafka, etc. to find better names and solidify the roles and responsibilities for
// each class, including possible new ones.
// TODO(james): Move the ClientReadReactor functionality into an internal class. It's an
// implementation detail. This class uses the functionality of a ClientReadReactor to accomplish its
// goals, but this class's main role is not to be just another form of ClientReadReactor.
template <typename ResponseT>
class PublicationService final : public grpc::ClientReadReactor<ResponseT> {
 private:
  Topic<ResponseT>* topic_;

  std::unique_ptr<Streamer<ResponseT>> streamer_;

  ResponseT response_{};
  grpc::Status status_{};

  mutable std::mutex mu_{};
  mutable std::condition_variable cv_;

  void OnReadDone(bool ok) override {
    DLOG_EVERY_N(INFO, 1000) << "PublicationService::OnReadDone()";
    std::unique_lock<std::mutex> l(mu_);
    if (ok) {
      // Publish the response to the PubSub topic.
      topic_->publish(response_);

      // Initiate a new read.
      this->StartRead(&response_);
    }
  }

  void OnDone(const grpc::Status& s) override {
    DLOG(INFO) << "PublicationService::OnDone()";
    {
      // Use a separate scope to ensure the lock gets released before calling stop().
      std::unique_lock<std::mutex> l(mu_);
      status_ = s;
    }
    stop();
  }

 public:
  PublicationService(Topic<ResponseT>& topic, std::unique_ptr<Streamer<ResponseT>> streamer)
      : topic_(&topic), streamer_(std::move(streamer)) {}

  void start() {
    DLOG(INFO) << "PublicationService::start()";
    std::unique_lock<std::mutex> l(mu_);
    if (!streamer_->is_running()) {
      DLOG(INFO) << "PublicationService::start() -- initiating RPC";
      streamer_->start_stream(*this);
      DLOG(INFO) << "PublicationService::start() -- streamer started";

      // Immediately request a read. These calls do not block.
      DLOG(INFO) << "PublicationService::start() -- calling StartRead()";
      this->StartRead(&response_);
      DLOG(INFO) << "PublicationService::start() -- calling StartCall()";
      this->StartCall();
    }
    DLOG(INFO) << "PublicationService::start() -- complete.";
  }

  void stop() {
    DLOG(INFO) << "PublicationService::stop()";
    std::unique_lock<std::mutex> l(mu_);
    streamer_->stop_stream();
    cv_.notify_one();
  }

  const grpc::Status& await() const {
    DLOG(INFO) << "PublicationService::await()";
    std::unique_lock<std::mutex> l(mu_);
    cv_.wait(l, [this] { return !streamer_->is_running(); });
    return status_;
  }

  const grpc::Status& status() const {
    std::unique_lock<std::mutex> l(mu_);
    return status_;
  }

  bool is_running() const {
    std::unique_lock<std::mutex> l(mu_);
    return streamer_->is_running();
  }
};

}  // namespace tvsc::pubsub
