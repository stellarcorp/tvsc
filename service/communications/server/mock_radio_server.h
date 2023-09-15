#pragma once

#include <condition_variable>
#include <future>
#include <map>
#include <memory>
#include <mutex>

#include "comms/radio/proto/settings.pb.h"
#include "grpcpp/grpcpp.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/common/communications.pb.h"

namespace tvsc::service::communications {

class MockRadioCommunicationsService final : public CommunicationsService::Service {
 private:
  std::mutex mu_{};
  std::condition_variable receive_message_available_{};
  std::condition_variable monitor_event_available_{};

  std::map<grpc::ServerWriter<Message>*, std::vector<Message>> receive_writer_queues_{};
  std::map<grpc::ServerWriter<tvsc::comms::radio::proto::TelemetryEvent>*,
           std::vector<tvsc::comms::radio::proto::TelemetryEvent>>
      monitor_writer_queues_{};

 public:
  grpc::Status transmit(grpc::ServerContext* context, const Message* request,
                        SuccessResult* reply) override;

  grpc::Status receive(grpc::ServerContext* context, const EmptyMessage* request,
                       grpc::ServerWriter<Message>* writer) override;

  grpc::Status monitor(
      grpc::ServerContext* context, const EmptyMessage* request,
      grpc::ServerWriter<tvsc::comms::radio::proto::TelemetryEvent>* writer) override;

  grpc::Status begin_sample_broadcast(grpc::ServerContext* context, const EmptyMessage* request,
                                      EmptyMessage* reply) override;

  grpc::Status end_sample_broadcast(grpc::ServerContext* context, const EmptyMessage* request,
                                    EmptyMessage* reply) override;
};

}  // namespace tvsc::service::communications
