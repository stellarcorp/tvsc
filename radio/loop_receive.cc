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
}

bool recv(std::string& buffer) {
  uint8_t length{buffer.capacity()};
  bool result = rf69.recv(reinterpret_cast<uint8_t*>(buffer.data()), &length, 200);

  if (result) {
    buffer.resize(length);
  } else {
    Serial.println("recv() call failed.");
  }

  return result;
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
  std::string buffer{};
  buffer.resize(rf69.mtu());

  if (recv(buffer)) {
    tvsc_radio_RadioIdentification other_id{};
    decode(buffer, other_id);

    Serial.print("Received id: ");
    print_id(other_id);
  }
}
