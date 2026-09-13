#include "buffer/array_store.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>

#include "buffer/store_string.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

namespace tvsc::buffer {

// Baseline tests. These apply to all Store configurations.

template <typename T>
class StoreTest : public ::testing::Test {};

using StoreImplementations = ::testing::Types<
    ArrayStore<int, uint8_t, 2, InsertionPolicy::APPEND, OverflowPolicy::REJECT>,
    ArrayStore<int, uint8_t, 2, InsertionPolicy::SORTED, OverflowPolicy::REJECT>,
    ArrayStore<int, size_t, 4, InsertionPolicy::APPEND, OverflowPolicy::REJECT>,
    ArrayStore<int, size_t, 4, InsertionPolicy::SORTED, OverflowPolicy::REJECT>,
    ArrayStore<int, uint8_t, 2, InsertionPolicy::APPEND, OverflowPolicy::DROP_FRONT>,
    ArrayStore<int, size_t, 4, InsertionPolicy::APPEND, OverflowPolicy::DROP_FRONT>>;

TYPED_TEST_SUITE(StoreTest, StoreImplementations);

TYPED_TEST(StoreTest, SatisfiesIsStoreConcept) { static_assert(IsStore<TypeParam>); }

TYPED_TEST(StoreTest, StartsEmpty) {
  TypeParam store{};
  EXPECT_EQ(0, store.size());
  EXPECT_EQ(store.begin(), store.end());
}

TYPED_TEST(StoreTest, CapacityMatchesTemplateParameter) {
  TypeParam store{};
  EXPECT_GT(TypeParam::capacity(), 0);
  EXPECT_GE(TypeParam::capacity(), store.size());
}

TYPED_TEST(StoreTest, CanAddAndFindSingleElement) {
  TypeParam store{};
  const typename TypeParam::value_type value{42};

  const auto inserted_location{store.add(value)};

  EXPECT_NE(store.end(), inserted_location) << to_string(store);
  EXPECT_EQ(value, *inserted_location);
  EXPECT_EQ(1, store.size());
  EXPECT_NE(store.end(), std::find(store.begin(), store.end(), value));
}

TYPED_TEST(StoreTest, SizeIncrementsWithEachAdd) {
  TypeParam store{};

  for (typename TypeParam::size_type i = 0; i < TypeParam::capacity(); ++i) {
    EXPECT_EQ(i, store.size());
    store.add(static_cast<typename TypeParam::value_type>(i + 1));
  }

  EXPECT_EQ(TypeParam::capacity(), store.size());
}

TYPED_TEST(StoreTest, CanAddUpToCapacity) {
  TypeParam store{};

  for (typename TypeParam::size_type i = 0; i < TypeParam::capacity(); ++i) {
    const auto value{static_cast<typename TypeParam::value_type>(i + 1)};
    EXPECT_EQ(i, store.size());
    const auto inserted_location{store.add(value)};
    EXPECT_NE(store.end(), inserted_location) << to_string(store);
    EXPECT_EQ(value, *inserted_location);
  }

  EXPECT_EQ(TypeParam::capacity(), store.size());

  for (typename TypeParam::size_type i = 0; i < TypeParam::capacity(); ++i) {
    const auto value{static_cast<typename TypeParam::value_type>(i + 1)};
    EXPECT_NE(store.end(), std::find(store.begin(), store.end(), value));
  }
}

// InsertionPolicy::APPEND Tests

TYPED_TEST(StoreTest, PreservesInsertionOrder) {
  if constexpr (TypeParam::insertion_policy() == InsertionPolicy::APPEND) {
    static constexpr std::array<typename TypeParam::value_type, 4> input{4, 1, 3, 2};

    TypeParam store{};
    for (auto i : input) {
      if (store.size() < store.capacity()) {
        store.add(i);
      }
    }

    if (input.size() > store.capacity()) {
      EXPECT_TRUE(std::equal(store.begin(), store.end(), input.begin(),
                             std::next(input.begin(), store.capacity())));
    } else {
      EXPECT_TRUE(std::equal(store.begin(), store.end(), input.begin(), input.end()));
    }
  }
}

TYPED_TEST(StoreTest, AddReturnsIteratorToInsertionLocation) {
  if constexpr (TypeParam::insertion_policy() == InsertionPolicy::APPEND) {
    static constexpr int VALUE{10};
    TypeParam store{};
    const auto insertion_location{store.add(VALUE)};

    EXPECT_NE(store.end(), insertion_location);
    EXPECT_EQ(VALUE, *insertion_location);
  }
}

// InsertionPolicy::SORTED Tests

TEST(ArrayStoreSortedTest, MaintainsSortedOrderOnRandomInserts) {
  using Store = ArrayStore<int, size_t, 4, InsertionPolicy::SORTED, OverflowPolicy::REJECT>;
  static constexpr std::array<int, 4> input = {4, 1, 3, 2};
  static constexpr std::array<int, 4> expected_sorted = {1, 2, 3, 4};

  Store store{};
  for (int val : input) {
    store.add(val);
    EXPECT_TRUE(std::ranges::is_sorted(store.begin(), store.end()));
  }

  EXPECT_TRUE(
      std::equal(store.begin(), store.end(), expected_sorted.begin(), expected_sorted.end()));
}

TEST(ArrayStoreSortedTest, AddReturnsIteratorToInsertedPosition) {
  using Store = ArrayStore<int, size_t, 6, InsertionPolicy::SORTED, OverflowPolicy::REJECT>;

  Store store{};
  store.add(10);
  store.add(30);
  EXPECT_TRUE(std::ranges::is_sorted(store.begin(), store.end()));

  {
    // Store contains [10, 30]
    const auto inserted_location{store.add(20)};

    EXPECT_EQ(20, *inserted_location);
    EXPECT_TRUE(std::ranges::is_sorted(store.begin(), store.end()));
  }

  {
    // Store contains [10, 20, 30]
    const auto inserted_location{store.add(40)};

    EXPECT_EQ(40, *inserted_location);
    EXPECT_TRUE(std::ranges::is_sorted(store.begin(), store.end()));
  }

  {
    // Store contains [10, 20, 30, 40]
    const auto inserted_location{store.add(0)};

    EXPECT_EQ(0, *inserted_location);
    EXPECT_TRUE(std::ranges::is_sorted(store.begin(), store.end()));
  }

  {
    // Store contains [0, 10, 20, 30, 40]
    const auto inserted_location{store.add(25)};

    EXPECT_EQ(25, *inserted_location);
    EXPECT_TRUE(std::ranges::is_sorted(store.begin(), store.end()));
  }
}

// OverflowPolicy::REJECT Tests

TEST(ArrayStoreOverflowRejectTest, RejectsElementsWhenFull) {
  using Store = ArrayStore<int, size_t, 4, InsertionPolicy::APPEND, OverflowPolicy::REJECT>;
  static constexpr std::array<int, 4> input = {1, 2, 3, 4};
  static constexpr int REJECTED_VALUE = 99;

  Store store{};
  for (int val : input) {
    store.add(val);
  }

  EXPECT_EQ(Store::capacity(), store.size());

  const auto insertion_point = store.add(REJECTED_VALUE);

  EXPECT_EQ(store.end(), insertion_point);
  EXPECT_EQ(Store::capacity(), store.size());
  EXPECT_EQ(store.end(), std::find(store.begin(), store.end(), REJECTED_VALUE));
}

}  // namespace tvsc::buffer
