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

namespace tvsc::radio {

class RadioActivities final {
 private:
  using RadioT = RF69HCW;
  using PacketT = Packet;
  using FragmentT = Fragment<RadioT::max_mtu()>;
  using EncodedPacketsT = EncodedPacket<RadioT::max_mtu(), 1>;

  const uint8_t RF69_RST{SingleRadioPinMapping::reset_pin()};
  const uint8_t RF69_CS{SingleRadioPinMapping::chip_select_pin()};
  const uint8_t RF69_DIO0{SingleRadioPinMapping::interrupt_pin()};

  tvsc::hal::spi::SpiBus bus{tvsc::hal::spi::get_default_spi_bus()};
  tvsc::hal::spi::SpiPeripheral spi_peripheral{bus, RF69_CS, 0x80};
  RadioT rf69{spi_peripheral, RF69_DIO0, RF69_RST};

  RadioConfiguration<RadioT> configuration{rf69, SingleRadioPinMapping::board_name()};

  TelemetryAccumulator telemetry{};

  // We only support a single packet waiting to be transmitted. That packet may only contain a
  // single fragment.
  // TODO(james): Relax these constraints by using the PacketTxQueue.
  FragmentT fragment{};
  PacketT packet{};
  EncodedPacketsT fragments{};
  bool have_packet_to_transmit_{false};

  uint32_t previous_sequence_number{};
  uint16_t next_telemetry_metric_to_report{0};
  uint16_t next_telemetry_sequence_number{0};

  uint64_t last_telemetry_report_time{};
  uint64_t last_rssi_measurement_time{};

  void maybe_measure_rssi(uint64_t current_time) {
    if (current_time - last_rssi_measurement_time > 2000) {
      telemetry.set_rssi_dbm(rf69.read_rssi_dbm());
      last_rssi_measurement_time = current_time;
    }
  }

  void maybe_transmit_telemetry(uint64_t current_time) {
    if (current_time - last_telemetry_report_time > 150 && !have_packet_to_transmit_) {
      last_telemetry_report_time = current_time;

      tvsc::hal::output::println("Generating telemetry report");
      const tvsc_radio_nano_TelemetryReport& report{telemetry.generate_telemetry_report()};
      if (report.events_count > 0) {
        if (next_telemetry_metric_to_report >= report.events_count) {
          next_telemetry_metric_to_report = 0;
        }

        const tvsc_radio_nano_TelemetryEvent& event{
            report.events[next_telemetry_metric_to_report++]};

        pb_ostream_t ostream = pb_ostream_from_buffer(
            reinterpret_cast<uint8_t*>(packet.payload().data()), packet.capacity());
        if (pb_encode(&ostream, nanopb::MessageDescriptor<tvsc_radio_nano_TelemetryEvent>::fields(),
                      &event)) {
          packet.set_payload_length(ostream.bytes_written);
          packet.set_protocol(Protocol::TVSC_TELEMETRY);
          packet.set_sender_id(configuration.id());
          packet.set_sequence_number(next_telemetry_sequence_number++);

          have_packet_to_transmit_ = true;
          telemetry.set_transmit_queue_size(1);
        } else {
          // Log telemetry encoding issue.
          tvsc::hal::output::println("Could not encode telemetry packet");
        }
      }
    }
  }

  void maybe_receive_fragment(uint64_t /*current_time*/) {
    // Clear the fragment buffer.
    fragment.length = 0;

    // See if the radio has any fragments to receive.
    if (recv(rf69, fragment)) {
      // If we have a fragment, check if we can decode it. Fragments that can't be decoded are just
      // ignored.
      if (decode(fragment, packet)) {
        // After we decode it, check if it is a fragment that we sent. Ignore our own fragments.
        // TODO(james): Determine if we actually need this check. For a half-duplex transceiver, it
        // is unlikely that we will receive our own transmissions, unless there is a repeater.
        if (packet.sender_id() != configuration.id()) {
          telemetry.increment_packets_received();

          if (packet.sequence_number() != previous_sequence_number + 1 &&
              previous_sequence_number != 0) {
            // Detect if we have dropped any fragments/packets.
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

          // Enqueue the same packet for transmission.
          // Mark ourselves as the sender now.
          packet.set_sender_id(configuration.id());

          have_packet_to_transmit_ = true;
          telemetry.set_transmit_queue_size(1);
        }
      }
    }
  }

  void maybe_transmit_fragment(uint64_t /*current_time*/) {
    if (have_packet_to_transmit_) {
      encode(packet, fragments);

      if (fragments.num_fragments == 1) {
        // Note that switching into TX mode and sending a packet takes between 50-150ms.
        if (send(rf69, fragments.buffers[0])) {
          tvsc::hal::output::println("Packet sent.");
          telemetry.increment_packets_transmitted();
          telemetry.set_transmit_queue_size(0);
          have_packet_to_transmit_ = false;
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

 public:
  RadioActivities() {
    tvsc::hal::output::println("Board id: ");
    print_id(configuration.identification());
    tvsc::hal::output::println();

    configuration.change_values(default_configuration<RadioT>());
    configuration.commit_changes();
  }

  void iterate() {
    const uint64_t current_time{tvsc::hal::time::time_millis()};

    maybe_receive_fragment(current_time);
    maybe_transmit_fragment(current_time);
    maybe_measure_rssi(current_time);
    maybe_transmit_telemetry(current_time);
  }
};

}  // namespace tvsc::radio

int main() {
  tvsc::random::initialize_seed();
  tvsc::hal::gpio::initialize_gpio();

  tvsc::radio::RadioActivities activities{};
  while (true) {
    activities.iterate();
  }

  return 0;
}
