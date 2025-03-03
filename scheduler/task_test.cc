#include "scheduler/task.h"

#include "gtest/gtest.h"
#include "scheduler/sample_tasks.h"

namespace tvsc::scheduler {

using ClockType = tvsc::hal::time::MockClock;
using TaskType = Task<ClockType>;

TEST(TaskTest, DefaultTaskIsInvalid) { EXPECT_FALSE(TaskType{}.is_valid()); }

TEST(TaskTest, CoroutinesCreateValidTasks) { EXPECT_TRUE(just_return<ClockType>().is_valid()); }

TEST(TaskTest, CanMakeCollectionOfTasks) {
  std::vector<TaskType> tasks{};
  int run_count{};
  tasks.emplace_back(run_forever<ClockType>(run_count));
  EXPECT_EQ(1, tasks.size());
}

TEST(TaskTest, CanDetectInvalidTasksInCollection) {
  std::array<TaskType, 3> tasks{};
  int run_count{};
  tasks[1] = run_forever<ClockType>(run_count);
  EXPECT_TRUE(tasks[1].is_valid());
  EXPECT_FALSE(tasks[0].is_valid());
  EXPECT_FALSE(tasks[2].is_valid());
}

}  // namespace tvsc::scheduler
