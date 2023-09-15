#pragma once

#include <condition_variable>
#include <future>
#include <map>
#include <memory>
#include <mutex>

#include "comms/packet/packet.h"
#include "comms/radio/half_duplex_radio.h"
#include "comms/radio/proto/settings.pb.h"
#include "comms/radio/radio_configuration.h"
#include "comms/radio/rf69hcw.h"
#include "comms/radio/rf69hcw_configuration.h"
#include "comms/radio/single_radio_pin_mapping.h"
#include "grpcpp/grpcpp.h"
#include "hal/spi/spi.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/common/communications.pb.h"

namespace tvsc::service::communications {

class CommunicationsServiceImpl final : public CommunicationsService::Service {
 private:
  using RadioT = tvsc::comms::radio::RF69HCW;
  static constexpr size_t MAX_TX_QUEUE_SIZE{16};
  static constexpr size_t MAX_FRAGMENTS_PER_PACKET{8};

  std::mutex mu_{};
  std::condition_variable receive_message_available_{};
  std::condition_variable monitor_event_available_{};

  std::map<grpc::ServerWriter<Message>*, std::vector<Message>> receive_writer_queues_{};
  std::map<grpc::ServerWriter<tvsc::comms::radio::proto::TelemetryEvent>*,
           std::vector<tvsc::comms::radio::proto::TelemetryEvent>>
      monitor_writer_queues_{};

  std::unique_ptr<tvsc::hal::spi::SpiBus> bus_{};
  std::unique_ptr<tvsc::hal::spi::SpiPeripheral> spi_peripheral_{};
  std::unique_ptr<RadioT> rf69_{};
  std::unique_ptr<tvsc::comms::radio::RadioConfiguration<RadioT>> configuration_{};

  std::future<void> monitor_task_{};

  void post_received_packet(const tvsc::comms::packet::Packet& packet);

  void reset_radio();

  void broadcast_shakespeare();
  std::future<void> sample_broadcast_task_{};
  std::atomic<bool> sample_broadcast_in_progress_{false};
  std::atomic<bool> broadcast_cancel_requested_{false};

 public:
  CommunicationsServiceImpl();

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
