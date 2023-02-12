#pragma once

#include <memory>
#include <string>

#include "discovery/service_types.h"
#include "grpcpp/create_channel.h"
#include "grpcpp/support/client_callback.h"

namespace tvsc::pubsub {

/**
 * Publisher of a stream of messages of type MessageT.
 *
 * Streamers provide the source of messages to be published in a PublicationService.
 */
template <typename MessageT>
class Streamer {
 public:
  virtual ~Streamer() = default;

  virtual void start_stream(grpc::ClientReadReactor<MessageT>& reactor) = 0;
  virtual void stop_stream() = 0;
  virtual bool is_running() const = 0;
};

/**
 * Variant of Streamer to publish the output of gRPC streaming methods.
 *
 * Implementations only need to implement the call_rpc_method() function, choosing the specific
 * function on the service to call, and to provide appropriate constructors.
 */
template <typename ServiceT, typename RequestT, typename ResponseT>
class GrpcStreamer : public Streamer<ResponseT> {
 private:
  std::unique_ptr<typename ServiceT::StubInterface> stub_;
  grpc::ClientContext context_{};
  RequestT request_{};
  bool is_running_{false};

  static std::unique_ptr<typename ServiceT::StubInterface> construct_stub(
      const std::string& bind_addr) {
    return ServiceT::NewStub(grpc::CreateChannel(bind_addr, grpc::InsecureChannelCredentials()));
  }

 protected:
  virtual void call_rpc_method(typename ServiceT::StubInterface::async_interface& async_stub,
                               grpc::ClientContext& context, const RequestT& request,
                               grpc::ClientReadReactor<ResponseT>& reactor) = 0;

 public:
  GrpcStreamer() : GrpcStreamer(tvsc::discovery::service_url<ServiceT>()) {}

  GrpcStreamer(const std::string& bind_addr) : stub_(construct_stub(bind_addr)) {}

  virtual ~GrpcStreamer() = default;

  const RequestT& request() const { return request_; }
  RequestT& request() { return request_; }

  void start_stream(grpc::ClientReadReactor<ResponseT>& reactor) override {
    if (!is_running_) {
      call_rpc_method(*stub_->async(), context_, request_, reactor);
      is_running_ = true;
    }
  }

  void stop_stream() override {
    if (is_running_) {
      context_.TryCancel();
      is_running_ = false;
    }
  }

  bool is_running() const override { return is_running_; }
};

}  // namespace tvsc::pubsub
