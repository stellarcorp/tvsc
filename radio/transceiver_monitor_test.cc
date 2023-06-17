#include "radio/transceiver_monitor.h"

#include <future>
#include <vector>

#include "gtest/gtest.h"
#include "radio/mock_transceiver.h"
#include "radio/packet.h"
#include "radio/packet_assembler.h"
#include "radio/packet_queue.h"
#include "radio/transceiver.h"

namespace tvsc::radio {

template <size_t MTU>
Fragment<MTU> create_fragment(uint16_t i) {
  EncodedPacket<MTU, 1> fragments{};
  Packet packet{};
  packet.set_sequence_number(i);
  encode(packet, fragments);
  return fragments.buffers[0];
}

class TransceiverMonitorTest : public ::testing::Test {
 public:
  std::vector<Packet> received_packets{};
  void packet_ready(const Packet& packet) { received_packets.emplace_back(packet); }
};

TEST_F(TransceiverMonitorTest, CanReceiveSinglePacket) {
  static constexpr int NUM_PACKETS{1};
  MockTransceiver transceiver{};
  for (int i = 0; i < NUM_PACKETS; ++i) {
    transceiver.add_rx_fragment(create_fragment<MockTransceiver::max_mtu()>(i));
  }

  {
    PacketTxQueue<Packet, 1> tx_queue{};
    PacketAssembler<Packet> rx_queue{};

    TransceiverMonitor<Packet, MockTransceiver::max_mtu(), PacketTxQueue<Packet, 1>::queue_size(),
                       10>
        monitor{transceiver, tx_queue, rx_queue,
                [this](const Packet& packet) { this->packet_ready(packet); }};

    auto result{std::async(std::launch::async, [&monitor] {
      tvsc::hal::time::delay_ms(100);
      DLOG(INFO) << "Stopping transceiver monitor....";
      monitor.cancel();
    })};

    DLOG(INFO) << "Starting mock transceiver interrupts.";
    transceiver.start_interrupts();

    DLOG(INFO) << "Starting transceiver monitor.";

    // This call blocks until the monitor is stopped via the cancel() method.
    monitor.start();

    // As result falls out of scope here, its destructor forces execution of the lambda and causes
    // the monitor to stop.
    DLOG(INFO) << "Destroying transceiver monitor....";
  }
  DLOG(INFO) << "Transceiver monitor stopped and destroyed.";

  DLOG(INFO) << "Stopping mock transceiver interrupts.";
  transceiver.stop_interrupts();

  DLOG(INFO) << "Comparing packets";
  ASSERT_EQ(NUM_PACKETS, received_packets.size());
  for (int i = 0; i < NUM_PACKETS; ++i) {
    DLOG(INFO) << "Comparing packets -- i: " << i;
    EXPECT_EQ(i, received_packets[i].sequence_number());
  }
}

TEST_F(TransceiverMonitorTest, CanReceiveSmallNumberOfPackets) {
  static constexpr int NUM_PACKETS{3};
  MockTransceiver transceiver{};
  for (int i = 0; i < NUM_PACKETS; ++i) {
    transceiver.add_rx_fragment(create_fragment<MockTransceiver::max_mtu()>(i));
  }

  {
    PacketTxQueue<Packet, 1> tx_queue{};
    PacketAssembler<Packet> rx_queue{};

    TransceiverMonitor<Packet, MockTransceiver::max_mtu(), PacketTxQueue<Packet, 1>::queue_size(),
                       10>
        monitor{transceiver, tx_queue, rx_queue,
                [this](const Packet& packet) { this->packet_ready(packet); }};

    auto result{std::async(std::launch::async, [&monitor] {
      tvsc::hal::time::delay_ms(200);
      DLOG(INFO) << "Stopping transceiver monitor....";
      monitor.cancel();
    })};

    DLOG(INFO) << "Starting mock transceiver interrupts.";
    transceiver.start_interrupts();

    DLOG(INFO) << "Starting transceiver monitor.";

    // This call blocks until the monitor is stopped via the cancel() method.
    monitor.start();

    // As result falls out of scope here, its destructor forces execution of the lambda and causes
    // the monitor to stop.
    DLOG(INFO) << "Destroying transceiver monitor....";
  }
  DLOG(INFO) << "Transceiver monitor stopped and destroyed.";

  DLOG(INFO) << "Stopping mock transceiver interrupts.";
  transceiver.stop_interrupts();

  DLOG(INFO) << "Comparing packets";
  ASSERT_EQ(NUM_PACKETS, received_packets.size());
  for (int i = 0; i < NUM_PACKETS; ++i) {
    DLOG(INFO) << "Comparing packets -- i: " << i;
    EXPECT_EQ(i, received_packets[i].sequence_number());
  }
}

TEST_F(TransceiverMonitorTest, CanReceivePackets) {
  static constexpr int NUM_PACKETS{25};
  MockTransceiver transceiver{};
  for (int i = 0; i < NUM_PACKETS; ++i) {
    transceiver.add_rx_fragment(create_fragment<MockTransceiver::max_mtu()>(i));
  }

  {
    PacketTxQueue<Packet, 1> tx_queue{};
    PacketAssembler<Packet> rx_queue{};

    TransceiverMonitor<Packet, MockTransceiver::max_mtu(), PacketTxQueue<Packet, 1>::queue_size(),
                       10>
        monitor{transceiver, tx_queue, rx_queue,
                [this](const Packet& packet) { this->packet_ready(packet); }};

    auto result{std::async(std::launch::async, [&monitor] {
      tvsc::hal::time::delay_ms(500);
      DLOG(INFO) << "Stopping transceiver monitor....";
      monitor.cancel();
    })};

    DLOG(INFO) << "Starting mock transceiver interrupts.";
    transceiver.start_interrupts();

    DLOG(INFO) << "Starting transceiver monitor.";

    // This call blocks until the monitor is stopped via the cancel() method.
    monitor.start();

    // As result falls out of scope here, its destructor forces execution of the lambda and causes
    // the monitor to stop.
    DLOG(INFO) << "Destroying transceiver monitor....";
  }
  DLOG(INFO) << "Transceiver monitor stopped and destroyed.";

  DLOG(INFO) << "Stopping mock transceiver interrupts.";
  transceiver.stop_interrupts();

  DLOG(INFO) << "Comparing packets";
  ASSERT_EQ(NUM_PACKETS, received_packets.size());
  for (int i = 0; i < NUM_PACKETS; ++i) {
    DLOG(INFO) << "Comparing packets -- i: " << i;
    EXPECT_EQ(i, received_packets[i].sequence_number());
  }
}

}  // namespace tvsc::radio
