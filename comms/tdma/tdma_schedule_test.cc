#include "comms/tdma/tdma_schedule.h"

#include "comms/tdma/tdma_frame.h"
#include "gtest/gtest.h"
#include "hal/time/mock_clock.h"

namespace tvsc::comms::tdma {

constexpr uint64_t BASE_STATION_ID{1};
constexpr uint64_t LOCAL_ID{2};
constexpr uint64_t OTHER_NODE_ID{3};

TEST(TdmaScheduleTest, PreventsTransmissionsUntilFrameDescriptionReceived) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock};
  EXPECT_FALSE(schedule.can_transmit());
}

TEST(TdmaScheduleTest, RequiresReceptionUntilFrameDescriptionReceived) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock};
  EXPECT_TRUE(schedule.should_receive());
}

TEST(TdmaScheduleTest, NoSlotBoundaryDurationUntilFrameDescriptionReceived) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock};
  EXPECT_EQ(0, schedule.time_slot_duration_remaining_us());
}

TEST(TdmaScheduleTest, CanDetermineCurrentTimeSlotTrivial) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame(schedule.cell_time_us());
    frame.add_time_skew_slot(SLOT_DURATION_US);
    schedule.set_frame(frame.build());
  }

  // Put us at the start of the time slot.
  clock.set_current_time_micros(0);

  EXPECT_EQ(TimeSlot::Role::TIME_SKEW_ALLOWANCE, schedule.time_slot_role());
}

TEST(TdmaScheduleTest, CanDetermineCurrentTimeSlot) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_node_tx_slot(SLOT_DURATION_US, BASE_STATION_ID);
    frame.add_time_skew_slot(SLOT_DURATION_US);
    frame.add_node_tx_slot(SLOT_DURATION_US, LOCAL_ID);
    frame.add_time_skew_slot(SLOT_DURATION_US);
    frame.add_association_slot(SLOT_DURATION_US);
    frame.add_time_skew_slot(SLOT_DURATION_US);

    schedule.set_frame(frame.build());
  }

  // Put us at the start of the time slot.
  clock.set_current_time_micros(0);
  EXPECT_EQ(TimeSlot::Role::NODE_TX, schedule.time_slot_role());
  clock.increment_current_time_micros(SLOT_DURATION_US / 2);
  EXPECT_EQ(TimeSlot::Role::NODE_TX, schedule.time_slot_role());

  // Middle of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::TIME_SKEW_ALLOWANCE, schedule.time_slot_role());

  // Middle of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::NODE_TX, schedule.time_slot_role());

  // Middle of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::TIME_SKEW_ALLOWANCE, schedule.time_slot_role());

  // Middle of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::ASSOCIATION, schedule.time_slot_role());

  // Middle of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::TIME_SKEW_ALLOWANCE, schedule.time_slot_role());
}

TEST(TdmaScheduleTest, CanDetermineCurrentTimeSlotEdges) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_node_tx_slot(SLOT_DURATION_US, BASE_STATION_ID);
    frame.add_time_skew_slot(SLOT_DURATION_US);
    frame.add_node_tx_slot(SLOT_DURATION_US, LOCAL_ID);
    frame.add_time_skew_slot(SLOT_DURATION_US);
    frame.add_association_slot(SLOT_DURATION_US);
    frame.add_time_skew_slot(SLOT_DURATION_US);

    schedule.set_frame(frame.build());
  }

  // Put us at the start of the time slot.
  clock.set_current_time_micros(0);

  EXPECT_EQ(TimeSlot::Role::NODE_TX, schedule.time_slot_role());

  // Start of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::TIME_SKEW_ALLOWANCE, schedule.time_slot_role());

  // Start of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::NODE_TX, schedule.time_slot_role());

  // Start of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::TIME_SKEW_ALLOWANCE, schedule.time_slot_role());

  // Start of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::ASSOCIATION, schedule.time_slot_role());

  // Start of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::TIME_SKEW_ALLOWANCE, schedule.time_slot_role());
}

TEST(TdmaScheduleBehaviorTest, PreventsTransmissionDuringBlackoutSlot) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_blackout_slot(SLOT_DURATION_US);
    schedule.set_frame(frame.build());
  }

  EXPECT_FALSE(schedule.can_transmit());
}

