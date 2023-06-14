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
  std::condition_variable cv_{};

  std::map<grpc::ServerWriter<Message>*, std::vector<Message>> writer_queues_{};

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

 public:
  CommunicationsServiceImpl();

  grpc::Status transmit(grpc::ServerContext* context, const Message* request,
                        SuccessResult* reply) override;

  grpc::Status receive(grpc::ServerContext* context, const EmptyMessage* request,
                       grpc::ServerWriter<Message>* writer) override;
};

}  // namespace tvsc::service::communications
