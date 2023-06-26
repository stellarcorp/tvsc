#pragma once

#include <condition_variable>
#include <future>
#include <map>
#include <memory>
#include <mutex>

#include "grpcpp/grpcpp.h"
#include "hal/spi/spi.h"
#include "radio/packet.h"
#include "radio/packet_queue.h"
#include "radio/proto/settings.pb.h"
#include "radio/radio_configuration.h"
#include "radio/rf69hcw.h"
#include "radio/rf69hcw_configuration.h"
#include "radio/single_radio_pin_mapping.h"
#include "radio/transceiver.h"
#include "radio/transceiver_monitor.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/common/communications.pb.h"

namespace tvsc::service::communications {

class CommunicationsServiceImpl final : public CommunicationsService::Service {
 private:
  using RadioT = tvsc::radio::RF69HCW;
  static constexpr size_t MAX_TX_QUEUE_SIZE{16};
  static constexpr size_t MAX_FRAGMENTS_PER_PACKET{8};

  std::mutex mu_{};
  std::condition_variable receive_message_available_{};
  std::condition_variable monitor_event_available_{};

  std::map<grpc::ServerWriter<Message>*, std::vector<Message>> receive_writer_queues_{};
  std::map<grpc::ServerWriter<tvsc::radio::proto::TelemetryEvent>*,
           std::vector<tvsc::radio::proto::TelemetryEvent>>
      monitor_writer_queues_{};

  std::unique_ptr<tvsc::hal::spi::SpiBus> bus_{};
  std::unique_ptr<tvsc::hal::spi::SpiPeripheral> spi_peripheral_{};
  std::unique_ptr<RadioT> rf69_{};
  std::unique_ptr<tvsc::radio::RadioConfiguration<RadioT>> configuration_{};
  std::unique_ptr<tvsc::radio::TransceiverMonitor<tvsc::radio::Packet, RadioT::max_mtu(),
                                                  MAX_TX_QUEUE_SIZE, MAX_FRAGMENTS_PER_PACKET>>
      monitor_{};

  tvsc::radio::PacketTxQueue<tvsc::radio::Packet, MAX_TX_QUEUE_SIZE> tx_queue_{};
  tvsc::radio::PacketAssembler<tvsc::radio::Packet> rx_queue_{};

  std::future<void> monitor_task_{};

  void post_received_packet(const tvsc::radio::Packet& packet);

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

  grpc::Status monitor(grpc::ServerContext* context, const EmptyMessage* request,
                       grpc::ServerWriter<tvsc::radio::proto::TelemetryEvent>* writer) override;

  grpc::Status begin_sample_broadcast(grpc::ServerContext* context, const EmptyMessage* request,
                                      EmptyMessage* reply) override;

  grpc::Status end_sample_broadcast(grpc::ServerContext* context, const EmptyMessage* request,
                                    EmptyMessage* reply) override;
};

}  // namespace tvsc::service::communications
