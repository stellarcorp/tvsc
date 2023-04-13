#include <string>

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
#include "random/random.h"

const uint8_t RF69_RST{tvsc::radio::SingleRadioPinMapping::reset_pin()};
const uint8_t RF69_CS{tvsc::radio::SingleRadioPinMapping::chip_select_pin()};
const uint8_t RF69_DIO0{tvsc::radio::SingleRadioPinMapping::interrupt_pin()};

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
  bool result = rf69.receive_fragment(reinterpret_cast<uint8_t*>(buffer.data()), &length, 1000);
  if (result) {
    buffer.resize(length);
  }
  return result;
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

void encode_packet(tvsc::radio::RF69HCW& rf69, const tvsc_radio_Packet& packet,
                   std::string& buffer) {
  buffer.resize(rf69.mtu());
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

  uint32_t total_packet_count{};
  uint32_t dropped_packet_count{};
  uint32_t send_success_count{};
  uint32_t send_failure_count{};
  uint32_t previous_sequence_number{};
  uint64_t last_print_time{};

  while (true) {
    std::string buffer{};
    buffer.resize(rf69.mtu());

    if (recv(rf69, buffer)) {
      tvsc_radio_Packet packet{};
      if (decode_packet(buffer, packet)) {
        if (packet.sender != configuration.id()) {
          ++total_packet_count;

          if (packet.sequence_number != previous_sequence_number + 1 &&
              previous_sequence_number != 0) {
            ++dropped_packet_count;
            tvsc::hal::output::print("Dropped packets. packet.sequence_number: ");
            tvsc::hal::output::print(packet.sequence_number);
            tvsc::hal::output::print(", previous_sequence_number: ");
            tvsc::hal::output::print(previous_sequence_number);
            tvsc::hal::output::println();
          }

          previous_sequence_number = packet.sequence_number;

          tvsc::hal::output::print("From sender: ");
          tvsc::hal::output::print(packet.sender);
          tvsc::hal::output::print(", sequence: ");
          tvsc::hal::output::print(packet.sequence_number);
          tvsc::hal::output::print(" -- ");
          tvsc::hal::output::println(reinterpret_cast<char*>(packet.payload.bytes));

          // Clear previous contents with all zeros.
          buffer.clear();
          buffer.resize(rf69.mtu());

          // Mark ourselves as the sender now.
          packet.sender = configuration.id();

          encode_packet(rf69, packet, buffer);
          // Note that switching into TX mode and sending a packet takes between 50-150ms.
          if (send(rf69, buffer)) {
            ++send_success_count;
          } else {
            ++send_failure_count;
          }
        }
      }
    }

    if (tvsc::hal::time::time_millis() - last_print_time > 1000) {
      last_print_time = tvsc::hal::time::time_millis();

      tvsc::hal::output::print("dropped_packet_count: ");
      tvsc::hal::output::print(dropped_packet_count);
      tvsc::hal::output::print(", total_packet_count: ");
      tvsc::hal::output::print(total_packet_count);
      tvsc::hal::output::print(", throughput: ");
      tvsc::hal::output::print(total_packet_count * 1000.f /
                               (tvsc::hal::time::time_millis() - start));
      tvsc::hal::output::print(" packets/sec");
      tvsc::hal::output::println();
    }
  }

  return 0;
}
