#include <algorithm>
#include <string>
#include <vector>

#include "hal/gpio/pins.h"
#include "hal/output/output.h"
#include "hal/spi/spi.h"
#include "hal/time/time.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "radio/packet.pb.h"
#include "radio/radio_configuration.h"
#include "radio/rf69hcw_configuration.h"
#include "radio/settings.h"
#include "radio/settings.pb.h"
#include "radio/single_radio_pin_mapping.h"
#include "radio/transceiver.h"
#include "random/random.h"

const uint8_t RF69_RST{tvsc::radio::SingleRadioPinMapping::reset_pin()};
const uint8_t RF69_CS{tvsc::radio::SingleRadioPinMapping::chip_select_pin()};
const uint8_t RF69_DIO0{tvsc::radio::SingleRadioPinMapping::interrupt_pin()};
constexpr uint32_t TX_SIZE_THRESHOLD{16};
constexpr uint32_t TX_TIME_THRESHOLD_MS{50};

struct RadioStatistics final {
  uint32_t packet_rx_count{};
  uint32_t packet_tx_count{};
  uint32_t dropped_packet_count{};
  uint32_t tx_failure_count{};
  uint64_t last_tx_time{};
  uint64_t last_print_time{};
};

void print_id(const tvsc_radio_RadioIdentification& id) {
  tvsc::hal::output::print("{");
  tvsc::hal::output::print(id.expanded_id);
  tvsc::hal::output::print(", ");
  tvsc::hal::output::print(id.id);
  tvsc::hal::output::print(", ");
  tvsc::hal::output::print(id.name);
  tvsc::hal::output::println("}");
}

bool recv(tvsc::radio::RF69HCW& rf69, std::string& buffer) {
  uint8_t length{buffer.capacity()};
  rf69.read_received_fragment(reinterpret_cast<uint8_t*>(buffer.data()), &length);
  buffer.resize(length);
  return length > 0;
}

bool send(tvsc::radio::RF69HCW& rf69, const std::string& msg) {
  bool result;
  result = rf69.transmit_fragment(reinterpret_cast<const uint8_t*>(msg.data()), msg.length(), 250);
  if (result) {
    result = rf69.wait_fragment_transmitted(250);
  }

  return result;
}

bool decode_packet(const std::string& buffer, tvsc_radio_Packet& packet) {
  pb_istream_t istream =
      pb_istream_from_buffer(reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size());

  bool status =
      pb_decode(&istream, nanopb::MessageDescriptor<tvsc_radio_Packet>::fields(), &packet);
  if (!status) {
    tvsc::hal::output::println("Could not decode packet");
    return false;
  }

  return true;
}

void encode_packet(const tvsc_radio_Packet& packet, std::string& buffer) {
  pb_ostream_t ostream =
      pb_ostream_from_buffer(reinterpret_cast<uint8_t*>(buffer.data()), buffer.capacity());
  bool status =
      pb_encode(&ostream, nanopb::MessageDescriptor<tvsc_radio_Packet>::fields(), &packet);
  if (!status) {
    tvsc::except<std::runtime_error>("Could not encode packet for message");
  }
  buffer.resize(ostream.bytes_written);
}

