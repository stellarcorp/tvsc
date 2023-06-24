#include <string>

#include "hal/gpio/pins.h"
#include "hal/output/output.h"
#include "hal/spi/spi.h"
#include "hal/time/time.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "radio/encoding.h"
#include "radio/nanopb_proto/settings.pb.h"
#include "radio/packet.h"
#include "radio/radio_configuration.h"
#include "radio/rf69hcw.h"
#include "radio/rf69hcw_configuration.h"
#include "radio/settings.h"
#include "radio/single_radio_pin_mapping.h"
#include "radio/telemetry_accumulator.h"
#include "radio/utilities.h"
#include "random/random.h"

const uint8_t RF69_RST{tvsc::radio::SingleRadioPinMapping::reset_pin()};
const uint8_t RF69_CS{tvsc::radio::SingleRadioPinMapping::chip_select_pin()};
const uint8_t RF69_DIO0{tvsc::radio::SingleRadioPinMapping::interrupt_pin()};

int main() {
  tvsc::random::initialize_seed();
  tvsc::hal::gpio::initialize_gpio();

  tvsc::hal::spi::SpiBus bus{tvsc::hal::spi::get_default_spi_bus()};
  tvsc::hal::spi::SpiPeripheral spi_peripheral{bus, RF69_CS, 0x80};
  tvsc::radio::RF69HCW rf69{spi_peripheral, RF69_DIO0, RF69_RST};

  tvsc::radio::RadioConfiguration<tvsc::radio::RF69HCW> configuration{
      rf69, tvsc::radio::SingleRadioPinMapping::board_name()};

  tvsc::hal::output::println("Board id: ");
  tvsc::radio::print_id(configuration.identification());
  tvsc::hal::output::println();

  configuration.change_values(tvsc::radio::default_configuration<tvsc::radio::RF69HCW>());
  configuration.commit_changes();

  tvsc::radio::TelemetryAccumulator telemetry{};

  uint32_t previous_sequence_number{};
  uint64_t last_telemetry_report_time{};
  uint16_t next_telemetry_metric_to_report{0};
  uint16_t next_telemetry_sequence_number{0};

  while (true) {
    tvsc::radio::Fragment<tvsc::radio::RF69HCW::max_mtu()> fragment{};
    tvsc::radio::Packet packet{};
    tvsc::radio::EncodedPacket<tvsc::radio::RF69HCW::max_mtu(), 1> fragments{};

    if (tvsc::radio::recv(rf69, fragment)) {
      if (tvsc::radio::decode(fragment, packet)) {
        if (packet.sender_id() != configuration.id()) {
          telemetry.increment_packets_received();

          if (packet.sequence_number() != previous_sequence_number + 1 &&
              previous_sequence_number != 0) {
            telemetry.increment_packets_dropped();
            tvsc::hal::output::print("Dropped packets. packet.sequence_number: ");
            tvsc::hal::output::print(packet.sequence_number());
            tvsc::hal::output::print(", previous_sequence_number: ");
            tvsc::hal::output::print(previous_sequence_number);
            tvsc::hal::output::println();
          }

          previous_sequence_number = packet.sequence_number();

          tvsc::hal::output::print("From sender: ");
          tvsc::hal::output::print(packet.sender_id());
          tvsc::hal::output::print(", sequence: ");
          tvsc::hal::output::print(packet.sequence_number());
          tvsc::hal::output::print(", payload_length: ");
          tvsc::hal::output::println(packet.payload_length());

          // Mark ourselves as the sender now.
          packet.set_sender_id(configuration.id());

          tvsc::radio::encode(packet, fragments);

          if (fragments.num_fragments == 1) {
            // Note that switching into TX mode and sending a packet takes between 50-150ms.
            if (tvsc::radio::send(rf69, fragments.buffers[0])) {
              tvsc::hal::output::println("Packet sent.");
              telemetry.increment_packets_transmitted();
            } else {
              tvsc::hal::output::println("Transmit failed.");
              telemetry.increment_transmit_errors();
            }
          } else if (fragments.num_fragments > 1) {
            tvsc::hal::output::println(
                "Packet required multiple fragments. Dropping. (Echo received packet.)");
          } else {
            tvsc::hal::output::println(
                "Packet required zero fragments. Dropping. (Echo received packet.)");
          }
        }
      }
    }

    if (tvsc::hal::time::time_millis() - last_telemetry_report_time > 1500) {
      last_telemetry_report_time = tvsc::hal::time::time_millis();

      tvsc::hal::output::println("Generating telemetry report");
      const tvsc_radio_nano_TelemetryReport& report{telemetry.generate_telemetry_report()};
      if (report.events_count > 0) {
        if (next_telemetry_metric_to_report >= report.events_count) {
          next_telemetry_metric_to_report = 0;
        }

        const tvsc_radio_nano_TelemetryEvent& event{
            report.events[next_telemetry_metric_to_report++]};
        packet.set_protocol(tvsc::radio::Protocol::TVSC_TELEMETRY);
        packet.set_sender_id(configuration.id());
        packet.set_sequence_number(next_telemetry_sequence_number++);

        pb_ostream_t ostream = pb_ostream_from_buffer(
            reinterpret_cast<uint8_t*>(packet.payload().data()), packet.capacity());
        bool status = pb_encode(
            &ostream, nanopb::MessageDescriptor<tvsc_radio_nano_TelemetryEvent>::fields(), &event);
        if (status) {
          packet.set_payload_length(ostream.bytes_written);

          tvsc::radio::encode(packet, fragments);

          if (fragments.num_fragments == 1) {
            if (tvsc::radio::send(rf69, fragments.buffers[0])) {
              tvsc::hal::output::println("Packet sent.");
              telemetry.increment_packets_transmitted();
            } else {
              tvsc::hal::output::println("Transmit failed.");
              telemetry.increment_transmit_errors();
            }
          } else if (fragments.num_fragments > 1) {
            tvsc::hal::output::println(
                "Packet required multiple fragments. Dropping. (Telemetry.)");
          } else {
            tvsc::hal::output::println("Packet required zero fragments. Dropping. (Telemetry.)");
          }

        } else {
          // Log telemetry encoding issue.
          tvsc::hal::output::println("Could not encode telemetry packet");
        }
      }
    }
  }

  return 0;
}
