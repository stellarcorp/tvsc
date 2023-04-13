#pragma once

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>

#include "grpcpp/grpcpp.h"
#include "hal/spi/spi.h"
#include "radio/packet.h"
#include "radio/radio_configuration.h"
#include "radio/rf69hcw.h"
#include "radio/rf69hcw_configuration.h"
#include "radio/single_radio_pin_mapping.h"
#include "radio/packet.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/common/communications.pb.h"

namespace tvsc::service::communications {

class CommunicationsServiceImpl final : public CommunicationsService::Service {
 private:
  std::mutex mu_{};
  std::condition_variable cv_{};

  std::map<grpc::ServerWriter<Message>*, std::vector<Message>> writer_queues_{};

  std::unique_ptr<tvsc::hal::spi::SpiBus> bus_{};
  std::unique_ptr<tvsc::hal::spi::SpiPeripheral> spi_peripheral_{};
  std::unique_ptr<tvsc::radio::RF69HCW> rf69_{};
  std::unique_ptr<tvsc::radio::RadioConfiguration<tvsc::radio::RF69HCW>> configuration_{};

  void post_received_packet(const tvsc::radio::Packet& packet);

  void receive_packets();

  void reset_radio();

 public:
  CommunicationsServiceImpl();

  grpc::Status list_radios(grpc::ServerContext* context, const EmptyMessage* request,
                           Radios* reply) override;

  grpc::Status transmit(grpc::ServerContext* context, const Message* request,
                        SuccessResult* reply) override;

  grpc::Status receive(grpc::ServerContext* context, const EmptyMessage* request,
                       grpc::ServerWriter<Message>* writer) override;
};

}  // namespace tvsc::service::communications