int main() {
  tvsc::random::initialize_seed();
  tvsc::hal::gpio::initialize_gpio();

  tvsc::hal::spi::SpiBus bus{tvsc::hal::spi::get_default_spi_bus()};
  tvsc::hal::spi::SpiPeripheral spi_peripheral{bus, RF69_CS, 0x80};
  tvsc::radio::RF69HCW rf69{spi_peripheral, RF69_DIO0, RF69_RST};

  tvsc::radio::RadioConfiguration<tvsc::radio::RF69HCW> configuration{
      rf69, tvsc::radio::SingleRadioPinMapping::board_name()};

  rf69.reset();

  tvsc::hal::output::println("Board id: ");
  print_id(configuration.identification());
  tvsc::hal::output::println();

  configuration.change_values(tvsc::radio::default_configuration<tvsc::radio::RF69HCW>());
  configuration.commit_changes();

  // Start time in milliseconds.
  uint64_t start = tvsc::hal::time::time_millis();
  uint32_t previous_sequence_number{};
  RadioStatistics statistics{};
  statistics.last_tx_time = start;
  statistics.last_print_time = start;

  std::vector<tvsc_radio_Packet> packet_queue{};
  std::vector<tvsc_radio_Packet> send_queue{packet_queue};

  while (true) {
    rf69.receive();

    std::string buffer{};
    buffer.resize(rf69.mtu());

    if (rf69.has_fragment_available()) {
      if (recv(rf69, buffer)) {
        tvsc_radio_Packet packet{};
        if (decode_packet(buffer, packet)) {
          if (packet.sender != configuration.id()) {
            ++statistics.packet_rx_count;

            if (packet.sequence_number != previous_sequence_number + 1 &&
                previous_sequence_number != 0) {
              ++statistics.dropped_packet_count;
            }
            previous_sequence_number = packet.sequence_number;

            // tvsc::hal::output::print("From sender: ");
            // tvsc::hal::output::print(packet.sender);
            // tvsc::hal::output::print(", sequence: ");
            // tvsc::hal::output::print(packet.sequence_number);
            // tvsc::hal::output::print(" -- ");
            // tvsc::hal::output::println(reinterpret_cast<char*>(packet.payload.bytes));

            packet_queue.emplace_back(std::move(packet));
          }
        }
      }
    }

    if (!packet_queue.empty()) {
      if (packet_queue.size() > TX_SIZE_THRESHOLD ||
          tvsc::hal::time::time_millis() - statistics.last_tx_time > TX_TIME_THRESHOLD_MS) {
        send_queue.swap(packet_queue);
        // We are going to pop packets off of the back, since that is an O(1) operation, but popping
        // off the front is O(n). And while reversing the vector is also O(n), we only have to do it
        // once for a total of O(n). But, if we were to pop off the front, it would be n * O(n) for
        // a total time complexity of O(n^2).
        std::reverse(send_queue.begin(), send_queue.end());
        while (!send_queue.empty()) {
          auto packet = send_queue.back();
          send_queue.pop_back();

          // Clear previous contents with all zeros.
          buffer.clear();
          buffer.resize(rf69.mtu());

          packet.sender = configuration.id();

          encode_packet(packet, buffer);
          if (send(rf69, buffer)) {
            ++statistics.packet_tx_count;
          } else {
            // Any packets that we fail to send, we put back in the queue to try again.
            packet_queue.emplace_back(packet);
            ++statistics.tx_failure_count;
            // Typically, after a single failure, we have a lot more failures. So, just go back to
            // receiving and try again in a moment.
            break;
          }
        }

        // Move any outstanding packets back to the packet queue.
        while (!send_queue.empty()) {
          packet_queue.emplace_back(send_queue.back());
          send_queue.pop_back();
        }

        // Switch back to RX mode ASAP. Helps avoid dropped packets.
        // Switching into TX mode from RX mode, sending a packet, and switching back to
        // RX mode takes 50-150ms, so these TX ops can get expensive.
        rf69.receive();
      }
    }

    if (tvsc::hal::time::time_millis() - statistics.last_print_time > 1000) {
      statistics.last_print_time = tvsc::hal::time::time_millis();

      tvsc::hal::output::print("packet_rx_count: ");
      tvsc::hal::output::print(statistics.packet_rx_count);
      tvsc::hal::output::print(", packet_tx_count: ");
      tvsc::hal::output::print(statistics.packet_tx_count);
      tvsc::hal::output::print(", packet_queue.size(): ");
      tvsc::hal::output::print(packet_queue.size());
      tvsc::hal::output::print(", dropped_packet_count: ");
      tvsc::hal::output::print(statistics.dropped_packet_count);
      tvsc::hal::output::print(", tx_failure_count: ");
      tvsc::hal::output::print(statistics.tx_failure_count);
      tvsc::hal::output::print(", throughput: ");
      tvsc::hal::output::print(statistics.packet_tx_count * 1000.f /
                               (tvsc::hal::time::time_millis() - start));
      tvsc::hal::output::print(" packets/sec");
      tvsc::hal::output::println();
    }
  }

  return 0;
}
