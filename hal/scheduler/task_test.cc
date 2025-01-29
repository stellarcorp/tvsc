#include "hal/scheduler/task.h"

#include "gtest/gtest.h"
#include "hal/scheduler/sample_tasks.h"

namespace tvsc::hal::scheduler {

TEST(TaskTest, DefaultTaskIsInvalid) { EXPECT_FALSE(Task{}.is_valid()); }

TEST(TaskTest, CoroutinesCreateValidTasks) { EXPECT_TRUE(just_return().is_valid()); }

TEST(TaskTest, CanMakeCollectionOfTasks) {
  std::vector<Task> tasks{};
  int run_count{};
  tasks.emplace_back(run_forever(run_count));
  EXPECT_EQ(1, tasks.size());
}

TEST(TaskTest, CanDetectInvalidTasksInCollection) {
  std::array<Task, 3> tasks{};
  int run_count{};
  tasks[1] = run_forever(run_count);
  EXPECT_TRUE(tasks[1].is_valid());
  EXPECT_FALSE(tasks[0].is_valid());
  EXPECT_FALSE(tasks[2].is_valid());
}

}  // namespace tvsc::hal::scheduler
