#include "system/scheduler.h"

#include <coroutine>
#include <cstdint>
#include <functional>

#include "gtest/gtest.h"
#include "hal/rcc/rcc_noop.h"
#include "system/sample_tasks.h"
#include "system/task.h"
#include "time/mock_clock.h"

namespace tvsc::system {

using ClockType = tvsc::time::MockClock;
using TaskType = Task<ClockType>;
static constexpr size_t DEFAULT_QUEUE_SIZE{4};
using SchedulerType = Scheduler<ClockType, DEFAULT_QUEUE_SIZE>;

TEST(SchedulerTest, QueueIsEmptyAfterCreation) {
  tvsc::hal::rcc::RccNoop rcc{};
  SchedulerType scheduler{rcc};
  EXPECT_EQ(0, scheduler.queue_size());
}

TEST(SchedulerTest, CanQueueTask) {
  tvsc::hal::rcc::RccNoop rcc{};
  TaskType task{just_return<ClockType>()};
  SchedulerType scheduler{rcc};
  scheduler.add_task(std::move(task));
  EXPECT_EQ(1, scheduler.queue_size()) << to_string(scheduler);
}

TEST(SchedulerTest, CanQueueTaskThatTakesParameters) {
  static constexpr size_t NUM_ITERATIONS{1};
  static constexpr uint64_t WAKE_INTERVAL_US{1};

  tvsc::hal::rcc::RccNoop rcc{};
  int run_count{};
  TaskType task{do_something<ClockType, NUM_ITERATIONS, WAKE_INTERVAL_US>(run_count)};
  SchedulerType scheduler{rcc};
  scheduler.add_task(std::move(task));
  EXPECT_EQ(1, scheduler.queue_size());
}

TEST(SchedulerTest, CanQueueTwoTasks) {
  tvsc::hal::rcc::RccNoop rcc{};
  int run_count{};
  TaskType task1{just_return<ClockType>()};
  TaskType task2{run_forever<ClockType>(run_count)};
  SchedulerType scheduler{rcc};
  scheduler.add_task(std::move(task1));
  scheduler.add_task(std::move(task2));
  EXPECT_EQ(2, scheduler.queue_size());
}

TEST(SchedulerTest, CanQueueSeveralTasks) {
  tvsc::hal::rcc::RccNoop rcc{};
  int run_count{};
  TaskType task1{just_return<ClockType>()};
  TaskType task2{run_forever<ClockType>(run_count)};
  TaskType task3{run_forever<ClockType>(run_count)};
  TaskType task4{just_return<ClockType>()};
  SchedulerType scheduler{rcc};
  scheduler.add_task(std::move(task1));
  scheduler.add_task(std::move(task2));
  scheduler.add_task(std::move(task3));
  scheduler.add_task(std::move(task4));
  EXPECT_EQ(4, scheduler.queue_size());
}

TEST(SchedulerTest, CanRunTaskThatRunsOnce) {
  tvsc::hal::rcc::RccNoop rcc{};
  TaskType task{just_return<ClockType>()};
  EXPECT_FALSE(task.is_complete());
  SchedulerType scheduler{rcc};
  scheduler.add_task(std::move(task));
  scheduler.run_tasks_once();
  EXPECT_TRUE(task.is_complete());
  EXPECT_EQ(0, scheduler.queue_size()) << to_string(scheduler);
}

TEST(SchedulerTest, CanRunNeverendingTaskTwice) {
  static constexpr size_t NUM_ITERATIONS{2};
  int run_count{};
  TaskType task{};

  tvsc::hal::rcc::RccNoop rcc{};
  Scheduler<ClockType, 1> scheduler{rcc};
  size_t task_index{scheduler.add_task(run_forever<ClockType>(run_count))};

  for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
    scheduler.run_tasks_once();
  }

  EXPECT_FALSE(scheduler.task(task_index).is_complete());
  EXPECT_EQ(NUM_ITERATIONS, run_count);
}

TEST(SchedulerTest, CanRunNeverendingTaskMultipleTimes) {
  static constexpr size_t NUM_ITERATIONS{10};
  int run_count{};

  tvsc::hal::rcc::RccNoop rcc{};
  Scheduler<ClockType, 1> scheduler{rcc};
  size_t task_index{scheduler.add_task(run_forever<ClockType>(run_count))};

  for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
    scheduler.run_tasks_once();
  }

  EXPECT_FALSE(scheduler.task(task_index).is_complete());
  EXPECT_EQ(NUM_ITERATIONS, run_count);
}

TEST(SchedulerTest, CanRunTaskThatAddsSubtasks) {
  int run_count{};
  tvsc::hal::rcc::RccNoop rcc{};
  SchedulerType scheduler{rcc};
  TaskType task{creates_subtask<ClockType, DEFAULT_QUEUE_SIZE, 1>(scheduler, run_count)};
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

  tvsc::hal::rcc::RccNoop rcc{};
  int run_count{};

  SchedulerType scheduler{rcc};
  ClockType& clock{ClockType::clock()};
  size_t task_index{
      scheduler.add_task(do_something<ClockType, NUM_ITERATIONS, WAKE_INTERVAL_US>(run_count))};

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

  tvsc::hal::rcc::RccNoop rcc{};
  int run_count{};

  SchedulerType scheduler{rcc};
  ClockType& clock{ClockType::clock()};
  size_t task_index{
      scheduler.add_task(do_something<ClockType, NUM_ITERATIONS, WAKE_INTERVAL_US>(run_count))};

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

  tvsc::hal::rcc::RccNoop rcc{};
  int run_count{};

  SchedulerType scheduler{rcc};
  ClockType& clock{ClockType::clock()};
  size_t task_index{
      scheduler.add_task(do_something<ClockType, NUM_ITERATIONS, WAKE_INTERVAL_US>(run_count))};

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

}  // namespace tvsc::system
