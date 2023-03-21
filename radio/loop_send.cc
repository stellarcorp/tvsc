#include <Arduino.h>
#include <Entropy.h>
#include <SPI.h>

#include <string>

#include "pb_decode.h"
#include "pb_encode.h"
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

void print_id(const tvsc_radio_RadioIdentification& id) {
  Serial.print("{");
  Serial.print(id.expanded_id);
  Serial.print(", ");
  Serial.print(id.id);
  Serial.print(", ");
  Serial.print(id.name);
  Serial.println("}");
}

/**
 * TODO(james): For telemetry, monitor the following:
 *
 * - power
 * - rssi (ambient)
 * - rssi (receiving)
 * - snr: the ratio of the receiving rssi to the ambient rssi.
 * - modulation index: 2 * Fdev / BR
 * - afc frequency error
 * - temperature of radio module
 */

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
  delay(2);
  digitalWrite(RF69_RST, HIGH);
  delay(2);
  digitalWrite(RF69_RST, LOW);
  delay(7);

  if (!rf69.init()) {
    Serial.println("init failed");
    while (true) {
    }
  }

  Serial.print("Board id: ");
  print_id(configuration.identification());
  Serial.println();

  configuration.change_value(tvsc_radio_Function_CARRIER_FREQUENCY_HZ,
                             tvsc::radio::as_discrete_value(433e6f));

  configuration.change_value(tvsc_radio_Function_TX_POWER_DBM,
                             tvsc::radio::as_discrete_value<int8_t>(-2));

  configuration.change_value(tvsc_radio_Function_MODULATION_SCHEME,
                             tvsc::radio::as_discrete_value(tvsc_radio_ModulationTechnique_GFSK));

  configuration.change_value(
      tvsc_radio_Function_LINE_CODING,
      tvsc::radio::as_discrete_value(tvsc_radio_LineCoding_MANCHESTER_ORIGINAL));

  configuration.change_value(tvsc_radio_Function_BIT_RATE,
                             tvsc::radio::as_discrete_value<float>(70000.f));

  configuration.change_value(tvsc_radio_Function_FREQUENCY_DEVIATION,
                             tvsc::radio::as_discrete_value<float>(150000.f));

  configuration.change_value(tvsc_radio_Function_CHANNEL_ACTIVITY_DETECTION_TIMEOUT_MS,
                             tvsc::radio::as_discrete_value<uint32_t>(0));

  configuration.change_value(tvsc_radio_Function_RECEIVE_SENSITIVITY_THRESHOLD_DBM,
                             tvsc::radio::as_discrete_value<float>(-70.f));

  configuration.change_value(
      tvsc_radio_Function_CHANNEL_ACTIVITY_THRESHOLD_DBM,
      // Initialize these thresholds to the same value.
      configuration.get_pending_value(tvsc_radio_Function_RECEIVE_SENSITIVITY_THRESHOLD_DBM));

  configuration.commit_changes();
}

bool send(const std::string& msg) {
  bool result;
  result = rf69.send(reinterpret_cast<const uint8_t*>(msg.data()), msg.length());
  if (result) {
    result = rf69.wait_packet_sent();
  }

  return result;
}

bool recv(std::string& buffer) {
  bool result;
  result = rf69.wait_available_timeout(1000);
  if (result) {
    buffer.clear();
    uint8_t length{rf69.mtu()};

    buffer.resize(length);

    result = rf69.recv(reinterpret_cast<uint8_t*>(buffer.data()), &length);
    if (result) {
      buffer.resize(length);
    }
  }

  return result;
}

template <typename MessageT>
void encode(const MessageT& message, std::string& buffer) {
  buffer.resize(rf69.mtu());
  pb_ostream_t ostream =
      pb_ostream_from_buffer(reinterpret_cast<uint8_t*>(buffer.data()), rf69.mtu());
  bool status = pb_encode(&ostream, nanopb::MessageDescriptor<MessageT>::fields(), &message);
  if (!status) {
    tvsc::except<std::runtime_error>("Could not encode incoming message");
  }
  buffer.resize(ostream.bytes_written);
}

template <typename MessageT>
void decode(const std::string& buffer, MessageT& message) {
  pb_istream_t istream =
      pb_istream_from_buffer(reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size());

  bool status = pb_decode(&istream, nanopb::MessageDescriptor<MessageT>::fields(), &message);
  if (!status) {
    tvsc::except<std::runtime_error>("Could not decode to outgoing message");
  }
}

void loop() {
  std::string id{};
  encode(configuration.identification(), id);

  std::string recv_buffer{};

  if (send(id)) {
    Serial.println("Published id.");
  } else {
    Serial.println("Send failed.");
  }

  delay(10);

  if (recv(recv_buffer)) {
    tvsc_radio_RadioIdentification other_id{};
    decode(recv_buffer, other_id);

    Serial.print("Received id: ");
    print_id(other_id);
    Serial.print("My id:");
    print_id(configuration.identification());
  }

  delay(tvsc::random::generate_random_value<uint8_t>(10, 255));
}
