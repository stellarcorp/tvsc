#include "comms/radio/mock_radio.h"

#include "comms/radio/fragment.h"
#include "gtest/gtest.h"
#include "hal/time/mock_clock.h"

namespace tvsc::comms::radio {

TEST(MockRadioTest, FragmentsAvailableAtDesignatedTime) {
  using RadioT = SmallBufferMockRadio;
  tvsc::hal::time::MockClock clock{};
  RadioT radio{clock};

  Fragment<RadioT::max_mtu()> fragment{};
  fragment.set_protocol(Protocol::TVSC_TDMA_CONTROL);
  fragment.set_sender_id(1);
  radio.add_rx_fragment(1, fragment);

  radio.set_receive_mode();
  clock.set_current_time_micros(1);

  EXPECT_TRUE(radio.has_fragment_available());
}

TEST(MockRadioTest, CanReceiveFragment) {
  using RadioT = SmallBufferMockRadio;
  tvsc::hal::time::MockClock clock{};
  RadioT radio{clock};

  Fragment<RadioT::max_mtu()> fragment{};
  fragment.set_protocol(Protocol::TVSC_TDMA_CONTROL);
  fragment.set_sender_id(1);
  radio.add_rx_fragment(1, fragment);

  radio.set_receive_mode();
  clock.set_current_time_micros(1);

  ASSERT_TRUE(radio.has_fragment_available());

  Fragment<RadioT::max_mtu()> received_fragment{};
  radio.read_received_fragment(received_fragment);

  EXPECT_EQ(Protocol::TVSC_TDMA_CONTROL, received_fragment.protocol());
  EXPECT_EQ(1, received_fragment.sender_id());
}

TEST(MockRadioTest, CanReceiveFragmentMultipleFragments) {
  using RadioT = SmallBufferMockRadio;
  tvsc::hal::time::MockClock clock{};
  RadioT radio{clock};

  Fragment<RadioT::max_mtu()> fragment{};
  fragment.set_protocol(Protocol::TVSC_TDMA_CONTROL);
  fragment.set_sender_id(1);
  radio.add_rx_fragment(1, fragment);

  fragment.set_sender_id(2);
  radio.add_rx_fragment(2, fragment);

  radio.set_receive_mode();
  clock.set_current_time_micros(1);

  EXPECT_TRUE(radio.has_fragment_available());

  Fragment<RadioT::max_mtu()> received_fragment{};
  radio.read_received_fragment(received_fragment);

  EXPECT_EQ(Protocol::TVSC_TDMA_CONTROL, received_fragment.protocol());
  EXPECT_EQ(1, received_fragment.sender_id());

  clock.set_current_time_micros(2);

  EXPECT_TRUE(radio.has_fragment_available());

  radio.read_received_fragment(received_fragment);

  EXPECT_EQ(Protocol::TVSC_TDMA_CONTROL, received_fragment.protocol());
  EXPECT_EQ(2, received_fragment.sender_id());

  clock.set_current_time_micros(3);

  // No more fragments.
  EXPECT_FALSE(radio.has_fragment_available());
}

TEST(MockRadioTest, LeavingReceiveModeBeforeFragmentReceiptDropsFragment) {
  using RadioT = SmallBufferMockRadio;
  tvsc::hal::time::MockClock clock{};
  RadioT radio{clock};

  Fragment<RadioT::max_mtu()> fragment{};
  fragment.set_protocol(Protocol::TVSC_TDMA_CONTROL);
  fragment.set_sender_id(1);
  radio.add_rx_fragment(1, fragment);

  fragment.set_sender_id(2);
  radio.add_rx_fragment(2, fragment);

  radio.set_receive_mode();
  clock.set_current_time_micros(1);

  ASSERT_TRUE(radio.has_fragment_available());

  Fragment<RadioT::max_mtu()> received_fragment{};
  radio.read_received_fragment(received_fragment);

  ASSERT_EQ(Protocol::TVSC_TDMA_CONTROL, received_fragment.protocol());
  ASSERT_EQ(1, received_fragment.sender_id());

  radio.set_standby_mode();
  clock.set_current_time_micros(3);

  EXPECT_FALSE(radio.has_fragment_available());
  EXPECT_EQ(1, radio.count_dropped_fragments());
}

TEST(MockRadioTest,
     LeavingReceiveModeBeforeFragmentReceiptButReturningToReceiveModeStillDropsFragment) {
  using RadioT = SmallBufferMockRadio;
  tvsc::hal::time::MockClock clock{};
  RadioT radio{clock};

  Fragment<RadioT::max_mtu()> fragment{};
  fragment.set_protocol(Protocol::TVSC_TDMA_CONTROL);
  fragment.set_sender_id(1);
  radio.add_rx_fragment(1, fragment);

  fragment.set_sender_id(2);
  radio.add_rx_fragment(2, fragment);

  radio.set_receive_mode();
  clock.set_current_time_micros(1);

  ASSERT_TRUE(radio.has_fragment_available());

  Fragment<RadioT::max_mtu()> received_fragment{};
  radio.read_received_fragment(received_fragment);

  ASSERT_EQ(Protocol::TVSC_TDMA_CONTROL, received_fragment.protocol());
  ASSERT_EQ(1, received_fragment.sender_id());

  radio.set_standby_mode();

  clock.set_current_time_micros(3);
  radio.set_receive_mode();

  EXPECT_FALSE(radio.has_fragment_available());
  EXPECT_EQ(1, radio.count_dropped_fragments());
}

TEST(MockRadioTest, CanTransmitFragment) {
  using RadioT = SmallBufferMockRadio;
  tvsc::hal::time::MockClock clock{};
  RadioT radio{clock};

  Fragment<RadioT::max_mtu()> fragment{};
  fragment.set_protocol(Protocol::TVSC_TDMA_CONTROL);
  fragment.set_sender_id(1);

  clock.set_current_time_micros(1);
  EXPECT_TRUE(radio.transmit_fragment(fragment));
  clock.set_current_time_micros(clock.current_time_micros() + radio.fragment_transmit_time_us());

  EXPECT_EQ(1, radio.sent_fragments().size());
  for (const Fragment<RadioT::max_mtu()>& sent_fragment : radio.sent_fragments()) {
    EXPECT_EQ(Protocol::TVSC_TDMA_CONTROL, sent_fragment.protocol());
    EXPECT_EQ(1, sent_fragment.sender_id());
  }
}

TEST(MockRadioTest, SwitchesToStandbyModeAfterTransmittingFragment) {
  using RadioT = SmallBufferMockRadio;
  tvsc::hal::time::MockClock clock{};
  RadioT radio{clock};

  Fragment<RadioT::max_mtu()> fragment{};
  fragment.set_protocol(Protocol::TVSC_TDMA_CONTROL);
  fragment.set_sender_id(1);

  clock.set_current_time_micros(1);
  ASSERT_TRUE(radio.transmit_fragment(fragment));
  EXPECT_TRUE(radio.in_tx_mode());

  clock.increment_current_time_micros(radio.fragment_transmit_time_us());

  ASSERT_EQ(1, radio.sent_fragments().size());
  EXPECT_TRUE(radio.in_standby_mode());
}

TEST(MockRadioTest, TransmittingFragmentWhileTransmittingOtherFragmentCorrupts) {
  using RadioT = SmallBufferMockRadio;
  tvsc::hal::time::MockClock clock{};
  RadioT radio{clock};

  Fragment<RadioT::max_mtu()> fragment{};
  fragment.set_protocol(Protocol::TVSC_TDMA_CONTROL);
  fragment.set_sender_id(1);

  clock.set_current_time_micros(1);
  ASSERT_TRUE(radio.transmit_fragment(fragment));

  // Start the second transmission too early.
  clock.set_current_time_micros(clock.current_time_micros() + radio.fragment_transmit_time_us() -
                                1);
  fragment.set_sender_id(2);
  ASSERT_TRUE(radio.transmit_fragment(fragment));

  // Allow the second transmission to finish.
  clock.set_current_time_micros(clock.current_time_micros() + radio.fragment_transmit_time_us());

  EXPECT_EQ(1, radio.sent_fragments().size());
  EXPECT_EQ(1, radio.count_corrupted_fragments());

  for (const Fragment<RadioT::max_mtu()>& sent_fragment : radio.sent_fragments()) {
    EXPECT_EQ(Protocol::TVSC_TDMA_CONTROL, sent_fragment.protocol());
    EXPECT_EQ(2, sent_fragment.sender_id());
  }
}

}  // namespace tvsc::comms::radio
