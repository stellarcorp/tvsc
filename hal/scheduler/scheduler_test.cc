#include "hal/scheduler/scheduler.h"

#include <coroutine>
#include <cstdint>
#include <functional>

#include "gtest/gtest.h"
#include "hal/rcc/rcc_noop.h"
#include "hal/scheduler/sample_tasks.h"
#include "hal/scheduler/task.h"
#include "hal/time/clock.h"
#include "hal/time/mock_clock.h"

namespace tvsc::hal::scheduler {

TEST(SchedulerTest, QueueIsEmptyAfterCreation) {
  time::MockClock clock{};
  rcc::RccNoop rcc{};
  Scheduler<4> scheduler{clock, rcc};
  EXPECT_EQ(0, scheduler.queue_size());
}

TEST(SchedulerTest, CanQueueTask) {
  time::MockClock clock{};
  rcc::RccNoop rcc{};
  Task task{just_return()};
  Scheduler<4> scheduler{clock, rcc};
  scheduler.add_task(std::move(task));
  EXPECT_EQ(1, scheduler.queue_size()) << to_string(scheduler);
}

TEST(SchedulerTest, CanQueueTaskThatTakesParameters) {
  static constexpr size_t NUM_ITERATIONS{1};
  static constexpr uint64_t WAKE_INTERVAL_US{1};
  time::MockClock clock{};
  rcc::RccNoop rcc{};
  int run_count{};
  Task task{do_something<NUM_ITERATIONS, WAKE_INTERVAL_US>(clock, run_count)};
  Scheduler<4> scheduler{clock, rcc};
  scheduler.add_task(std::move(task));
  EXPECT_EQ(1, scheduler.queue_size());
}

TEST(SchedulerTest, CanQueueTwoTasks) {
  time::MockClock clock{};
  rcc::RccNoop rcc{};
  int run_count{};
  Task task1{just_return()};
  Task task2{run_forever(run_count)};
  Scheduler<4> scheduler{clock, rcc};
  scheduler.add_task(std::move(task1));
  scheduler.add_task(std::move(task2));
  EXPECT_EQ(2, scheduler.queue_size());
}

TEST(SchedulerTest, CanQueueSeveralTasks) {
  time::MockClock clock{};
  rcc::RccNoop rcc{};
  int run_count{};
  Task task1{just_return()};
  Task task2{run_forever(run_count)};
  Task task3{run_forever(run_count)};
  Task task4{just_return()};
  Scheduler<4> scheduler{clock, rcc};
  scheduler.add_task(std::move(task1));
  scheduler.add_task(std::move(task2));
  scheduler.add_task(std::move(task3));
  scheduler.add_task(std::move(task4));
  EXPECT_EQ(4, scheduler.queue_size());
}

TEST(SchedulerTest, CanRunTaskThatRunsOnce) {
  time::MockClock clock{};
  rcc::RccNoop rcc{};
  Task task{just_return()};
  EXPECT_FALSE(task.is_complete());
  Scheduler<4> scheduler{clock, rcc};
  scheduler.add_task(std::move(task));
  scheduler.run_tasks_once();
  EXPECT_TRUE(task.is_complete());
  EXPECT_EQ(0, scheduler.queue_size()) << to_string(scheduler);
}

TEST(SchedulerTest, CanRunNeverendingTaskTwice) {
  static constexpr size_t NUM_ITERATIONS{2};
  int run_count{};
  Task task{};

  time::MockClock clock{};
  rcc::RccNoop rcc{};
  Scheduler<1> scheduler{clock, rcc};
  size_t task_index{scheduler.add_task(run_forever(run_count))};

  for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
    scheduler.run_tasks_once();
  }

  EXPECT_FALSE(scheduler.task(task_index).is_complete());
  EXPECT_EQ(NUM_ITERATIONS, run_count);
}

TEST(SchedulerTest, CanRunNeverendingTaskMultipleTimes) {
  static constexpr size_t NUM_ITERATIONS{10};
  int run_count{};

  time::MockClock clock{};
  rcc::RccNoop rcc{};
  Scheduler<1> scheduler{clock, rcc};
  size_t task_index{scheduler.add_task(run_forever(run_count))};

  for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
    scheduler.run_tasks_once();
  }

  EXPECT_FALSE(scheduler.task(task_index).is_complete());
  EXPECT_EQ(NUM_ITERATIONS, run_count);
}

