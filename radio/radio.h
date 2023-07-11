#include <string>

#include "hal/eeprom/eeprom.h"
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
#include "radio/transceiver_utilities.h"
#include "random/random.h"

namespace tvsc::radio {

template <typename RadioT, typename PacketT, size_t MAX_FRAGMENTS_PER_PACKET>
class Radio final {
 private:
  using FragmentT = Fragment<RadioT::max_mtu()>;
  using EncodedPacketsT = EncodedPacket<RadioT::max_mtu(), MAX_FRAGMENTS_PER_PACKET>;

  const uint8_t RADIO_RESET_PIN{SingleRadioPinMapping::reset_pin()};
  const uint8_t RADIO_CHIP_SELECT_PIN{SingleRadioPinMapping::chip_select_pin()};
  const uint8_t RADIO_INTERRUPT_PIN{SingleRadioPinMapping::interrupt_pin()};

  tvsc::hal::spi::SpiBus bus_{tvsc::hal::spi::get_default_spi_bus()};
  tvsc::hal::spi::SpiPeripheral spi_peripheral_{bus_, RADIO_CHIP_SELECT_PIN, 0x80};
  RadioT radio_{spi_peripheral_, RADIO_INTERRUPT_PIN, RADIO_RESET_PIN};

  RadioConfiguration<RadioT> configuration_;

  TelemetryAccumulator telemetry_{configuration_.id()};

  // We only support a single packet waiting to be transmitted. That packet may only contain a
  // single fragment.
  // TODO(james): Relax these constraints by using the PacketTxQueue.
  FragmentT fragment_{};
  PacketT packet_{};
  EncodedPacketsT fragments_{};
  bool have_packet_to_transmit_{false};

  uint32_t previous_sequence_number_{};
  uint16_t next_telemetry_metric_to_report_{0};
  uint16_t next_telemetry_sequence_number_{0};

  uint64_t last_telemetry_report_time_{};
  uint64_t last_rssi_measurement_time_{};

  void maybe_measure_rssi(uint64_t current_time) {
    if (current_time - last_rssi_measurement_time_ > 2000) {
      last_rssi_measurement_time_ = current_time;
      telemetry_.set_rssi_dbm(radio_.read_rssi_dbm());
    }
  }

  void maybe_transmit_telemetry(uint64_t current_time) {
    if (current_time - last_telemetry_report_time_ > 1250 && !have_packet_to_transmit_) {
      last_telemetry_report_time_ = current_time;

      const tvsc_radio_nano_TelemetryReport& report{telemetry_.generate_telemetry_report()};
      if (report.events_count > 0) {
        if (next_telemetry_metric_to_report_ >= report.events_count) {
          next_telemetry_metric_to_report_ = 0;
        }

        const tvsc_radio_nano_TelemetryEvent& event{
            report.events[next_telemetry_metric_to_report_++]};

        pb_ostream_t ostream = pb_ostream_from_buffer(
            reinterpret_cast<uint8_t*>(packet_.payload().data()), packet_.capacity());
        if (pb_encode(&ostream, nanopb::MessageDescriptor<tvsc_radio_nano_TelemetryEvent>::fields(),
                      &event)) {
          packet_.set_payload_length(ostream.bytes_written);
          packet_.set_protocol(Protocol::TVSC_TELEMETRY);
          packet_.set_sender_id(configuration_.id());
          packet_.set_destination_id(0xff);
          packet_.set_sequence_number(next_telemetry_sequence_number_++);

          have_packet_to_transmit_ = true;
          telemetry_.set_transmit_queue_size(1);
        } else {
          // Log telemetry encoding issue.
          tvsc::hal::output::println("Could not encode telemetry packet");
        }
      }
    }
  }

