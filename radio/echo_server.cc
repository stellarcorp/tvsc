#include <Arduino.h>
#include <Entropy.h>
#include <SPI.h>

#include <string>

#include "pb_decode.h"
#include "pb_encode.h"
#include "radio/packet.pb.h"
#include "radio/radio_configuration.h"
#include "radio/rf69hcw_configuration.h"
#include "radio/settings.h"
#include "radio/settings.pb.h"
#include "random/random.h"

constexpr uint8_t RF69_RST{9};
constexpr uint8_t RF69_CS{10};

#if defined(RFM69_INTERRUPT_PIN)
constexpr uint8_t RF69_DIO0{RFM69_INTERRUPT_PIN};
#else
constexpr uint8_t RF69_DIO0{17};
#endif

SPISettings spi_settings{};
tvsc::radio::RF69HCW rf69{RF69_CS, digitalPinToInterrupt(RF69_DIO0), SPI, spi_settings};

#ifdef TEENSY40
tvsc::radio::RadioConfiguration<tvsc::radio::RF69HCW> configuration{rf69, "RF69HCW 433 Teensy40"};
#elif TEENSY41
tvsc::radio::RadioConfiguration<tvsc::radio::RF69HCW> configuration{rf69, "RF69HCW 433 Teensy41"};
#endif

// Start time in milliseconds.
uint32_t start{};

void print_id(const tvsc_radio_RadioIdentification& id) {
  Serial.print("{");
  Serial.print(id.expanded_id);
  Serial.print(", ");
  Serial.print(id.id);
  Serial.print(", ");
  Serial.print(id.name);
  Serial.println("}");
}

void setup() {
  Serial.begin(9600);

  Entropy.Initialize();
  tvsc::random::set_seed(Entropy.random());
  configuration.regenerate_identifiers();

  pinMode(RF69_RST, OUTPUT);

  // Manual reset of board.
  // To reset, according to the datasheet, the reset pin needs to be high for 100us, then low for
  // 5ms, and then it will be ready. The pin should be pulled low by default on the radio module,
  // but we drive it low first anyway.
  digitalWrite(RF69_RST, LOW);
  delay(10);
  digitalWrite(RF69_RST, HIGH);
  delay(10);
  digitalWrite(RF69_RST, LOW);
  delay(10);

  if (!rf69.init()) {
    Serial.println("init failed");
    while (true) {
    }
  }

  Serial.print("Board id: ");
  print_id(configuration.identification());
  Serial.println();

  configuration.change_values(tvsc::radio::default_configuration<tvsc::radio::RF69HCW>());
  configuration.commit_changes();

  start = millis();
}

bool recv(std::string& buffer) {
  uint8_t length{buffer.capacity()};
  bool result = rf69.recv(reinterpret_cast<uint8_t*>(buffer.data()), &length, 1000);
  if (result) {
    buffer.resize(length);
  }
  return result;
}

bool send(const std::string& msg) {
  bool result;
  result = rf69.send(reinterpret_cast<const uint8_t*>(msg.data()), msg.length());
  if (result) {
    result = rf69.wait_packet_sent();
  }

  return result;
}

bool decode_packet(const std::string& buffer, tvsc_radio_Packet& packet) {
  pb_istream_t istream =
      pb_istream_from_buffer(reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size());

  bool status =
      pb_decode(&istream, nanopb::MessageDescriptor<tvsc_radio_Packet>::fields(), &packet);
  if (!status) {
    Serial.println("Could not decode packet");
    return false;
  }

  return true;
}

void encode_packet(const tvsc_radio_Packet& packet, std::string& buffer) {
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

uint32_t total_packet_count{};
uint32_t dropped_packet_count{};
uint32_t send_success_count{};
uint32_t send_failure_count{};
uint32_t previous_sequence_number{};
uint32_t last_print_time{};

void loop() {
  std::string buffer{};
  buffer.resize(rf69.mtu());

  if (recv(buffer)) {
    tvsc_radio_Packet packet{};
    if (decode_packet(buffer, packet)) {
      if (packet.sender != configuration.id()) {
        ++total_packet_count;

        if (packet.sequence_number != previous_sequence_number + 1 &&
            previous_sequence_number != 0) {
          ++dropped_packet_count;
          Serial.print("Dropped packets. packet.sequence_number: ");
          Serial.print(packet.sequence_number);
          Serial.print(", previous_sequence_number: ");
          Serial.print(previous_sequence_number);
          Serial.println();
        }

        previous_sequence_number = packet.sequence_number;

        Serial.print("From sender: ");
        Serial.print(packet.sender);
        Serial.print(", sequence: ");
        Serial.print(packet.sequence_number);
        Serial.print(" -- ");
        Serial.println(reinterpret_cast<char*>(packet.payload.bytes));

        // Clear previous contents with all zeros.
        buffer.clear();
        buffer.resize(rf69.mtu());

        // Mark ourselves as the sender now.
        packet.sender = configuration.id();

        encode_packet(packet, buffer);
	// Note that switching into TX mode and sending a packet takes between 50-150ms.
        if (send(buffer)) {
          ++send_success_count;
        } else {
          ++send_failure_count;
        }
      }
    }
  }

  // if (millis() - last_print_time > 1000) {
  //   last_print_time = millis();

  //   Serial.print("dropped_packet_count: ");
  //   Serial.print(dropped_packet_count);
  //   Serial.print(", total_packet_count: ");
  //   Serial.print(total_packet_count);
  //   Serial.print(", throughput: ");
  //   Serial.print(total_packet_count * 1000.f / (millis() - start));
  //   Serial.print(" packets/sec");
  //   Serial.println();
  // }
}