TEST(SchedulerTest, CanRunTaskThatAddsSubtasks) {
  static constexpr size_t QUEUE_SIZE{4};
  int run_count{};
  time::MockClock clock{};
  rcc::RccNoop rcc{};
  Scheduler<QUEUE_SIZE> scheduler{clock, rcc};
  Task task{creates_subtask<QUEUE_SIZE, 1>(scheduler, run_count)};
  scheduler.add_task(std::move(task));

  scheduler.run_tasks_once();
  EXPECT_EQ(2, scheduler.queue_size()) << to_string(scheduler);
  EXPECT_EQ(2, run_count);

  run_count = 0;
  scheduler.run_tasks_once();
  EXPECT_EQ(3, scheduler.queue_size()) << to_string(scheduler);
  EXPECT_EQ(3, run_count);
}

TEST(SchedulerTest, CanRunTaskOnce) {
  static constexpr size_t NUM_ITERATIONS{1};
  static constexpr uint64_t WAKE_INTERVAL_US{1};
  time::MockClock clock{};
  rcc::RccNoop rcc{};
  int run_count{};

  Scheduler<4> scheduler{clock, rcc};
  size_t task_index{
      scheduler.add_task(do_something<NUM_ITERATIONS, WAKE_INTERVAL_US>(clock, run_count))};

  EXPECT_FALSE(scheduler.task(task_index).is_complete());

  for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
    for (size_t j = 0; j < WAKE_INTERVAL_US; ++j) {
      scheduler.run_tasks_once();
      EXPECT_FALSE(scheduler.task(task_index).is_complete());
      EXPECT_EQ(1, scheduler.queue_size()) << to_string(scheduler);
      clock.increment_current_time_micros();
    }
  }

  scheduler.run_tasks_once();
  EXPECT_TRUE(scheduler.task(task_index).is_complete());
  EXPECT_EQ(0, scheduler.queue_size()) << to_string(scheduler);
  EXPECT_EQ(NUM_ITERATIONS, run_count);
}

TEST(SchedulerTest, CanRunTaskThatSleeps) {
  static constexpr size_t NUM_ITERATIONS{1};
  static constexpr uint64_t WAKE_INTERVAL_US{10};
  time::MockClock clock{};
  rcc::RccNoop rcc{};
  int run_count{};

  Scheduler<4> scheduler{clock, rcc};
  size_t task_index{
      scheduler.add_task(do_something<NUM_ITERATIONS, WAKE_INTERVAL_US>(clock, run_count))};

  EXPECT_FALSE(scheduler.task(task_index).is_complete());

  for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
    for (size_t j = 0; j < WAKE_INTERVAL_US; ++j) {
      scheduler.run_tasks_once();
      EXPECT_FALSE(scheduler.task(task_index).is_complete());
      EXPECT_EQ(1, scheduler.queue_size()) << to_string(scheduler);
      clock.increment_current_time_micros();
    }
  }

  scheduler.run_tasks_once();
  EXPECT_TRUE(scheduler.task(task_index).is_complete());
  EXPECT_EQ(0, scheduler.queue_size()) << to_string(scheduler);
  EXPECT_EQ(NUM_ITERATIONS, run_count);
}

TEST(SchedulerTest, CanRunTaskThatSleepsMultipleTimes) {
  static constexpr size_t NUM_ITERATIONS{20};
  static constexpr uint64_t WAKE_INTERVAL_US{10};
  time::MockClock clock{};
  rcc::RccNoop rcc{};
  int run_count{};

  Scheduler<4> scheduler{clock, rcc};
  size_t task_index{
      scheduler.add_task(do_something<NUM_ITERATIONS, WAKE_INTERVAL_US>(clock, run_count))};

  EXPECT_FALSE(scheduler.task(task_index).is_complete());

  for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
    for (size_t j = 0; j < WAKE_INTERVAL_US; ++j) {
      scheduler.run_tasks_once();
      EXPECT_FALSE(scheduler.task(task_index).is_complete());
      EXPECT_EQ(1, scheduler.queue_size()) << to_string(scheduler);
      clock.increment_current_time_micros();
    }
  }

  scheduler.run_tasks_once();
  EXPECT_TRUE(scheduler.task(task_index).is_complete());
  EXPECT_EQ(0, scheduler.queue_size()) << to_string(scheduler);
  EXPECT_EQ(NUM_ITERATIONS, run_count);
}

}  // namespace tvsc::hal::scheduler
