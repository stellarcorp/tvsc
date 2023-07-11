#include "service/communications/server/server.h"

#include <chrono>
#include <condition_variable>
#include <ctime>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>

#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "hal/spi/spi.h"
#include "hal/time/time.h"
#include "radio/packet.h"
#include "radio/proto/settings.pb.h"
#include "radio/radio_configuration.h"
#include "radio/rf69hcw.h"
#include "radio/rf69hcw_configuration.h"
#include "radio/single_radio_pin_mapping.h"
#include "radio/transceiver.h"
#include "random/random.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/common/communications.pb.h"
#include "service/communications/server/shakespeare.h"

namespace tvsc::service::communications {

using namespace std::literals::chrono_literals;

void CommunicationsServiceImpl::reset_radio() {
  rf69_->reset();

  configuration_->change_values(tvsc::radio::default_configuration<RadioT>());
  configuration_->commit_changes();
}

CommunicationsServiceImpl::CommunicationsServiceImpl() {
  const uint8_t RF69_RST{tvsc::radio::SingleRadioPinMapping::reset_pin()};
  const uint8_t RF69_CS{tvsc::radio::SingleRadioPinMapping::chip_select_pin()};
  const uint8_t RF69_DIO0{tvsc::radio::SingleRadioPinMapping::interrupt_pin()};

  tvsc::random::initialize_seed();
  tvsc::hal::gpio::initialize_gpio();

  bus_ = std::make_unique<tvsc::hal::spi::SpiBus>(tvsc::hal::spi::get_default_spi_bus());
  spi_peripheral_ = std::make_unique<tvsc::hal::spi::SpiPeripheral>(*bus_, RF69_CS, 0x80);
  rf69_ = std::make_unique<tvsc::radio::RF69HCW>(*spi_peripheral_, RF69_DIO0, RF69_RST);
  configuration_ = std::make_unique<tvsc::radio::RadioConfiguration<RadioT>>(
      *rf69_, tvsc::radio::SingleRadioPinMapping::board_name());

  monitor_ = std ::make_unique<tvsc::radio::TransceiverMonitor<
      tvsc::radio::Packet, RadioT::max_mtu(), MAX_TX_QUEUE_SIZE, MAX_FRAGMENTS_PER_PACKET>>(
      *rf69_, tx_queue_, rx_queue_,
      [this](const tvsc::radio::Packet& packet) { this->post_received_packet(packet); });

  reset_radio();

  monitor_task_ = std::async(
      std::launch::async,
      &tvsc::radio::TransceiverMonitor<tvsc::radio::Packet, RadioT::max_mtu(), MAX_TX_QUEUE_SIZE,
                                       MAX_FRAGMENTS_PER_PACKET>::start,
      monitor_.get());
}

void CommunicationsServiceImpl::post_received_packet(const tvsc::radio::Packet& packet) {
  if (packet.protocol() == tvsc::radio::Protocol::INET) {
    Message message{};
    message.ParseFromString(std::string(packet.payload().as_string_view(packet.payload_length())));

    std::lock_guard<std::mutex> l(mu_);
    for (auto& writer_queue : receive_writer_queues_) {
      // TODO(james): Make this more efficient. This approach just copies the message to the queue
      // for every writer. Better would be to share a single instance of the message across all of
      // the writers.
      LOG(WARNING)
          << "CommunicationsServerImpl::post_received_packet() -- posting to receive writer queue";
      writer_queue.second.push_back(message);
    }

    LOG(WARNING) << "CommunicationsServerImpl::post_received_packet() -- Received INET packet. "
                    "Notifying writers";
    receive_message_available_.notify_all();
  } else if (packet.protocol() == tvsc::radio::Protocol::TVSC_TELEMETRY) {
    tvsc::radio::proto::TelemetryEvent event{};
    event.ParseFromString(std::string(packet.payload().as_string_view(packet.payload_length())));

    LOG(WARNING) << "CommunicationsServerImpl::post_received_packet() -- Received telemetry packet. "
                    "Notifying writers";

    std::lock_guard<std::mutex> l(mu_);
    for (auto& writer_queue : monitor_writer_queues_) {
      // TODO(james): Make this more efficient. This approach just copies the message to the queue
      // for every writer. Better would be to share a single instance of the message across all of
      // the writers.
      writer_queue.second.push_back(event);
    }

    monitor_event_available_.notify_all();
  }
}

grpc::Status CommunicationsServiceImpl::transmit(grpc::ServerContext* context,
                                                 const Message* request, SuccessResult* reply) {
  LOG(WARNING) << "CommunicationsServiceImpl::transmit() -- message: " << request->message();
  tvsc::radio::Packet packet{};
  packet.set_protocol(tvsc::radio::Protocol::INET);
  packet.set_sender_id(configuration_->id());
  // packet.set_sequence_number(?);

  request->SerializeToArray(packet.payload().data(), packet.payload().size());
  packet.set_payload_length(request->ByteSizeLong());
  tx_queue_.push_normal(packet);
  LOG(WARNING) << "CommunicationsServiceImpl::transmit() -- message pushed to TX queue";
  return grpc::Status::OK;
}

grpc::Status CommunicationsServiceImpl::receive(grpc::ServerContext* context,
                                                const EmptyMessage* /*request*/,
                                                grpc::ServerWriter<Message>* writer) {
  using namespace std::literals::chrono_literals;
  std::unique_lock<std::mutex> l(mu_);
  receive_writer_queues_.emplace(writer, std::vector<Message>{});

  while (!context->IsCancelled()) {
    if (receive_message_available_.wait_for(l, 20ms,
                                            [context] { return context->IsCancelled(); })) {
      break;
    }

    auto& queue{receive_writer_queues_.at(writer)};
    for (const auto& msg : queue) {
      LOG(WARNING) << "CommunicationsServiceImpl::receive() -- writing message.";
      writer->Write(msg);
    }
    queue.clear();
  }
  LOG(WARNING) << "CommunicationsServiceImpl::receive() -- exiting receive(). context->IsCancelled(): "
               << (context->IsCancelled() ? "true" : "false");

  receive_writer_queues_.erase(writer);

  LOG(WARNING) << "CommunicationsServiceImpl::receive() -- exiting.";
  // Client-side cancelling of the stream is expected, so we return OK instead of CANCELLED.
  return grpc::Status::OK;
}

grpc::Status CommunicationsServiceImpl::monitor(
    grpc::ServerContext* context, const EmptyMessage* /*request*/,
    grpc::ServerWriter<tvsc::radio::proto::TelemetryEvent>* writer) {
  using namespace std::literals::chrono_literals;
  std::unique_lock<std::mutex> l(mu_);
  monitor_writer_queues_.emplace(writer, std::vector<tvsc::radio::proto::TelemetryEvent>{});

  while (!context->IsCancelled()) {
    if (monitor_event_available_.wait_for(l, 20ms, [context] { return context->IsCancelled(); })) {
      break;
    }

    auto& queue{monitor_writer_queues_.at(writer)};
    for (const auto& msg : queue) {
      writer->Write(msg);
    }
    queue.clear();
  }
  monitor_writer_queues_.erase(writer);

  LOG(WARNING) << "CommunicationsServiceImpl::monitor() -- exiting.";
  // Client-side cancelling of the stream is expected, so we return OK instead of CANCELLED.
  return grpc::Status::OK;
}

grpc::Status CommunicationsServiceImpl::begin_sample_broadcast(grpc::ServerContext* context,
                                                               const EmptyMessage* /*request*/,
                                                               EmptyMessage* /*reply*/) {
  LOG(INFO) << "CommunicationsServiceImpl::begin_sample_broadcast()";
  if (!sample_broadcast_in_progress_) {
    broadcast_cancel_requested_ = false;
    sample_broadcast_task_ =
        std::async(std::launch::async, &CommunicationsServiceImpl::broadcast_shakespeare, this);

    sample_broadcast_in_progress_ = true;
  }

  return grpc::Status::OK;
}

grpc::Status CommunicationsServiceImpl::end_sample_broadcast(grpc::ServerContext* context,
                                                             const EmptyMessage* /*request*/,
                                                             EmptyMessage* /*reply*/) {
  LOG(INFO) << "CommunicationsServiceImpl::end_sample_broadcast()";
  if (sample_broadcast_in_progress_) {
    broadcast_cancel_requested_ = true;
  }

  return grpc::Status::OK;
}

void CommunicationsServiceImpl::broadcast_shakespeare() {
  LOG(INFO) << "CommunicationsServiceImpl::broadcast_shakespeare() -- Thread starting.";
  char buffer[tvsc::radio::Packet::max_payload_size()];
  Shakespeare shakespeare{};
  while (!broadcast_cancel_requested_) {
    tvsc::radio::Packet packet{};
    packet.set_protocol(tvsc::radio::Protocol::INET);
    packet.set_sender_id(configuration_->id());
    // packet.set_sequence_number(?);

    size_t fragment_capacity{rf69_->mtu() - tvsc::radio::Packet::header_size() -
                             tvsc::radio::Packet::payload_size_bytes_required()};
    LOG(INFO) << "CommunicationsServiceImpl::broadcast_shakespeare() -- fragment_capacity: "
              << fragment_capacity;
    size_t line_length{shakespeare.get_next_line(buffer, tvsc::radio::Packet::max_payload_size())};
    Message message{};
    message.set_message(buffer, line_length);
    message.SerializeToArray(packet.payload().data(), tvsc::radio::Packet::max_payload_size());
    packet.set_payload_length(message.ByteSizeLong());
    LOG(INFO) << "CommunicationsServiceImpl::broadcast_shakespeare() -- line_length: "
              << line_length;

    tx_queue_.push_normal(packet);

    tvsc::hal::time::delay_ms(1500);
  }
  LOG(INFO) << "CommunicationsServiceImpl::broadcast_shakespeare() -- Thread finishing.";
}

}  // namespace tvsc::service::communications
