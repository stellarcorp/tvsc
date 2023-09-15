#pragma once

#include <string>

#include "comms/radio/nanopb_proto/settings.pb.h"
#include "comms/radio/radio_configuration.h"
#include "comms/radio/radio_utilities.h"
#include "comms/radio/rf69hcw.h"
#include "comms/radio/rf69hcw_configuration.h"
#include "comms/radio/settings.h"
#include "comms/radio/single_radio_pin_mapping.h"
#include "comms/radio/telemetry_accumulator.h"
#include "hal/eeprom/eeprom.h"
#include "hal/gpio/pins.h"
#include "hal/output/output.h"
#include "hal/spi/spi.h"
#include "hal/time/time.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "random/random.h"

namespace tvsc::comms::radio {

template <typename RadioT>
class FragmentTransceiver final {
 private:
  using FragmentT = Fragment<RadioT::max_mtu()>;

  const uint8_t RADIO_RESET_PIN{SingleRadioPinMapping::reset_pin()};
  const uint8_t RADIO_CHIP_SELECT_PIN{SingleRadioPinMapping::chip_select_pin()};
  const uint8_t RADIO_INTERRUPT_PIN{SingleRadioPinMapping::interrupt_pin()};

  tvsc::hal::spi::SpiBus bus_{tvsc::hal::spi::get_default_spi_bus()};
  tvsc::hal::spi::SpiPeripheral spi_peripheral_{bus_, RADIO_CHIP_SELECT_PIN, 0x80};
  RadioT radio_{spi_peripheral_, RADIO_INTERRUPT_PIN, RADIO_RESET_PIN};

  RadioConfiguration<RadioT> configuration_;

  TelemetryAccumulator telemetry_{configuration_.id()};

  // We only support a single fragment waiting to be transmitted.
  FragmentT fragment_{};
  bool have_fragment_to_transmit_{false};

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
    if (current_time - last_telemetry_report_time_ > 1250 && !have_fragment_to_transmit_) {
      last_telemetry_report_time_ = current_time;

      const tvsc_comms_radio_nano_TelemetryReport& report{telemetry_.generate_telemetry_report()};
      if (report.events_count > 0) {
        if (next_telemetry_metric_to_report_ >= report.events_count) {
          next_telemetry_metric_to_report_ = 0;
        }

        const tvsc_comms_radio_nano_TelemetryEvent& event{
            report.events[next_telemetry_metric_to_report_++]};

        pb_ostream_t ostream = pb_ostream_from_buffer(
            reinterpret_cast<uint8_t*>(fragment_.payload_start()), FragmentT::max_payload_size());
        if (pb_encode(&ostream,
                      nanopb::MessageDescriptor<tvsc_comms_radio_nano_TelemetryEvent>::fields(),
                      &event)) {
          fragment_.set_payload_size(ostream.bytes_written);
          fragment_.set_protocol(Protocol::TVSC_TELEMETRY);
          fragment_.set_sender_id(configuration_.id());
          fragment_.set_destination_id(0xff);
          fragment_.set_sequence_number(next_telemetry_sequence_number_++);

          have_fragment_to_transmit_ = true;
          telemetry_.set_transmit_queue_size(1);
        } else {
          // Log telemetry encoding issue.
          tvsc::hal::output::println("Could not encode telemetry fragment");
        }
      }
    }
  }

  void maybe_receive_fragment(uint64_t /*current_time*/) {
    using std::to_string;

    // See if the radio has any fragments to receive.
    if (radio_.has_fragment_available()) {
      radio_.read_received_fragment(fragment_);

      // Check if it is a fragment that we sent. Ignore our own fragments.
      if (fragment_.sender_id() != configuration_.id()) {
        telemetry_.increment_fragments_received();

        if (fragment_.sequence_number() != previous_sequence_number_ + 1 &&
            previous_sequence_number_ != 0) {
          // Detect if we have dropped any fragments/packets.
          telemetry_.increment_fragments_dropped();
          tvsc::hal::output::print("Dropped fragments. fragment.sequence_number: ");
          tvsc::hal::output::print(fragment_.sequence_number());
          tvsc::hal::output::print(", previous_sequence_number: ");
          tvsc::hal::output::print(previous_sequence_number_);
          tvsc::hal::output::println();
        }

        previous_sequence_number_ = fragment_.sequence_number();

        tvsc::hal::output::print("From sender: ");
        tvsc::hal::output::print(fragment_.sender_id());
        tvsc::hal::output::print(", sequence: ");
        tvsc::hal::output::print(fragment_.sequence_number());
        tvsc::hal::output::print(", payload_size: ");
        tvsc::hal::output::println(fragment_.payload_size());

        // Enqueue the same fragment for transmission.
        // Mark ourselves as the sender now.
        fragment_.set_sender_id(configuration_.id());

        have_fragment_to_transmit_ = true;
        telemetry_.set_transmit_queue_size(1);
      } else {
        tvsc::hal::output::println(
            "maybe_receive_fragment() -- Received a fragment from ourselves?");
        tvsc::hal::output::println(to_string(fragment_));
      }
    }
  }

  void maybe_transmit_fragment(uint64_t /*current_time*/) {
    if (have_fragment_to_transmit_) {
      // Note that switching into TX mode and sending a packet takes between 50-150ms.
      if (send(radio_, fragment_)) {
        telemetry_.increment_fragments_transmitted();
        telemetry_.set_transmit_queue_size(0);
        have_fragment_to_transmit_ = false;
      } else {
        tvsc::hal::output::println("Transmit failed.");
        telemetry_.increment_transmit_errors();
      }
    }
  }

 public:
  FragmentTransceiver() : configuration_(radio_, SingleRadioPinMapping::board_name()) {
    configuration_.change_values(default_configuration<RadioT>());
    configuration_.commit_changes();
  }

  FragmentTransceiver(const tvsc_comms_radio_nano_RadioIdentification& identification)
      : configuration_(radio_, identification) {
    configuration_.change_values(default_configuration<RadioT>());
    configuration_.commit_changes();
  }

  void print_configuration() const {
    tvsc::hal::output::println("Board id: ");
    print_id(configuration_.identification());
    tvsc::hal::output::println();
  }

  void iterate() {
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

}  // namespace tvsc::comms::radio