TEST(TdmaScheduleBehaviorTest, DoesNotRequireReceptionDuringBlackoutSlot) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_blackout_slot(SLOT_DURATION_US);
    schedule.set_frame(frame.build());
  }

  EXPECT_FALSE(schedule.should_receive());
}

TEST(TdmaScheduleBehaviorTest, PreventsTransmissionDuringNodeTxSlotWhenNotOwner) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_node_tx_slot(SLOT_DURATION_US, OTHER_NODE_ID);
    schedule.set_frame(frame.build());
  }

  EXPECT_FALSE(schedule.can_transmit());
}

TEST(TdmaScheduleBehaviorTest, RequiresReceptionDuringNodeTxSlotWhenNotOwner) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_node_tx_slot(SLOT_DURATION_US, OTHER_NODE_ID);
    schedule.set_frame(frame.build());
  }

  EXPECT_TRUE(schedule.should_receive());
}

TEST(TdmaScheduleBehaviorTest, AllowsTransmissionDuringNodeTxSlotWhenOwner) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_node_tx_slot(SLOT_DURATION_US, LOCAL_ID);
    schedule.set_frame(frame.build());
  }

  EXPECT_TRUE(schedule.can_transmit());
}

TEST(TdmaScheduleBehaviorTest, DoesNotRequireReceptionDuringNodeTxSlotWhenOwner) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_node_tx_slot(SLOT_DURATION_US, LOCAL_ID);
    schedule.set_frame(frame.build());
  }

  EXPECT_FALSE(schedule.should_receive());
}

TEST(TdmaScheduleBehaviorTest, PreventsTransmissionDuringAssociationSlotWhenAssociated) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.set_base_station_id(BASE_STATION_ID);
    frame.add_association_slot(SLOT_DURATION_US);
    frame.add_node_tx_slot(SLOT_DURATION_US, LOCAL_ID);
    schedule.set_frame(frame.build());
  }

  ASSERT_TRUE(schedule.is_associated());

  EXPECT_FALSE(schedule.can_transmit());
}

TEST(TdmaScheduleBehaviorTest, PreventsTransmissionDuringAssociationSlotWhenBaseStation) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock, BASE_STATION_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.set_base_station_id(BASE_STATION_ID);
    frame.add_association_slot(SLOT_DURATION_US);
    schedule.set_frame(frame.build());
  }

  ASSERT_FALSE(schedule.is_associated());
  ASSERT_TRUE(schedule.is_base_station());

  EXPECT_FALSE(schedule.can_transmit());
}

TEST(TdmaScheduleBehaviorTest, AllowsTransmissionDuringAssociationSlotWhenNotAssociated) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.set_base_station_id(BASE_STATION_ID);
    frame.add_association_slot(SLOT_DURATION_US);
    schedule.set_frame(frame.build());
  }

  ASSERT_FALSE(schedule.is_associated());
  ASSERT_FALSE(schedule.is_base_station());

  EXPECT_TRUE(schedule.can_transmit());
}

TEST(TdmaScheduleBehaviorTest, DoesNotRequireReceptionDuringAssociationSlotIfNotBaseStation) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.set_base_station_id(BASE_STATION_ID);
    frame.add_association_slot(SLOT_DURATION_US);
    schedule.set_frame(frame.build());
  }

  ASSERT_FALSE(schedule.is_base_station());

  EXPECT_FALSE(schedule.should_receive());
}

TEST(TdmaScheduleBehaviorTest, RequiresReceptionDuringAssociationSlotIfBaseStation) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock, BASE_STATION_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.set_base_station_id(BASE_STATION_ID);
    frame.add_association_slot(SLOT_DURATION_US);
    schedule.set_frame(frame.build());
  }

  ASSERT_TRUE(schedule.is_base_station());

  EXPECT_TRUE(schedule.should_receive());
}

TEST(TdmaScheduleBehaviorTest, PreventsTransmissionDuringTimeSkewAllowanceSlot) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_time_skew_slot(SLOT_DURATION_US);
    schedule.set_frame(frame.build());
  }

  EXPECT_FALSE(schedule.can_transmit());
}

TEST(TdmaScheduleBehaviorTest, RequiresReceptionDuringTimeSkewAllowanceSlot) {
  tvsc::hal::time::MockClock clock{};
  TdmaSchedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_time_skew_slot(SLOT_DURATION_US);
    schedule.set_frame(frame.build());
  }

  EXPECT_TRUE(schedule.should_receive());
}

}  // namespace tvsc::comms::tdma
