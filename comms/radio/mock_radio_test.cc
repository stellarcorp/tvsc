#include "comms/radio/mock_radio.h"

#include "gtest/gtest.h"
#include "hal/time/mock_clock.h"
#include "comms/radio/fragment.h"

namespace tvsc::comms::radio {

TEST(MockRadioTest, FragmentsAvailableAtDesignatedTime) {
  using RadioT = SmallBufferMockRadio;
  tvsc::hal::time::MockClock clock{};
  RadioT radio{clock};

  Fragment<RadioT::max_mtu()> fragment{};
  fragment.set_protocol(Protocol::TVSC_CONTROL);
  fragment.set_sender_id(1);
  radio.add_rx_fragment(1, fragment);

  radio.set_receive_mode();
  clock.set_current_time_millis(1);

  EXPECT_TRUE(radio.has_fragment_available());
}

TEST(MockRadioTest, CanReceiveFragment) {
  using RadioT = SmallBufferMockRadio;
  tvsc::hal::time::MockClock clock{};
  RadioT radio{clock};

  Fragment<RadioT::max_mtu()> fragment{};
  fragment.set_protocol(Protocol::TVSC_CONTROL);
  fragment.set_sender_id(1);
  radio.add_rx_fragment(1, fragment);

  radio.set_receive_mode();
  clock.set_current_time_millis(1);

  ASSERT_TRUE(radio.has_fragment_available());

  Fragment<RadioT::max_mtu()> received_fragment{};
  radio.read_received_fragment(received_fragment);

  EXPECT_EQ(Protocol::TVSC_CONTROL, received_fragment.protocol());
  EXPECT_EQ(1, received_fragment.sender_id());
}

TEST(MockRadioTest, CanReceiveFragmentMultipleFragments) {
  using RadioT = SmallBufferMockRadio;
  tvsc::hal::time::MockClock clock{};
  RadioT radio{clock};

  Fragment<RadioT::max_mtu()> fragment{};
  fragment.set_protocol(Protocol::TVSC_CONTROL);
  fragment.set_sender_id(1);
  radio.add_rx_fragment(1, fragment);

  fragment.set_sender_id(2);
  radio.add_rx_fragment(2, fragment);

  radio.set_receive_mode();
  clock.set_current_time_millis(1);

  ASSERT_TRUE(radio.has_fragment_available());

  Fragment<RadioT::max_mtu()> received_fragment{};
  radio.read_received_fragment(received_fragment);

  EXPECT_EQ(Protocol::TVSC_CONTROL, received_fragment.protocol());
  EXPECT_EQ(1, received_fragment.sender_id());

  clock.set_current_time_millis(2);

  radio.read_received_fragment(received_fragment);

  EXPECT_EQ(Protocol::TVSC_CONTROL, received_fragment.protocol());
  EXPECT_EQ(2, received_fragment.sender_id());
}

TEST(MockRadioTest, LeavingReceiveModeBeforeFragmentReceiptDropsFragment) {
  using RadioT = SmallBufferMockRadio;
  tvsc::hal::time::MockClock clock{};
  RadioT radio{clock};

  Fragment<RadioT::max_mtu()> fragment{};
  fragment.set_protocol(Protocol::TVSC_CONTROL);
  fragment.set_sender_id(1);
  radio.add_rx_fragment(1, fragment);

  fragment.set_sender_id(2);
  radio.add_rx_fragment(2, fragment);

  radio.set_receive_mode();
  clock.set_current_time_millis(1);

  ASSERT_TRUE(radio.has_fragment_available());

  Fragment<RadioT::max_mtu()> received_fragment{};
  radio.read_received_fragment(received_fragment);

  ASSERT_EQ(Protocol::TVSC_CONTROL, received_fragment.protocol());
  ASSERT_EQ(1, received_fragment.sender_id());

  radio.set_standby_mode();
  clock.set_current_time_millis(2);

  EXPECT_FALSE(radio.has_fragment_available());
  EXPECT_EQ(1, radio.count_dropped_fragments());
}

TEST(MockRadioTest,
     LeavingReceiveModeBeforeFragmentReceiptButReturningToReceiveModeStillDropsFragment) {
  using RadioT = SmallBufferMockRadio;
  tvsc::hal::time::MockClock clock{};
  RadioT radio{clock};

  Fragment<RadioT::max_mtu()> fragment{};
  fragment.set_protocol(Protocol::TVSC_CONTROL);
  fragment.set_sender_id(1);
  radio.add_rx_fragment(1, fragment);

  fragment.set_sender_id(2);
  radio.add_rx_fragment(2, fragment);

  radio.set_receive_mode();
  clock.set_current_time_millis(1);

  ASSERT_TRUE(radio.has_fragment_available());

  Fragment<RadioT::max_mtu()> received_fragment{};
  radio.read_received_fragment(received_fragment);

  ASSERT_EQ(Protocol::TVSC_CONTROL, received_fragment.protocol());
  ASSERT_EQ(1, received_fragment.sender_id());

  radio.set_standby_mode();
  clock.set_current_time_millis(2);
  radio.set_receive_mode();

  EXPECT_FALSE(radio.has_fragment_available());
  EXPECT_EQ(1, radio.count_dropped_fragments());
}

TEST(MockRadioTest, CanTransmitFragment) {
  using RadioT = SmallBufferMockRadio;
  tvsc::hal::time::MockClock clock{};
  RadioT radio{clock};

  Fragment<RadioT::max_mtu()> fragment{};
  fragment.set_protocol(Protocol::TVSC_CONTROL);
  fragment.set_sender_id(1);

  clock.set_current_time_millis(1);
  EXPECT_TRUE(radio.transmit_fragment(fragment, radio.fragment_transmit_time_ms()));
  clock.set_current_time_millis(clock.current_time_millis() + radio.fragment_transmit_time_ms());

  EXPECT_EQ(1, radio.sent_fragments().size());
  for (const Fragment<RadioT::max_mtu()>& sent_fragment : radio.sent_fragments()) {
    EXPECT_EQ(Protocol::TVSC_CONTROL, sent_fragment.protocol());
    EXPECT_EQ(1, sent_fragment.sender_id());
  }
}

TEST(MockRadioTest, TransmittingFragmentWhileTransmittingOtherFragmentCorrupts) {
  using RadioT = SmallBufferMockRadio;
  tvsc::hal::time::MockClock clock{};
  RadioT radio{clock};

  Fragment<RadioT::max_mtu()> fragment{};
  fragment.set_protocol(Protocol::TVSC_CONTROL);
  fragment.set_sender_id(1);

  clock.set_current_time_millis(1);
  ASSERT_TRUE(radio.transmit_fragment(fragment, radio.fragment_transmit_time_ms()));

  // Start the second transmission too early.
  clock.set_current_time_millis(clock.current_time_millis() + radio.fragment_transmit_time_ms() -
                                1);
  fragment.set_sender_id(2);
  ASSERT_TRUE(radio.transmit_fragment(fragment, radio.fragment_transmit_time_ms()));

  // Allow the second transmission to finish.
  clock.set_current_time_millis(clock.current_time_millis() + radio.fragment_transmit_time_ms());

  EXPECT_EQ(1, radio.sent_fragments().size());
  EXPECT_EQ(1, radio.count_corrupted_fragments());

  for (const Fragment<RadioT::max_mtu()>& sent_fragment : radio.sent_fragments()) {
    EXPECT_EQ(Protocol::TVSC_CONTROL, sent_fragment.protocol());
    EXPECT_EQ(2, sent_fragment.sender_id());
  }
}

}  // namespace tvsc::comms::radio
