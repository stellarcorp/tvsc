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
  bool result = rf69.recv(reinterpret_cast<uint8_t*>(buffer.data()), &length, 200);
  if (result) {
    buffer.resize(length);
  }
  return result;
}

template <typename MessageT>
bool decode_packet(const std::string& buffer, tvsc_radio_Packet& packet, MessageT& contents) {
  {
    pb_istream_t istream =
        pb_istream_from_buffer(reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size());

    bool status =
        pb_decode(&istream, nanopb::MessageDescriptor<tvsc_radio_Packet>::fields(), &packet);
    if (!status) {
      tvsc::hal::output::println("Could not decode packet");
      return false;
    }
  }

  {
    pb_istream_t istream = pb_istream_from_buffer(
        reinterpret_cast<const uint8_t*>(packet.payload.bytes), packet.payload.size);
    bool status = pb_decode(&istream, nanopb::MessageDescriptor<MessageT>::fields(), &contents);
    if (!status) {
      tvsc::hal::output::println("Could not decode packet contents");
      return false;
    }
  }
  return true;
}

int main() {
  tvsc::random::initialize_seed();
  tvsc::hal::gpio::initialize_gpio();

  tvsc::hal::spi::SpiBus bus{tvsc::hal::spi::get_default_spi_bus()};
  tvsc::hal::spi::SpiPeripheral spi_peripheral{bus, RF69_CS, 0x80};
  tvsc::radio::RF69HCW rf69{};

  tvsc::radio::RadioConfiguration<tvsc::radio::RF69HCW> configuration{
      rf69, tvsc::radio::SingleRadioPinMapping::board_name()};

  tvsc::hal::gpio::set_mode(RF69_RST, tvsc::hal::gpio::PinMode::MODE_OUTPUT);

  // Manual reset of board.
  // To reset, according to the datasheet, the reset pin needs to be high for 100us, then low for
  // 5ms, and then it will be ready. The pin should be pulled low by default on the radio module,
  // but we drive it low first anyway.
  tvsc::hal::gpio::write_pin(RF69_RST, tvsc::hal::gpio::DigitalValue::VALUE_LOW);
  tvsc::hal::time::delay_ms(10);
  tvsc::hal::gpio::write_pin(RF69_RST, tvsc::hal::gpio::DigitalValue::VALUE_HIGH);
  tvsc::hal::time::delay_ms(10);
  tvsc::hal::gpio::write_pin(RF69_RST, tvsc::hal::gpio::DigitalValue::VALUE_LOW);
  tvsc::hal::time::delay_ms(10);

  bus.init();
  spi_peripheral.init();

  if (!rf69.init(spi_peripheral, RF69_DIO0)) {
    tvsc::hal::output::println("init failed");
    while (true) {
    }
  }

  tvsc::hal::output::print("Board id: ");
  print_id(configuration.identification());
  tvsc::hal::output::println();

  configuration.change_values(tvsc::radio::high_throughput_configuration());
  configuration.commit_changes();

  // Start time in milliseconds.
  uint64_t start = tvsc::hal::time::time_millis();

  uint32_t total_packet_count{};
  uint32_t dropped_packet_count{};
  uint32_t previous_sequence_number{};
  uint32_t receive_timeout_count{};
  uint64_t last_print_time{};

  while (true) {
    std::string buffer{};
    buffer.resize(rf69.mtu());

    if (recv(rf69, buffer)) {
      ++total_packet_count;
      tvsc_radio_Packet packet{};
      tvsc_radio_RadioIdentification other_id{};
      if (decode_packet(buffer, packet, other_id)) {
        if (packet.sequence_number != previous_sequence_number + 1 &&
            previous_sequence_number != 0) {
          ++dropped_packet_count;
          tvsc::hal::output::print("Dropped packets. packet.sequence_number: ");
          tvsc::hal::output::print(packet.sequence_number);
          tvsc::hal::output::print(", previous_sequence_number: ");
          tvsc::hal::output::print(previous_sequence_number);
          tvsc::hal::output::println();
        }
      }
      previous_sequence_number = packet.sequence_number;

    } else {
      ++receive_timeout_count;
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

    if (receive_timeout_count > 0 && (receive_timeout_count % 10) == 0) {
      tvsc::hal::output::print("Receive timeout count: ");
      tvsc::hal::output::println(receive_timeout_count);
    }
  }

  return 0;
}
