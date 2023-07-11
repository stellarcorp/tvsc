#pragma once

#include <cstdint>
#include <cstring>
#include <numeric>
#include <stdexcept>

#include "base/except.h"
#include "hal/gpio/interrupts.h"
#include "hal/gpio/pins.h"
#include "hal/output/output.h"
#include "hal/spi/spi.h"
#include "hal/time/time.h"
#include "radio/fragment.h"
#include "radio/nanopb_proto/radio.pb.h"
#include "radio/transceiver.h"
#include "radio/yield.h"
#include "random/random.h"

namespace tvsc::radio {

#ifndef ATOMIC_BLOCK_START
#define ATOMIC_BLOCK_START
#define ATOMIC_BLOCK_END
#endif

class RF69HCW final : public HalfDuplexTransceiver</* Hardware MTU. This is the FIFO size of 66
                                                      bytes minus one byte for the message size. */
                                                   65> {
 public:
  enum class OperationalMode {
    STANDBY,
    RX,
    TX,
    SLEEP,
    CHANNEL_ACTIVITY_DETECTION,
  };

  // The crystal oscillator frequency of the RF69 module. This value is used to determine the
  // possible bit rates for the module.
  static constexpr float RF69HCW_FXOSC{32000000.f};

  // The frequency synthesizer step interval. All frequencies are multiples of this interval.
  static constexpr float RF69HCW_FSTEP{RF69HCW_FXOSC / (1 << 19)};

 private:
  // Register names and addresses from the RFM69 datasheet:
  // https://cdn-shop.adafruit.com/product-files/3076/RFM69HCW-V1.1.pdf
  static constexpr uint8_t RF69HCW_REG_00_FIFO{0x00};
  static constexpr uint8_t RF69HCW_REG_01_OPMODE{0x01};
  static constexpr uint8_t RF69HCW_REG_02_DATAMODUL{0x02};
  static constexpr uint8_t RF69HCW_REG_03_BITRATEMSB{0x03};
  static constexpr uint8_t RF69HCW_REG_04_BITRATELSB{0x04};
  static constexpr uint8_t RF69HCW_REG_05_FDEVMSB{0x05};
  static constexpr uint8_t RF69HCW_REG_06_FDEVLSB{0x06};
  static constexpr uint8_t RF69HCW_REG_07_FRFMSB{0x07};
  static constexpr uint8_t RF69HCW_REG_08_FRFMID{0x08};
  static constexpr uint8_t RF69HCW_REG_09_FRFLSB{0x09};
  static constexpr uint8_t RF69HCW_REG_0A_OSC1{0x0a};
  static constexpr uint8_t RF69HCW_REG_0B_AFCCTRL{0x0b};
  static constexpr uint8_t RF69HCW_REG_0C_RESERVED{0x0c};
  static constexpr uint8_t RF69HCW_REG_0D_LISTEN1{0x0d};
  static constexpr uint8_t RF69HCW_REG_0E_LISTEN2{0x0e};
  static constexpr uint8_t RF69HCW_REG_0F_LISTEN3{0x0f};
  static constexpr uint8_t RF69HCW_REG_10_VERSION{0x10};
  static constexpr uint8_t RF69HCW_REG_11_PALEVEL{0x11};
  static constexpr uint8_t RF69HCW_REG_12_PARAMP{0x12};
  static constexpr uint8_t RF69HCW_REG_13_OCP{0x13};
  static constexpr uint8_t RF69HCW_REG_14_RESERVED{0x14};
  static constexpr uint8_t RF69HCW_REG_15_RESERVED{0x15};
  static constexpr uint8_t RF69HCW_REG_16_RESERVED{0x16};
  static constexpr uint8_t RF69HCW_REG_17_RESERVED{0x17};
  static constexpr uint8_t RF69HCW_REG_18_LNA{0x18};
  static constexpr uint8_t RF69HCW_REG_19_RXBW{0x19};
  static constexpr uint8_t RF69HCW_REG_1A_AFCBW{0x1a};
  static constexpr uint8_t RF69HCW_REG_1B_OOKPEAK{0x1b};
  static constexpr uint8_t RF69HCW_REG_1C_OOKAVG{0x1c};
  static constexpr uint8_t RF69HCW_REG_1D_OOKFIX{0x1d};
  static constexpr uint8_t RF69HCW_REG_1E_AFCFEI{0x1e};
  static constexpr uint8_t RF69HCW_REG_1F_AFCMSB{0x1f};
  static constexpr uint8_t RF69HCW_REG_20_AFCLSB{0x20};
  static constexpr uint8_t RF69HCW_REG_21_FEIMSB{0x21};
  static constexpr uint8_t RF69HCW_REG_22_FEILSB{0x22};
  static constexpr uint8_t RF69HCW_REG_23_RSSICONFIG{0x23};
  static constexpr uint8_t RF69HCW_REG_24_RSSIVALUE{0x24};
  static constexpr uint8_t RF69HCW_REG_25_DIOMAPPING1{0x25};
  static constexpr uint8_t RF69HCW_REG_26_DIOMAPPING2{0x26};
  static constexpr uint8_t RF69HCW_REG_27_IRQFLAGS1{0x27};
  static constexpr uint8_t RF69HCW_REG_28_IRQFLAGS2{0x28};
  static constexpr uint8_t RF69HCW_REG_29_RSSITHRESH{0x29};
  static constexpr uint8_t RF69HCW_REG_2A_RXTIMEOUT1{0x2a};
  static constexpr uint8_t RF69HCW_REG_2B_RXTIMEOUT2{0x2b};
  static constexpr uint8_t RF69HCW_REG_2C_PREAMBLEMSB{0x2c};
  static constexpr uint8_t RF69HCW_REG_2D_PREAMBLELSB{0x2d};
  static constexpr uint8_t RF69HCW_REG_2E_SYNCCONFIG{0x2e};
  static constexpr uint8_t RF69HCW_REG_2F_SYNCVALUE1{0x2f};
  static constexpr uint8_t RF69HCW_REG_37_PACKETCONFIG1{0x37};
  static constexpr uint8_t RF69HCW_REG_38_PAYLOADLENGTH{0x38};
  static constexpr uint8_t RF69HCW_REG_39_NODEADRS{0x39};
  static constexpr uint8_t RF69HCW_REG_3A_BROADCASTADRS{0x3a};
  static constexpr uint8_t RF69HCW_REG_3B_AUTOMODES{0x3b};
  static constexpr uint8_t RF69HCW_REG_3C_FIFOTHRESH{0x3c};
  static constexpr uint8_t RF69HCW_REG_3D_PACKETCONFIG2{0x3d};
  static constexpr uint8_t RF69HCW_REG_3E_AESKEY1{0x3e};
  static constexpr uint8_t RF69HCW_REG_4E_TEMP1{0x4e};
  static constexpr uint8_t RF69HCW_REG_4F_TEMP2{0x4f};
  static constexpr uint8_t RF69HCW_REG_58_TESTLNA{0x58};
  static constexpr uint8_t RF69HCW_REG_5A_TESTPA1{0x5a};
  static constexpr uint8_t RF69HCW_REG_5C_TESTPA2{0x5c};
  static constexpr uint8_t RF69HCW_REG_6F_TESTDAGC{0x6f};
  static constexpr uint8_t RF69HCW_REG_71_TESTAFC{0x71};

  // Names of select register functions and their location within the register.
  // The names of these functions are taken from the datasheet.

  // RF69HCW_REG_01_OPMODE
  static constexpr uint8_t RF69HCW_OPMODE_SEQUENCEROFF{0x80};
  static constexpr uint8_t RF69HCW_OPMODE_LISTENON{0x40};
  static constexpr uint8_t RF69HCW_OPMODE_LISTENABORT{0x20};
  static constexpr uint8_t RF69HCW_OPMODE_MODE{0x1c};
  static constexpr uint8_t RF69HCW_OPMODE_MODE_SLEEP{0x00};
  static constexpr uint8_t RF69HCW_OPMODE_MODE_STDBY{0x04};
  static constexpr uint8_t RF69HCW_OPMODE_MODE_FS{0x08};
  static constexpr uint8_t RF69HCW_OPMODE_MODE_TX{0x0c};
  static constexpr uint8_t RF69HCW_OPMODE_MODE_RX{0x10};

  // RF69HCW_REG_02_DATAMODUL
  static constexpr uint8_t RF69HCW_DATAMODUL_DATAMODE{0x60};
  static constexpr uint8_t RF69HCW_DATAMODUL_DATAMODE_PACKET{0x00};
  static constexpr uint8_t RF69HCW_DATAMODUL_DATAMODE_CONT_WITH_SYNC{0x40};
  static constexpr uint8_t RF69HCW_DATAMODUL_DATAMODE_CONT_WITHOUT_SYNC{0x60};
  static constexpr uint8_t RF69HCW_DATAMODUL_MODULATIONTYPE{0x18};
  static constexpr uint8_t RF69HCW_DATAMODUL_MODULATIONTYPE_FSK{0x00};
  static constexpr uint8_t RF69HCW_DATAMODUL_MODULATIONTYPE_OOK{0x08};
  static constexpr uint8_t RF69HCW_DATAMODUL_MODULATIONSHAPING{0x03};
  static constexpr uint8_t RF69HCW_DATAMODUL_MODULATIONSHAPING_FSK_NONE{0x00};
  static constexpr uint8_t RF69HCW_DATAMODUL_MODULATIONSHAPING_FSK_BT1_0{0x01};
  static constexpr uint8_t RF69HCW_DATAMODUL_MODULATIONSHAPING_FSK_BT0_5{0x02};
  static constexpr uint8_t RF69HCW_DATAMODUL_MODULATIONSHAPING_FSK_BT0_3{0x03};
  static constexpr uint8_t RF69HCW_DATAMODUL_MODULATIONSHAPING_OOK_NONE{0x00};
  static constexpr uint8_t RF69HCW_DATAMODUL_MODULATIONSHAPING_OOK_BR{0x01};
  static constexpr uint8_t RF69HCW_DATAMODUL_MODULATIONSHAPING_OOK_2BR{0x02};

  // RF69HCW_REG_11_PALEVEL
  static constexpr uint8_t RF69HCW_PALEVEL_PA0ON{0x80};
  static constexpr uint8_t RF69HCW_PALEVEL_PA1ON{0x40};
  static constexpr uint8_t RF69HCW_PALEVEL_PA2ON{0x20};
  static constexpr uint8_t RF69HCW_PALEVEL_OUTPUTPOWER{0x1f};

  // RF69HCW_REG_23_RSSICONFIG
  static constexpr uint8_t RF69HCW_RSSICONFIG_RSSIDONE{0x02};
  static constexpr uint8_t RF69HCW_RSSICONFIG_RSSISTART{0x01};

  // RF69HCW_REG_25_DIOMAPPING1
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO0MAPPING{0xc0};
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO0MAPPING_00{0x00};
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO0MAPPING_01{0x40};
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO0MAPPING_10{0x80};
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO0MAPPING_11{0xc0};

  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO1MAPPING{0x30};
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO1MAPPING_00{0x00};
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO1MAPPING_01{0x10};
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO1MAPPING_10{0x20};
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO1MAPPING_11{0x30};

  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO2MAPPING{0x0c};
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO2MAPPING_00{0x00};
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO2MAPPING_01{0x04};
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO2MAPPING_10{0x08};
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO2MAPPING_11{0x0c};

  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO3MAPPING{0x03};
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO3MAPPING_00{0x00};
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO3MAPPING_01{0x01};
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO3MAPPING_10{0x02};
  static constexpr uint8_t RF69HCW_DIOMAPPING1_DIO3MAPPING_11{0x03};

  // RF69HCW_REG_26_DIOMAPPING2
  static constexpr uint8_t RF69HCW_DIOMAPPING2_DIO4MAPPING{0xc0};
  static constexpr uint8_t RF69HCW_DIOMAPPING2_DIO4MAPPING_00{0x00};
  static constexpr uint8_t RF69HCW_DIOMAPPING2_DIO4MAPPING_01{0x40};
  static constexpr uint8_t RF69HCW_DIOMAPPING2_DIO4MAPPING_10{0x80};
  static constexpr uint8_t RF69HCW_DIOMAPPING2_DIO4MAPPING_11{0xc0};

  static constexpr uint8_t RF69HCW_DIOMAPPING2_DIO5MAPPING{0x30};
  static constexpr uint8_t RF69HCW_DIOMAPPING2_DIO5MAPPING_00{0x00};
  static constexpr uint8_t RF69HCW_DIOMAPPING2_DIO5MAPPING_01{0x10};
  static constexpr uint8_t RF69HCW_DIOMAPPING2_DIO5MAPPING_10{0x20};
  static constexpr uint8_t RF69HCW_DIOMAPPING2_DIO5MAPPING_11{0x30};

  static constexpr uint8_t RF69HCW_DIOMAPPING2_CLKOUT{0x07};
  static constexpr uint8_t RF69HCW_DIOMAPPING2_CLKOUT_FXOSC_{0x00};
  static constexpr uint8_t RF69HCW_DIOMAPPING2_CLKOUT_FXOSC_2{0x01};
  static constexpr uint8_t RF69HCW_DIOMAPPING2_CLKOUT_FXOSC_4{0x02};
  static constexpr uint8_t RF69HCW_DIOMAPPING2_CLKOUT_FXOSC_8{0x03};
  static constexpr uint8_t RF69HCW_DIOMAPPING2_CLKOUT_FXOSC_16{0x04};
  static constexpr uint8_t RF69HCW_DIOMAPPING2_CLKOUT_FXOSC_32{0x05};
  static constexpr uint8_t RF69HCW_DIOMAPPING2_CLKOUT_FXOSC_RC{0x06};
  static constexpr uint8_t RF69HCW_DIOMAPPING2_CLKOUT_FXOSC_OFF{0x07};

  // RF69HCW_REG_27_IRQFLAGS1
  static constexpr uint8_t RF69HCW_IRQFLAGS1_MODEREADY{0x80};
  static constexpr uint8_t RF69HCW_IRQFLAGS1_RXREADY{0x40};
  static constexpr uint8_t RF69HCW_IRQFLAGS1_TXREADY{0x20};
  static constexpr uint8_t RF69HCW_IRQFLAGS1_PLLLOCK{0x10};
  static constexpr uint8_t RF69HCW_IRQFLAGS1_RSSI{0x08};
  static constexpr uint8_t RF69HCW_IRQFLAGS1_TIMEOUT{0x04};
  static constexpr uint8_t RF69HCW_IRQFLAGS1_AUTOMODE{0x02};
  static constexpr uint8_t RF69HCW_IRQFLAGS1_SYNADDRESSMATCH{0x01};

  // RF69HCW_REG_28_IRQFLAGS2
  static constexpr uint8_t RF69HCW_IRQFLAGS2_FIFOFULL{0x80};
  static constexpr uint8_t RF69HCW_IRQFLAGS2_FIFONOTEMPTY{0x40};
  static constexpr uint8_t RF69HCW_IRQFLAGS2_FIFOLEVEL{0x20};
  static constexpr uint8_t RF69HCW_IRQFLAGS2_FIFOOVERRUN{0x10};
  static constexpr uint8_t RF69HCW_IRQFLAGS2_PACKETSENT{0x08};
  static constexpr uint8_t RF69HCW_IRQFLAGS2_PAYLOADREADY{0x04};
  static constexpr uint8_t RF69HCW_IRQFLAGS2_CRCOK{0x02};

  // RF69HCW_REG_2E_SYNCCONFIG
  static constexpr uint8_t RF69HCW_SYNCCONFIG_SYNCON{0x80};
  static constexpr uint8_t RF69HCW_SYNCCONFIG_FIFOFILLCONDITION_MANUAL{0x40};
  static constexpr uint8_t RF69HCW_SYNCCONFIG_SYNCSIZE{0x38};
  static constexpr uint8_t RF69HCW_SYNCCONFIG_SYNCSIZE_1{0x00};
  static constexpr uint8_t RF69HCW_SYNCCONFIG_SYNCSIZE_2{0x08};
  static constexpr uint8_t RF69HCW_SYNCCONFIG_SYNCSIZE_3{0x10};
  static constexpr uint8_t RF69HCW_SYNCCONFIG_SYNCSIZE_4{0x18};
  static constexpr uint8_t RF69HCW_SYNCCONFIG_SYNCSIZE_5{0x20};
  static constexpr uint8_t RF69HCW_SYNCCONFIG_SYNCSIZE_6{0x28};
  static constexpr uint8_t RF69HCW_SYNCCONFIG_SYNCSIZE_7{0x30};
  static constexpr uint8_t RF69HCW_SYNCCONFIG_SYNCSIZE_8{0x38};
  static constexpr uint8_t RF69HCW_SYNCCONFIG_SYNCSIZE_SYNCTOL{0x07};

  // RF69HCW_REG_37_PACKETCONFIG1
  static constexpr uint8_t RF69HCW_PACKETCONFIG1_PACKETFORMAT_VARIABLE{0x80};
  static constexpr uint8_t RF69HCW_PACKETCONFIG1_DCFREE{0x60};
  static constexpr uint8_t RF69HCW_PACKETCONFIG1_DCFREE_NONE{0x00};
  static constexpr uint8_t RF69HCW_PACKETCONFIG1_DCFREE_MANCHESTER{0x20};
  static constexpr uint8_t RF69HCW_PACKETCONFIG1_DCFREE_WHITENING{0x40};
  static constexpr uint8_t RF69HCW_PACKETCONFIG1_DCFREE_RESERVED{0x60};
  static constexpr uint8_t RF69HCW_PACKETCONFIG1_CRC_ON{0x10};
  static constexpr uint8_t RF69HCW_PACKETCONFIG1_CRCAUTOCLEAROFF{0x08};
  static constexpr uint8_t RF69HCW_PACKETCONFIG1_ADDRESSFILTERING{0x06};
  static constexpr uint8_t RF69HCW_PACKETCONFIG1_ADDRESSFILTERING_NONE{0x00};
  static constexpr uint8_t RF69HCW_PACKETCONFIG1_ADDRESSFILTERING_NODE{0x02};
  static constexpr uint8_t RF69HCW_PACKETCONFIG1_ADDRESSFILTERING_NODE_BC{0x04};
  static constexpr uint8_t RF69HCW_PACKETCONFIG1_ADDRESSFILTERING_RESERVED{0x06};

  // RF69HCW_REG_3B_AUTOMODES
  static constexpr uint8_t RF69HCW_AUTOMODE_ENTER_COND_NONE{0x00};
  static constexpr uint8_t RF69HCW_AUTOMODE_ENTER_COND_FIFO_NOT_EMPTY{0x20};
  static constexpr uint8_t RF69HCW_AUTOMODE_ENTER_COND_FIFO_LEVEL{0x40};
  static constexpr uint8_t RF69HCW_AUTOMODE_ENTER_COND_CRC_OK{0x60};
  static constexpr uint8_t RF69HCW_AUTOMODE_ENTER_COND_PAYLOAD_READY{0x80};
  static constexpr uint8_t RF69HCW_AUTOMODE_ENTER_COND_SYNC_ADDRESS{0xa0};
  static constexpr uint8_t RF69HCW_AUTOMODE_ENTER_COND_PACKET_SENT{0xc0};
  static constexpr uint8_t RF69HCW_AUTOMODE_ENTER_COND_FIFO_EMPTY{0xe0};

  static constexpr uint8_t RF69HCW_AUTOMODE_EXIT_COND_NONE{0x00};
  static constexpr uint8_t RF69HCW_AUTOMODE_EXIT_COND_FIFO_EMPTY{0x04};
  static constexpr uint8_t RF69HCW_AUTOMODE_EXIT_COND_FIFO_LEVEL{0x08};
  static constexpr uint8_t RF69HCW_AUTOMODE_EXIT_COND_CRC_OK{0x0c};
  static constexpr uint8_t RF69HCW_AUTOMODE_EXIT_COND_PAYLOAD_READY{0x10};
  static constexpr uint8_t RF69HCW_AUTOMODE_EXIT_COND_SYNC_ADDRESS{0x14};
  static constexpr uint8_t RF69HCW_AUTOMODE_EXIT_COND_PACKET_SENT{0x18};
  static constexpr uint8_t RF69HCW_AUTOMODE_EXIT_COND_TIMEOUT{0x1c};

  static constexpr uint8_t RF69HCW_AUTOMODE_INTERMEDIATE_MODE_SLEEP{0x00};
  static constexpr uint8_t RF69HCW_AUTOMODE_INTERMEDIATE_MODE_STDBY{0x01};
  static constexpr uint8_t RF69HCW_AUTOMODE_INTERMEDIATE_MODE_RX{0x02};
  static constexpr uint8_t RF69HCW_AUTOMODE_INTERMEDIATE_MODE_TX{0x03};

  // RF69HCW_REG_3C_FIFOTHRESH
  static constexpr uint8_t RF69HCW_FIFOTHRESH_TXSTARTCONDITION_NOTEMPTY{0x80};
  static constexpr uint8_t RF69HCW_FIFOTHRESH_FIFOTHRESHOLD{0x7f};

  // RF69HCW_REG_3D_PACKETCONFIG2
  static constexpr uint8_t RF69HCW_PACKETCONFIG2_INTERPACKETRXDELAY{0xf0};
  static constexpr uint8_t RF69HCW_PACKETCONFIG2_RESTARTRX{0x04};
  static constexpr uint8_t RF69HCW_PACKETCONFIG2_AUTORXRESTARTON{0x02};
  static constexpr uint8_t RF69HCW_PACKETCONFIG2_AESON{0x01};

  // RF69HCW_REG_4E_TEMP1
  static constexpr uint8_t RF69HCW_TEMP1_TEMPMEASSTART{0x08};
  static constexpr uint8_t RF69HCW_TEMP1_TEMPMEASRUNNING{0x04};

  // RF69HCW_REG_5A_TESTPA1
  static constexpr uint8_t RF69HCW_TESTPA1_NORMAL{0x55};
  static constexpr uint8_t RF69HCW_TESTPA1_BOOST{0x5d};

  // RF69HCW_REG_5C_TESTPA2
  static constexpr uint8_t RF69HCW_TESTPA2_NORMAL{0x70};
  static constexpr uint8_t RF69HCW_TESTPA2_BOOST{0x7c};

  // RF69HCW_REG_6F_TESTDAGC
  static constexpr uint8_t RF69HCW_TESTDAGC_CONTINUOUSDAGC_NORMAL{0x00};
  static constexpr uint8_t RF69HCW_TESTDAGC_CONTINUOUSDAGC_IMPROVED_LOWBETAON{0x20};
  static constexpr uint8_t RF69HCW_TESTDAGC_CONTINUOUSDAGC_IMPROVED_LOWBETAOFF{0x30};

  static constexpr uint8_t SYNC_WORDS[] = "SR90tvsc";

  // static constexpr uint8_t RX_BUFFER_LENGTH{max_mtu()};

  tvsc::hal::spi::SpiPeripheral* spi_;

  Fragment<MAX_MTU_VALUE> rx_buffer_{};
  bool rx_buffer_valid_{false};

  int8_t power_;
  OperationalMode op_mode_{OperationalMode::STANDBY};

  float channel_activity_threshold_dbm_{-.5f};

  // Pin assignments.
  uint8_t interrupt_pin_;
  uint8_t reset_pin_;

  void print_interrupt_registers() {
    uint8_t irq_flags1 = spi_->read(RF69HCW_REG_27_IRQFLAGS1);
    uint8_t irq_flags2 = spi_->read(RF69HCW_REG_28_IRQFLAGS2);
    tvsc::hal::output::print("RF69HCW::print_interrupt_register() -- irq_flags1: ");
    tvsc::hal::output::println(static_cast<uint32_t>(irq_flags1));
    tvsc::hal::output::print("RF69HCW::print_interrupt_register() -- irq_flags2: ");
    tvsc::hal::output::println(static_cast<uint32_t>(irq_flags2));
  }

  void set_op_mode(uint8_t mode) {
    uint8_t opmode = spi_->read(RF69HCW_REG_01_OPMODE);

    opmode &= ~RF69HCW_OPMODE_MODE;
    opmode |= (mode & RF69HCW_OPMODE_MODE);
    spi_->write(RF69HCW_REG_01_OPMODE, opmode);

    // Block until the RF module is ready.
    uint8_t irq_flags;
    bool in_mode{false};
    do {
      // Note: this works, but it feels weird polling on an IRQ register.
      // Also, adding a YIELD here, instead of just looping as fast as possible, causes spurious
      // failures.
      irq_flags = spi_->read(RF69HCW_REG_27_IRQFLAGS1);
      // if (mode == RF69HCW_OPMODE_MODE_RX) {
      //   in_mode = ((irq_flags & RF69HCW_IRQFLAGS1_MODEREADY) != 0) &&
      //             ((irq_flags & RF69HCW_IRQFLAGS1_RXREADY) != 0);
      // } else if (mode == RF69HCW_OPMODE_MODE_TX) {
      //   in_mode = ((irq_flags & RF69HCW_IRQFLAGS1_MODEREADY) != 0) &&
      //             ((irq_flags & RF69HCW_IRQFLAGS1_TXREADY) != 0);
      // } else {
      in_mode = ((irq_flags & RF69HCW_IRQFLAGS1_MODEREADY) != 0);
      // }
    } while (!in_mode);
  }

  /**
   * Read the received data from the FIFO into the RX buffer in this class.
   */
  void read_fifo() {
    const size_t bytes_read =
        spi_->fifo_read(RF69HCW_REG_00_FIFO, rx_buffer_.data.data(), max_mtu());
    if (bytes_read == rx_buffer_.total_length()) {
      rx_buffer_valid_ = true;
    } else {
      rx_buffer_valid_ = false;
    }
  }

  // Interrupt vectoring.
  static void isr0();
  static void isr1();
  static void isr2();

  static RF69HCW* interrupt_devices_[3];

  // Next interrupt index to use as an interrupt vector. Valid values are [0,1,2]. If it is greater
  // than 2, no more interrupts can be established.
  static uint8_t next_interrupt_index_;

  void handle_interrupt() {
    uint8_t irq_flags2 = spi_->read(RF69HCW_REG_28_IRQFLAGS2);

    // print_interrupt_registers();

    if (op_mode_ == OperationalMode::TX && ((irq_flags2 & RF69HCW_IRQFLAGS2_PACKETSENT) != 0)) {
      // A message has been fully transmitted.
      // Clear the FIFO and move the operational mode away from TX.
      // Note that wait_fragment_transmitted() and derivatives use the operational mode to know if
      // the packet has been sent.
      set_mode_standby();
    }

    // The datasheet indicates that CRCOK would be the appropriate flag to check for, but it gets
    // set before decryption. Instead, we check for PAYLOADREADY which gets set after decryption.
    // This check also guarantees that we have a valid CRC.
    if (op_mode_ == OperationalMode::RX && ((irq_flags2 & RF69HCW_IRQFLAGS2_PAYLOADREADY) != 0)) {
      set_mode_standby();

      // Transfer the data in the FIFO to our buffer.
      read_fifo();
    }
  }

  void set_mode_rx() {
    if (op_mode_ != OperationalMode::RX) {
      if (power_ >= 18) {
        // If we are using the high power boost, we must turn it off to receive.
        spi_->write(RF69HCW_REG_5A_TESTPA1, RF69HCW_TESTPA1_NORMAL);
        spi_->write(RF69HCW_REG_5C_TESTPA2, RF69HCW_TESTPA2_NORMAL);
      }
      spi_->write(RF69HCW_REG_25_DIOMAPPING1, RF69HCW_DIOMAPPING1_DIO0MAPPING_01);
      set_op_mode(RF69HCW_OPMODE_MODE_RX);
      op_mode_ = OperationalMode::RX;
    }
  }

  void set_mode_standby() {
    if (op_mode_ != OperationalMode::STANDBY) {
      if (power_ >= 18) {
        // If we are using the high power boost, we must turn it off to receive.
        // It's unclear if we need to turn it off to enter standby mode, but since we are likely
        // entering this mode to conserve power, we turn it off here.
        // TODO(james): Determine how the high power boost is expected to interact with the
        // different operational modes.
        spi_->write(RF69HCW_REG_5A_TESTPA1, RF69HCW_TESTPA1_NORMAL);
        spi_->write(RF69HCW_REG_5C_TESTPA2, RF69HCW_TESTPA2_NORMAL);
      }
      set_op_mode(RF69HCW_OPMODE_MODE_STDBY);
      op_mode_ = OperationalMode::STANDBY;
    }
  }

  void set_mode_tx() {
    if (op_mode_ != OperationalMode::TX) {
      if (power_ >= 18) {
        // Turn on the high power boost.
        // TODO(james): Determine if we need to turn off over current protection (OCP) to activate
        // high power boost. The datasheet suggests so (page 21), but we seem to get high power
        // boost with OCP on.
        spi_->write(RF69HCW_REG_5A_TESTPA1, RF69HCW_TESTPA1_BOOST);
        spi_->write(RF69HCW_REG_5C_TESTPA2, RF69HCW_TESTPA2_BOOST);
      }
      spi_->write(RF69HCW_REG_25_DIOMAPPING1, RF69HCW_DIOMAPPING1_DIO0MAPPING_00);
      set_op_mode(RF69HCW_OPMODE_MODE_TX);
      op_mode_ = OperationalMode::TX;
    }
  }

 public:
  RF69HCW(tvsc::hal::spi::SpiPeripheral& peripheral, uint8_t interrupt_pin, uint8_t reset_pin)
      : spi_(&peripheral), interrupt_pin_(interrupt_pin), reset_pin_(reset_pin) {
    tvsc::hal::gpio::set_mode(reset_pin_, tvsc::hal::gpio::PinMode::MODE_OUTPUT);

    void (*interrupt_fn)(void) = nullptr;
    if (next_interrupt_index_ <= 2) {
      interrupt_devices_[next_interrupt_index_] = this;
      if (next_interrupt_index_ == 0) {
        interrupt_fn = isr0;
      } else if (next_interrupt_index_ == 1) {
        interrupt_fn = isr1;
      } else if (next_interrupt_index_ == 2) {
        interrupt_fn = isr2;
      }
      ++next_interrupt_index_;
    } else {
      except<std::domain_error>(
          "No interrupts available. Too many devices or instantiations of the RF69HCW class.");
    }

    tvsc::hal::gpio::set_mode(interrupt_pin_, tvsc::hal::gpio::PinMode::MODE_INPUT);
    spi_->bus().using_interrupt(interrupt_pin_);
    tvsc::hal::gpio::attach_interrupt(interrupt_pin_, interrupt_fn);

    // Put the radio module into its default state.
    reset();
  }

  void reset() override {
    // Manual reset of board.
    // To reset, according to the datasheet, the reset pin needs to be high for 100us, then low for
    // 5ms, and then it will be ready. The pin should be pulled low by default on the radio module,
    // but we drive it low first anyway.
    tvsc::hal::gpio::write_pin(reset_pin_, tvsc::hal::gpio::DigitalValue::VALUE_LOW);
    tvsc::hal::time::delay_ms(10);
    tvsc::hal::gpio::write_pin(reset_pin_, tvsc::hal::gpio::DigitalValue::VALUE_HIGH);
    tvsc::hal::time::delay_ms(10);
    tvsc::hal::gpio::write_pin(reset_pin_, tvsc::hal::gpio::DigitalValue::VALUE_LOW);
    tvsc::hal::time::delay_ms(10);

    // Verify that we are actually connected to a device. We expect this will return 0x00 or 0xff
    // only if the device is not connected correctly.
    // TODO(james): Include this information in telemetry via a periodic check (not a cached value)
    // and in device identification. This check will provide a good way to determine if a wire has
    // come loose.
    uint8_t device_type{};
    do {
      tvsc::hal::time::delay_ms(1000);
      device_type = spi_->read(RF69HCW_REG_10_VERSION);
      // except<std::runtime_error>("Could not read radio device type.");
    } while (device_type == 00 || device_type == 0xff);

    tvsc::hal::output::print("Device type: ");
    tvsc::hal::output::println(device_type);

    // Clear out the opmode register to remove any spurious settings and then switch to standby
    // mode.
    // spi_->write(RF69HCW_REG_01_OPMODE, 0x00);
    // tvsc::hal::time::delay_ms(10);
    set_mode_standby();

    // Specify time to ramp up and down the amplifiers. Note that the PARAMP must match the
    // INTERPACKETRXDELAY.
    spi_->write(RF69HCW_REG_12_PARAMP, 0x09);
    spi_->write(RF69HCW_REG_3D_PACKETCONFIG2,
                ((0x09 << 4) & RF69HCW_PACKETCONFIG2_INTERPACKETRXDELAY));

    // Reset the power amplifiers.
    spi_->write(RF69HCW_REG_5A_TESTPA1, RF69HCW_TESTPA1_NORMAL);
    spi_->write(RF69HCW_REG_5C_TESTPA2, RF69HCW_TESTPA2_NORMAL);

    spi_->write(RF69HCW_REG_3C_FIFOTHRESH, RF69HCW_FIFOTHRESH_TXSTARTCONDITION_NOTEMPTY | 0x02);

    spi_->write(RF69HCW_REG_6F_TESTDAGC, RF69HCW_TESTDAGC_CONTINUOUSDAGC_IMPROVED_LOWBETAOFF);

    spi_->write(RF69HCW_REG_37_PACKETCONFIG1, RF69HCW_PACKETCONFIG1_PACKETFORMAT_VARIABLE |
                                                  RF69HCW_PACKETCONFIG1_CRC_ON |
                                                  RF69HCW_PACKETCONFIG1_DCFREE_WHITENING |
                                                  RF69HCW_PACKETCONFIG1_ADDRESSFILTERING_NONE);

    set_sync_words_length(8);

    // Reset to +13dBm, same as power-on default.
    set_power_dbm(13);
  }

  float read_rssi_dbm() override {
    // From page 28 of the datasheet:
    // "RssiValue can only be read when it exceeds RssiThreshold"
    // We cache the current value of RssiThreshold, set the threshold to the minimum value, make our
    // measurement, and then reset the threshold.
    const float saved_threshold{get_receive_sensitivity_threshold_dbm()};

    // Set the threshold to its minimum value.
    set_receive_sensitivity_threshold_dbm(-127.5f);

    static constexpr int NUMBER_MEASUREMENTS{4};
    static constexpr int DELAY_BETWEEN_MEASUREMENTS_US{5};
    std::array<uint8_t, NUMBER_MEASUREMENTS> rssi{};
    for (int i = 0; i < NUMBER_MEASUREMENTS; ++i) {
      if (i > 0) {
        tvsc::hal::time::delay_us(DELAY_BETWEEN_MEASUREMENTS_US);
      }
      rssi[i] = spi_->read(RF69HCW_REG_24_RSSIVALUE);
    }

    // Reset the threshold to its previous value.
    set_receive_sensitivity_threshold_dbm(saved_threshold);

    // Return the average RSSI measurement converted to dBm.
    float result = -1.f * std::accumulate(rssi.begin(), rssi.end(), 0) / 2.f / NUMBER_MEASUREMENTS;
    return result;
  }

  void set_receive_mode() override { set_mode_rx(); }
  void set_standby_mode() override { set_mode_standby(); }

  bool in_standby_mode() const override { return op_mode_ == OperationalMode::STANDBY; }
  bool in_rx_mode() const override { return op_mode_ == OperationalMode::RX; }
  bool in_tx_mode() const override { return op_mode_ == OperationalMode::TX; }

  bool has_fragment_available() const override { return rx_buffer_valid_; }

  // bool wait_fragment_available(uint16_t timeout_ms) const override {
  //   if (has_fragment_available()) {
  //     // tvsc::hal::output::println(
  //     //     "RF69HCW::wait_fragment_available() -- has_fragment_available() true first check");
  //     return true;
  //   }

  //   static constexpr uint16_t poll_delay_ms{0};
  //   auto start = tvsc::hal::time::time_millis();
  //   while ((tvsc::hal::time::time_millis() - start) < timeout_ms) {
  //     if (has_fragment_available()) {
  //       // tvsc::hal::output::println(
  //       //     "RF69HCW::wait_fragment_available() -- has_fragment_available() true");
  //       return true;
  //     }
  //     if (poll_delay_ms > 0) {
  //       tvsc::hal::time::delay_ms(poll_delay_ms);
  //     } else {
  //       YIELD;
  //     }
  //   }
  //   return false;
  // }

  void read_received_fragment(Fragment<MAX_MTU_VALUE>& fragment) override {
    ATOMIC_BLOCK_START;
    fragment = rx_buffer_;
    rx_buffer_valid_ = false;
    ATOMIC_BLOCK_END;
  }

  bool channel_activity_detected() override {
    return read_rssi_dbm() > channel_activity_threshold_dbm_;
  }

  bool transmit_fragment(const Fragment<MAX_MTU_VALUE>& fragment, uint16_t timeout_ms) override {
    if (fragment.total_length() > mtu()) {
      tvsc::hal::output::print("fragment.length larger than MTU: ");
      tvsc::hal::output::println(fragment.total_length());
      return false;
    }
    if (fragment.total_length() == 0) {
      tvsc::hal::output::println("fragment.length is zero.");
      return false;
    }

    // Ensure that we aren't interrupting an ongoing transmission.
    if (is_transmitting_fragment()) {
      tvsc::hal::output::println("transmit_fragment() -- already in tx_mode.");
      return false;
    }

    if (has_fragment_available()) {
      tvsc::hal::output::println(
          "Attempting to transmit when a fragment is available to be received.");
      tvsc::hal::output::print("rx_buffer_: ");
      tvsc::hal::output::println(to_string(rx_buffer_));
      return false;
    }

    // Ensure that we don't start receiving a message while we are pushing data into the FIFO.
    set_standby_mode();

    bool fifo_write_status{
        spi_->fifo_write(RF69HCW_REG_00_FIFO, fragment.data.data(), fragment.total_length())};
    if (!fifo_write_status) {
      tvsc::hal::output::println("transmit_fragment() -- fifo write failure.");
      return false;
    }

    // Start the transmitter.
    set_mode_tx();
    return true;
  }

  // Configuration getters and setters.

  void set_receive_sensitivity_threshold_dbm(float threshold_dbm) {
    const uint8_t value = static_cast<uint8_t>(threshold_dbm * -2);
    spi_->write(RF69HCW_REG_29_RSSITHRESH, value);
  }

  float get_receive_sensitivity_threshold_dbm() {
    uint8_t rssi_threshold = spi_->read(RF69HCW_REG_29_RSSITHRESH);
    return -1.f * rssi_threshold / 2.f;
  }

  void set_channel_activity_threshold_dbm(float threshold_dbm) {
    channel_activity_threshold_dbm_ = threshold_dbm;
  }

  float get_channel_activity_threshold_dbm() { return channel_activity_threshold_dbm_; }

  void set_frequency_hz(float frequency_hz) {
    uint32_t frf = static_cast<uint32_t>(frequency_hz / RF69HCW_FSTEP);
    spi_->write(RF69HCW_REG_07_FRFMSB, (frf >> 16) & 0xff);
    spi_->write(RF69HCW_REG_08_FRFMID, (frf >> 8) & 0xff);
    spi_->write(RF69HCW_REG_09_FRFLSB, frf & 0xff);
  }

  float get_frequency_hz() {
    uint32_t frf = spi_->read(RF69HCW_REG_07_FRFMSB);
    frf = (frf << 8) | spi_->read(RF69HCW_REG_08_FRFMID);
    frf = (frf << 8) | spi_->read(RF69HCW_REG_09_FRFLSB);
    return frf * RF69HCW_FSTEP;
  }

  void set_preamble_length(uint16_t length) {
    spi_->write(RF69HCW_REG_2C_PREAMBLEMSB, length >> 8);
    spi_->write(RF69HCW_REG_2D_PREAMBLELSB, length & 0xff);
  }

  uint16_t get_preamble_length() {
    uint16_t length = spi_->read(RF69HCW_REG_2C_PREAMBLEMSB);
    length = (length << 8) | spi_->read(RF69HCW_REG_2D_PREAMBLELSB);
    return length;
  }

  void set_sync_words_length(uint8_t length) {
    if (length > 8) {
      length = 8;
    }
    uint8_t sync_configuration = spi_->read(RF69HCW_REG_2E_SYNCCONFIG);
    if (length == 0) {
      sync_configuration &= ~RF69HCW_SYNCCONFIG_SYNCON;
    } else {
      spi_->burst_write(RF69HCW_REG_2F_SYNCVALUE1, SYNC_WORDS, length);
      sync_configuration |= RF69HCW_SYNCCONFIG_SYNCON;
    }
    sync_configuration &= ~RF69HCW_SYNCCONFIG_SYNCSIZE;
    sync_configuration |= (length - 1) << 3;
    spi_->write(RF69HCW_REG_2E_SYNCCONFIG, sync_configuration);
  }

  uint8_t get_sync_words_length() {
    uint8_t length = spi_->read(RF69HCW_REG_2E_SYNCCONFIG);
    // RF69HCW_REG_2E_SYNCCONFIG stores the sync words length minus 1 and has an explicit flag for
    // turning on the sync words at all. The assumption here is that if you turn on the sync words,
    // the length will be at least 1. See page 71 of the datasheet
    // (https://cdn-shop.adafruit.com/product-files/3076/RFM69HCW-V1.1.pdf) for more details.
    length &= RF69HCW_SYNCCONFIG_SYNCSIZE;
    length >>= 3;
    length += 1;
    return length;
  }

  void set_power_dbm(int8_t power_dbm) {
    power_ = power_dbm;
    uint8_t amplifier_configuration;

    // Clamp the power value.
    if (power_ < -2) {
      power_ = -2;
    }
    if (power_ > 20) {
      power_ = 20;
    }

    if (power_ <= 13) {
      // [-2, 13]: We only need to turn on PA1.
      amplifier_configuration =
          RF69HCW_PALEVEL_PA1ON | ((power_ + 18) & RF69HCW_PALEVEL_OUTPUTPOWER);
    } else if (power_ >= 18) {
      // [18, 20]: We need to turn on both PA1 and PA2, plus enable the high power boost.
      // Note that there is special handling on transmit for the high power boost as well.
      amplifier_configuration = RF69HCW_PALEVEL_PA1ON | RF69HCW_PALEVEL_PA2ON |
                                ((power_ + 11) & RF69HCW_PALEVEL_OUTPUTPOWER);
    } else {
      // [14, 17]: We need to turn on both PA1 and PA2, but leave the high power boost off.
      amplifier_configuration = RF69HCW_PALEVEL_PA1ON | RF69HCW_PALEVEL_PA2ON |
                                ((power_ + 14) & RF69HCW_PALEVEL_OUTPUTPOWER);
    }

    spi_->write(RF69HCW_REG_11_PALEVEL, amplifier_configuration);
  }

  int8_t get_power_dbm() const { return power_; }

  void set_modulation_scheme(tvsc_radio_nano_ModulationTechnique modulation) {
    uint8_t register_value{RF69HCW_DATAMODUL_DATAMODE_PACKET};
    switch (modulation) {
      case tvsc_radio_nano_ModulationTechnique_OOK:
        register_value |=
            RF69HCW_DATAMODUL_MODULATIONTYPE_OOK | RF69HCW_DATAMODUL_MODULATIONSHAPING_OOK_NONE;
        break;
      case tvsc_radio_nano_ModulationTechnique_FSK:
        register_value |=
            RF69HCW_DATAMODUL_MODULATIONTYPE_FSK | RF69HCW_DATAMODUL_MODULATIONSHAPING_FSK_NONE;
        break;
      case tvsc_radio_nano_ModulationTechnique_GFSK:
        register_value |=
            RF69HCW_DATAMODUL_MODULATIONTYPE_FSK | RF69HCW_DATAMODUL_MODULATIONSHAPING_FSK_BT1_0;
        break;
      default:
        except<std::domain_error>("Invalid modulation technique");
    }
    spi_->write(RF69HCW_REG_02_DATAMODUL, register_value);
  }

  tvsc_radio_nano_ModulationTechnique get_modulation_scheme() {
    uint8_t register_value = spi_->read(RF69HCW_REG_02_DATAMODUL);

    if (register_value & RF69HCW_DATAMODUL_MODULATIONTYPE_FSK) {
      if (register_value & RF69HCW_DATAMODUL_MODULATIONSHAPING_FSK_BT1_0) {
        return tvsc_radio_nano_ModulationTechnique_GFSK;
      } else {
        return tvsc_radio_nano_ModulationTechnique_FSK;
      }
    } else if (register_value & RF69HCW_DATAMODUL_MODULATIONTYPE_OOK) {
      return tvsc_radio_nano_ModulationTechnique_OOK;
    } else {
      except<std::domain_error>("Unknown modulation technique");
    }
  }

  void set_line_coding(tvsc_radio_nano_LineCoding coding) {
    uint8_t register_value = spi_->read(RF69HCW_REG_37_PACKETCONFIG1);
    register_value &= ~RF69HCW_PACKETCONFIG1_DCFREE;
    switch (coding) {
      case tvsc_radio_nano_LineCoding_NONE:
        register_value |= RF69HCW_PACKETCONFIG1_DCFREE_NONE;
        break;
      case tvsc_radio_nano_LineCoding_WHITENING:
        register_value |= RF69HCW_PACKETCONFIG1_DCFREE_WHITENING;
        break;
      case tvsc_radio_nano_LineCoding_MANCHESTER_ORIGINAL:
        register_value |= RF69HCW_PACKETCONFIG1_DCFREE_MANCHESTER;
        break;
      default:
        except<std::domain_error>("Invalid line coding");
    }
    spi_->write(RF69HCW_REG_37_PACKETCONFIG1, register_value);
  }

  tvsc_radio_nano_LineCoding get_line_coding() {
    uint8_t register_value = spi_->read(RF69HCW_REG_37_PACKETCONFIG1);

    if (register_value & RF69HCW_PACKETCONFIG1_DCFREE_MANCHESTER) {
      return tvsc_radio_nano_LineCoding_MANCHESTER_ORIGINAL;
    } else if (register_value & RF69HCW_PACKETCONFIG1_DCFREE_WHITENING) {
      return tvsc_radio_nano_LineCoding_WHITENING;
    } else {
      return tvsc_radio_nano_LineCoding_NONE;
    }
  }

  void set_bit_rate(float bit_rate) {
    uint16_t bit_rate_register_values = RF69HCW_FXOSC / bit_rate;
    spi_->write(RF69HCW_REG_03_BITRATEMSB, ((bit_rate_register_values >> 8) & 0xff));
    spi_->write(RF69HCW_REG_04_BITRATELSB, bit_rate_register_values & 0xff);

    // Setting the bit rate also means that we need to set the Rx bandwidth. We set the Rx bandwidth
    // to be 2 * bit rate. Also, we put the same value in the AfcRxBw register.
    // See page 67 of the datasheet:
    // https://cdn-shop.adafruit.com/product-files/3076/RFM69HCW-V1.1.pdf for the register
    // descriptions. See page 26 for the lookup table driving the if-block that we use below.
    // TODO(james): Adjust lookup values after measurements.
    // TODO(james): Adjust lookup values for OOK modulation. Currently, these values are for FSK
    // variants. The datasheet recommends lowering the rx_bw thresholds by 2x for OOK.
    const float rx_bw = 2.f * bit_rate;
    uint8_t mantissa{};
    uint8_t exponent{};
    if (rx_bw < 2.6f) {
      mantissa = 0b10;
      exponent = 7;
    } else if (rx_bw < 3.1f) {
      mantissa = 0b01;
      exponent = 7;
    } else if (rx_bw < 3.9f) {
      mantissa = 0b00;
      exponent = 7;
    } else if (rx_bw < 5.2f) {
      mantissa = 0b10;
      exponent = 6;
    } else if (rx_bw < 6.3f) {
      mantissa = 0b01;
      exponent = 6;
    } else if (rx_bw < 7.8f) {
      mantissa = 0b00;
      exponent = 6;
    } else if (rx_bw < 10.4f) {
      mantissa = 0b10;
      exponent = 5;
    } else if (rx_bw < 12.5f) {
      mantissa = 0b01;
      exponent = 5;
    } else if (rx_bw < 15.6f) {
      mantissa = 0b00;
      exponent = 5;
    } else if (rx_bw < 20.8f) {
      mantissa = 0b10;
      exponent = 4;
    } else if (rx_bw < 25.0f) {
      mantissa = 0b01;
      exponent = 4;
    } else if (rx_bw < 31.3f) {
      mantissa = 0b00;
      exponent = 4;
    } else if (rx_bw < 41.7f) {
      mantissa = 0b10;
      exponent = 3;
    } else if (rx_bw < 50.0f) {
      mantissa = 0b01;
      exponent = 3;
    } else if (rx_bw < 62.5f) {
      mantissa = 0b00;
      exponent = 3;
    } else if (rx_bw < 83.3f) {
      mantissa = 0b10;
      exponent = 2;
    } else if (rx_bw < 100.0f) {
      mantissa = 0b01;
      exponent = 2;
    } else if (rx_bw < 125.0f) {
      mantissa = 0b00;
      exponent = 2;
    } else if (rx_bw < 166.7f) {
      mantissa = 0b10;
      exponent = 1;
    } else if (rx_bw < 200.0f) {
      mantissa = 0b01;
      exponent = 1;
    } else if (rx_bw < 250.0f) {
      mantissa = 0b00;
      exponent = 1;
    } else if (rx_bw < 333.3f) {
      mantissa = 0b10;
      exponent = 0;
    } else if (rx_bw < 400.0f) {
      mantissa = 0b01;
      exponent = 0;
    } else {  // rx_bw ~= 500.0f
      mantissa = 0b00;
      exponent = 0;
    }

    constexpr uint8_t rx_bw_dcc_freq{2};
    constexpr uint8_t afc_bw_dcc_freq{4};
    spi_->write(RF69HCW_REG_19_RXBW, (rx_bw_dcc_freq << 5) | (mantissa << 3) | exponent);
    spi_->write(RF69HCW_REG_1A_AFCBW, (afc_bw_dcc_freq << 5) | (mantissa << 3) | exponent);
  }

  float get_bit_rate() {
    uint8_t bit_rate_msb = spi_->read(RF69HCW_REG_03_BITRATEMSB);
    uint8_t bit_rate_lsb = spi_->read(RF69HCW_REG_04_BITRATELSB);

    return RF69HCW_FXOSC / ((bit_rate_msb << 8) | bit_rate_lsb);
  }

  // TODO(james): Document what this does, and/or come up with a better name.
  void set_frequency_deviation_hz(float deviation_hz) {
    uint16_t scaled_deviation{static_cast<uint16_t>(deviation_hz / RF69HCW_FSTEP)};
    scaled_deviation &= 0x3fff;
    spi_->write(RF69HCW_REG_05_FDEVMSB, ((scaled_deviation >> 8) & 0xff));
    spi_->write(RF69HCW_REG_06_FDEVLSB, scaled_deviation & 0xff);
  }

  float get_frequency_deviation_hz() {
    uint8_t deviation_msb = spi_->read(RF69HCW_REG_05_FDEVMSB);
    uint8_t deviation_lsb = spi_->read(RF69HCW_REG_06_FDEVLSB);

    return RF69HCW_FSTEP * ((deviation_msb << 8) | deviation_lsb);
  }
};

}  // namespace tvsc::radio
