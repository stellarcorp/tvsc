#include <RH_RF69.h>
#include <SPI.h>

#include <string>

constexpr int RF69_RST{9};
constexpr int RF69_CS{10};

#if defined(RFM69_INTERRUPT_PIN)
constexpr int RF69_DIO0{RFM69_INTERRUPT_PIN};
#else
constexpr int RF69_DIO0{17};
#endif

RH_RF69 rf69{RF69_CS, digitalPinToInterrupt(RF69_DIO0)};

void setup() {
  Serial.begin(9600);
  // Give the serial connection a small amount of time to come up.
  delay(100);

  pinMode(RF69_RST, OUTPUT);
  digitalWrite(RF69_RST, LOW);
  delay(100);

  Serial.println("Teensy RFM69 Client!");
  Serial.println();

  // Manual reset of board.
  digitalWrite(RF69_RST, HIGH);
  delay(10);
  digitalWrite(RF69_RST, LOW);
  delay(10);

  if (!rf69.init()) {
    Serial.println("init failed");
    while (true) {
    }
  }

  if (!rf69.setFrequency(460.0)) Serial.println("setFrequency failed");

  rf69.setTxPower(5, /* ishighpowermodule */ true);

  // Use a very slow encoding/modulation scheme like this so that the signal stands out.
  rf69.setModemConfig(RH_RF69::OOK_Rb1Bw1);
  // Use a faster scheme.
  // rf69.setModemConfig(RH_RF69::OOK_Rb32Bw64);
  // Fast/high bandwidth GFSK modulation scheme.
  // rf69.setModemConfig(RH_RF69::GFSK_Rb250Fd250);
  // Fast/high bandwidth FSK modulation scheme.
  // rf69.setModemConfig(RH_RF69::FSK_Rb250Fd250);
  // Slow FSK modulation scheme.
  // rf69.setModemConfig(RH_RF69::FSK_Rb2_4Fd4_8);
  // rf69.setModemConfig(RH_RF69::FSK_Rb57_6Fd120);

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  // rf69.setEncryptionKey(key);
}

bool send(const std::string& msg) {
  Serial.println("Sending to rf69_server");

  bool result;
  result = rf69.send(msg.data(), msg.length());
  Serial.println("Send requested.");

  if (result) {
    result = rf69.waitPacketSent();
  }

  if (result) {
    Serial.println("Sent.");
  }

  return result;
}

bool recv(std::string& buffer) {
  bool result;
  result = rf69.waitAvailableTimeout(100);
  if (result) {
    buffer.clear();
    uint8_t length{rf69.maxMessageLength()};

    buffer.resize(length + 1);

    result = rf69.recv(buffer.data(), &length);
    if (result) {
      buffer.resize(length);
      Serial.print("got reply: ");
      Serial.println(buffer.c_str());
    } else {
      Serial.println("recv failed");
    }
  } else {
    Serial.println("No reply available.");
  }

  return result;
}

void loop() {
  constexpr char data[] = "Hello World!";
  std::string buffer{};

  send(data);

  if (recv(buffer)) {
    Serial.print("Received: ");
    Serial.println(buffer.c_str());
  }

  delay(200);

  send(data);

  if (recv(buffer)) {
    Serial.print("Received: ");
    Serial.println(buffer.c_str());
  }

  delay(750);
}