  void maybe_receive_fragment(uint64_t /*current_time*/) {
    using std::to_string;

    // See if the radio has any fragments to receive.
    if (radio_.has_fragment_available()) {
      radio_.read_received_fragment(fragment_);

      // If we have a fragment, check if we can decode it. Fragments that can't be decoded are just
      // ignored.
      if (decode(fragment_, packet_)) {
        // After we decode it, check if it is a fragment that we sent. Ignore our own fragments.
        // TODO(james): Determine if we actually need this check. For a half-duplex transceiver, it
        // is unlikely that we will receive our own transmissions, unless there is a repeater.
        if (packet_.sender_id() != configuration_.id()) {
          telemetry_.increment_packets_received();

          if (packet_.sequence_number() != previous_sequence_number_ + 1 &&
              previous_sequence_number_ != 0) {
            // Detect if we have dropped any fragments/packets.
            telemetry_.increment_packets_dropped();
            tvsc::hal::output::print("Dropped packets. packet.sequence_number: ");
            tvsc::hal::output::print(packet_.sequence_number());
            tvsc::hal::output::print(", previous_sequence_number: ");
            tvsc::hal::output::print(previous_sequence_number_);
            tvsc::hal::output::println();
          }

          previous_sequence_number_ = packet_.sequence_number();

          tvsc::hal::output::print("From sender: ");
          tvsc::hal::output::print(packet_.sender_id());
          tvsc::hal::output::print(", sequence: ");
          tvsc::hal::output::print(packet_.sequence_number());
          tvsc::hal::output::print(", payload_length: ");
          tvsc::hal::output::println(packet_.payload_length());

          // Enqueue the same packet for transmission.
          // Mark ourselves as the sender now.
          packet_.set_sender_id(configuration_.id());

          have_packet_to_transmit_ = true;
          telemetry_.set_transmit_queue_size(1);
        } else {
          tvsc::hal::output::println(
              "maybe_receive_fragment() -- Received a packet from ourselves?");
          tvsc::hal::output::println(to_string(fragment_));
        }
      } else {
        tvsc::hal::output::println("maybe_receive_fragment() -- Couldn't decode packet.");
        tvsc::hal::output::println(to_string(fragment_));
      }
    }
  }

  void maybe_transmit_fragment(uint64_t /*current_time*/) {
    if (have_packet_to_transmit_) {
      if (!encode(packet_, fragments_)) {
        tvsc::hal::output::println("maybe_transmit_fragment() -- Couldn't encode packet.");
        tvsc::hal::output::println(to_string(packet_));
      }

      if (fragments_.num_fragments == 1) {
        // Note that switching into TX mode and sending a packet takes between 50-150ms.
        if (send(radio_, fragments_.buffers[0])) {
          telemetry_.increment_packets_transmitted();
          telemetry_.set_transmit_queue_size(0);
          have_packet_to_transmit_ = false;
        } else {
          tvsc::hal::output::println("Transmit failed.");
          telemetry_.increment_transmit_errors();
        }
      } else if (fragments_.num_fragments > 1) {
        tvsc::hal::output::println(
            "Packet required multiple fragments. Dropping. (Echo received packet.)");
      } else {
        tvsc::hal::output::println(
            "Packet required zero fragments. Dropping. (Echo received packet.)");
      }
    }
  }

 public:
  Radio() : configuration_(radio_, SingleRadioPinMapping::board_name()) {
    configuration_.change_values(default_configuration<RadioT>());
    configuration_.commit_changes();
  }

  Radio(const tvsc_radio_nano_RadioIdentification& identification)
      : configuration_(radio_, identification) {
    configuration_.change_values(default_configuration<RadioT>());
    configuration_.commit_changes();
  }

  void print_configuration() const {
    tvsc::hal::output::println("Board id: ");
    print_id(configuration_.identification());
    tvsc::hal::output::println();
  }

  void process() {
    const uint64_t current_time{tvsc::hal::time::time_millis()};

    radio_.set_receive_mode();

    maybe_receive_fragment(current_time);
    radio_.set_receive_mode();

    maybe_transmit_fragment(current_time);
    radio_.set_receive_mode();

    maybe_measure_rssi(current_time);
    radio_.set_receive_mode();

    maybe_transmit_telemetry(current_time);
    radio_.set_receive_mode();
  }
};

}  // namespace tvsc::radio
