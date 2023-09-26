#include "comms/tdma/schedule.h"

#include "comms/tdma/tdma_frame.h"
#include "gtest/gtest.h"
#include "hal/time/mock_clock.h"

namespace tvsc::comms::tdma {

constexpr uint64_t BASE_STATION_ID{1};
constexpr uint64_t LOCAL_ID{2};
constexpr uint64_t OTHER_NODE_ID{3};

TEST(ScheduleTest, DefaultFramePreventsTransmissions) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock};
  EXPECT_FALSE(schedule.can_transmit());
}

TEST(ScheduleTest, DefaultFrameRequiresReception) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock};
  EXPECT_TRUE(schedule.should_receive());
}

TEST(ScheduleTest, BaseStationFrameAllowsBaseStationToTransmit) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, BASE_STATION_ID};

  schedule.set_frame(FrameBuilder::create_default_base_station_frame(BASE_STATION_ID));

  clock.set_current_time_micros(0);
  EXPECT_EQ(TimeSlot::Role::NODE_TX, schedule.time_slot_role());

  EXPECT_TRUE(schedule.is_base_station());
  EXPECT_TRUE(schedule.is_associated());
  EXPECT_TRUE(schedule.can_transmit());
  EXPECT_FALSE(schedule.should_receive());
}

TEST(ScheduleTest, CanDetermineCurrentTimeSlotTrivial) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame(schedule.cell_time_us());
    frame.add_guard_interval(SLOT_DURATION_US);
    schedule.set_frame(frame.build());
  }

  // Put us at the start of the time slot.
  clock.set_current_time_micros(0);

  EXPECT_EQ(TimeSlot::Role::GUARD_INTERVAL, schedule.time_slot_role());
}

TEST(ScheduleTest, CanDetermineCurrentTimeSlot) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_node_tx_slot(SLOT_DURATION_US, BASE_STATION_ID);
    frame.add_guard_interval(SLOT_DURATION_US);
    frame.add_node_tx_slot(SLOT_DURATION_US, LOCAL_ID);
    frame.add_guard_interval(SLOT_DURATION_US);
    frame.add_association_slot(SLOT_DURATION_US);
    frame.add_guard_interval(SLOT_DURATION_US);

    schedule.set_frame(frame.build());
  }

  // Put us at the start of the time slot.
  clock.set_current_time_micros(0);
  EXPECT_EQ(TimeSlot::Role::NODE_TX, schedule.time_slot_role());
  clock.increment_current_time_micros(SLOT_DURATION_US / 2);
  EXPECT_EQ(TimeSlot::Role::NODE_TX, schedule.time_slot_role());

  // Middle of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::GUARD_INTERVAL, schedule.time_slot_role());

  // Middle of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::NODE_TX, schedule.time_slot_role());

  // Middle of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::GUARD_INTERVAL, schedule.time_slot_role());

  // Middle of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::ASSOCIATION, schedule.time_slot_role());

  // Middle of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::GUARD_INTERVAL, schedule.time_slot_role());
}

TEST(ScheduleTest, CanDetermineCurrentTimeSlotEdges) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_node_tx_slot(SLOT_DURATION_US, BASE_STATION_ID);
    frame.add_guard_interval(SLOT_DURATION_US);
    frame.add_node_tx_slot(SLOT_DURATION_US, LOCAL_ID);
    frame.add_guard_interval(SLOT_DURATION_US);
    frame.add_association_slot(SLOT_DURATION_US);
    frame.add_guard_interval(SLOT_DURATION_US);

    schedule.set_frame(frame.build());
  }

  // Put us at the start of the time slot.
  clock.set_current_time_micros(0);

  EXPECT_EQ(TimeSlot::Role::NODE_TX, schedule.time_slot_role());

  // Start of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::GUARD_INTERVAL, schedule.time_slot_role());

  // Start of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::NODE_TX, schedule.time_slot_role());

  // Start of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::GUARD_INTERVAL, schedule.time_slot_role());

  // Start of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::ASSOCIATION, schedule.time_slot_role());

  // Start of next time slot.
  clock.increment_current_time_micros(SLOT_DURATION_US);
  EXPECT_EQ(TimeSlot::Role::GUARD_INTERVAL, schedule.time_slot_role());
}

TEST(ScheduleBehaviorTest, PreventsTransmissionDuringBlackoutSlot) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_blackout_slot(SLOT_DURATION_US);
    schedule.set_frame(frame.build());
  }

  EXPECT_FALSE(schedule.can_transmit());
}

TEST(ScheduleBehaviorTest, DoesNotRequireReceptionDuringBlackoutSlot) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_blackout_slot(SLOT_DURATION_US);
    schedule.set_frame(frame.build());
  }

  EXPECT_FALSE(schedule.should_receive());
}

TEST(ScheduleBehaviorTest, PreventsTransmissionDuringNodeTxSlotWhenNotOwner) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_node_tx_slot(SLOT_DURATION_US, OTHER_NODE_ID);
    schedule.set_frame(frame.build());
  }

  EXPECT_FALSE(schedule.can_transmit());
}

TEST(ScheduleBehaviorTest, RequiresReceptionDuringNodeTxSlotWhenNotOwner) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_node_tx_slot(SLOT_DURATION_US, OTHER_NODE_ID);
    schedule.set_frame(frame.build());
  }

  EXPECT_TRUE(schedule.should_receive());
}

TEST(ScheduleBehaviorTest, AllowsTransmissionDuringNodeTxSlotWhenOwner) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_node_tx_slot(SLOT_DURATION_US, LOCAL_ID);
    schedule.set_frame(frame.build());
  }

  EXPECT_TRUE(schedule.can_transmit());
}

TEST(ScheduleBehaviorTest, DoesNotRequireReceptionDuringNodeTxSlotWhenOwner) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_node_tx_slot(SLOT_DURATION_US, LOCAL_ID);
    schedule.set_frame(frame.build());
  }

  EXPECT_FALSE(schedule.should_receive());
}

TEST(ScheduleBehaviorTest, PreventsTransmissionDuringAssociationSlotWhenAssociated) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, LOCAL_ID};

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

TEST(ScheduleBehaviorTest, PreventsTransmissionDuringAssociationSlotWhenBaseStation) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, BASE_STATION_ID};

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

TEST(ScheduleBehaviorTest, AllowsTransmissionDuringAssociationSlotWhenNotAssociated) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, LOCAL_ID};

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

TEST(ScheduleBehaviorTest, DoesNotRequireReceptionDuringAssociationSlotIfNotBaseStation) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, LOCAL_ID};

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

TEST(ScheduleBehaviorTest, RequiresReceptionDuringAssociationSlotIfBaseStation) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, BASE_STATION_ID};

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

TEST(ScheduleBehaviorTest, PreventsTransmissionDuringTimeSkewAllowanceSlot) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_guard_interval(SLOT_DURATION_US);
    schedule.set_frame(frame.build());
  }

  EXPECT_FALSE(schedule.can_transmit());
}

TEST(ScheduleBehaviorTest, RequiresReceptionDuringTimeSkewAllowanceSlot) {
  tvsc::hal::time::MockClock clock{};
  Schedule schedule{clock, LOCAL_ID};

  constexpr uint32_t SLOT_DURATION_US{1000};
  {
    FrameBuilder frame{schedule.cell_time_us()};
    frame.add_guard_interval(SLOT_DURATION_US);
    schedule.set_frame(frame.build());
  }

  EXPECT_TRUE(schedule.should_receive());
}

}  // namespace tvsc::comms::tdma
