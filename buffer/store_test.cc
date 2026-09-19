#include "buffer/store.h"

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

// Helper templates that make the different Store specializations in the Types below somewhat easier
// to read.

template <typename Value, size_t CAPACITY, InsertionPolicy INSERTION_POLICY,
          OverflowPolicy OVERFLOW_POLICY>
  requires std::default_initializable<Value>
using ArrayStore = internal::Store<Value, size_t, CAPACITY, CAPACITY, INSERTION_POLICY,
                                   OVERFLOW_POLICY, std::array<Value, CAPACITY>>;

template <typename Value, size_t MIN_CAPACITY, size_t MAX_CAPACITY,
          InsertionPolicy INSERTION_POLICY, OverflowPolicy OVERFLOW_POLICY>
using VectorStore = internal::Store<Value, size_t, MIN_CAPACITY, MAX_CAPACITY, INSERTION_POLICY,
                                    OVERFLOW_POLICY, std::vector<Value>>;

using StoreImplementations =
    ::testing::Types<ArrayStore<int, 256, InsertionPolicy::APPEND, OverflowPolicy::REJECT>,
                     ArrayStore<int, 2, InsertionPolicy::APPEND, OverflowPolicy::REJECT>,
                     ArrayStore<int, 2, InsertionPolicy::SORTED, OverflowPolicy::REJECT>,
                     ArrayStore<int, 8, InsertionPolicy::APPEND, OverflowPolicy::REJECT>,
                     ArrayStore<int, 8, InsertionPolicy::SORTED, OverflowPolicy::REJECT>,
                     ArrayStore<int, 2, InsertionPolicy::APPEND, OverflowPolicy::DROP_OLDEST>,
                     ArrayStore<int, 2, InsertionPolicy::SORTED, OverflowPolicy::DROP_OLDEST>,
                     ArrayStore<int, 8, InsertionPolicy::APPEND, OverflowPolicy::DROP_OLDEST>,
                     ArrayStore<int, 8, InsertionPolicy::SORTED, OverflowPolicy::DROP_OLDEST>,

                     VectorStore<int, 128, 256, InsertionPolicy::APPEND, OverflowPolicy::REJECT>,
                     VectorStore<int, 2, 16, InsertionPolicy::APPEND, OverflowPolicy::REJECT>,
                     VectorStore<int, 2, 16, InsertionPolicy::SORTED, OverflowPolicy::REJECT>,
                     VectorStore<int, 8, 16, InsertionPolicy::APPEND, OverflowPolicy::REJECT>,
                     VectorStore<int, 8, 16, InsertionPolicy::SORTED, OverflowPolicy::REJECT>,
                     VectorStore<int, 2, 16, InsertionPolicy::APPEND, OverflowPolicy::DROP_OLDEST>,
                     VectorStore<int, 2, 16, InsertionPolicy::SORTED, OverflowPolicy::DROP_OLDEST>,
                     VectorStore<int, 8, 16, InsertionPolicy::APPEND, OverflowPolicy::DROP_OLDEST>,
                     VectorStore<int, 8, 16, InsertionPolicy::SORTED, OverflowPolicy::DROP_OLDEST>>;

TYPED_TEST_SUITE(StoreTest, StoreImplementations);

TYPED_TEST(StoreTest, SatisfiesIsStoreConcept) { static_assert(IsStore<TypeParam>); }

TYPED_TEST(StoreTest, StartsEmpty) {
  TypeParam store{};
  EXPECT_EQ(0, store.size());
  EXPECT_EQ(store.begin(), store.end());
}

TYPED_TEST(StoreTest, CapacityMatchesTemplateParameter) {
  TypeParam store{};
  const typename TypeParam::value_type value{42};
  // Note that before a value is added to the store, the store's capacity might be zero.
  store.insert(value);
  EXPECT_EQ(1, store.size());
  EXPECT_GT(store.capacity(), 0);
  EXPECT_GT(store.min_capacity(), 0);
  EXPECT_GE(store.max_capacity(), store.min_capacity());
}

TYPED_TEST(StoreTest, CanInsertAndFindSingleElement) {
  TypeParam store{};
  const typename TypeParam::value_type value{42};

  const auto inserted_location{store.insert(value)};

  EXPECT_NE(store.end(), inserted_location) << to_string(store);
  EXPECT_EQ(value, *inserted_location);
  EXPECT_EQ(1, store.size());
  EXPECT_NE(store.end(), std::find(store.begin(), store.end(), value));
}

TYPED_TEST(StoreTest, CanEraseSingleElementByIterator) {
  TypeParam store{};
  const typename TypeParam::value_type value{42};

  const auto inserted_location{store.insert(value)};
  const auto erased_location{store.erase(inserted_location)};

  EXPECT_TRUE(store.empty());
  EXPECT_EQ(store.end(), erased_location)
      << "begin: " << to_string(store.begin()) << ", end: " << to_string(store.end())
      << ", erased_location: " << to_string(erased_location);
}

TYPED_TEST(StoreTest, SizeIncrementsWithEachInsert) {
  TypeParam store{};

  for (typename TypeParam::size_type i = 0; i < store.capacity(); ++i) {
    EXPECT_EQ(i, store.size());
    store.insert(static_cast<typename TypeParam::value_type>(i + 1));
  }

  EXPECT_EQ(store.capacity(), store.size());
}

TYPED_TEST(StoreTest, CanInsertUpToCapacity) {
  TypeParam store{};

  for (typename TypeParam::size_type i = 0; i < store.capacity(); ++i) {
    const auto value{static_cast<typename TypeParam::value_type>(i + 1)};
    EXPECT_EQ(i, store.size());
    const auto inserted_location{store.insert(value)};
    EXPECT_NE(store.end(), inserted_location) << to_string(store);
    EXPECT_EQ(value, *inserted_location);
  }

  EXPECT_EQ(store.capacity(), store.size());

  for (typename TypeParam::size_type i = 0; i < store.capacity(); ++i) {
    const auto value{static_cast<typename TypeParam::value_type>(i + 1)};
    EXPECT_NE(store.end(), std::find(store.begin(), store.end(), value));
  }
}

// InsertionPolicy::APPEND Tests

TYPED_TEST(StoreTest, AppendPreservesInsertionOrder) {
  if constexpr (TypeParam::insertion_policy() == InsertionPolicy::APPEND) {
    static constexpr std::array<typename TypeParam::value_type, 4> input{4, 1, 3, 2};

    TypeParam store{};
    for (auto i : input) {
      if (store.size() < store.max_capacity()) {
        store.insert(i);
      }
    }

    if (input.size() > store.capacity()) {
      EXPECT_TRUE(std::equal(store.begin(), store.end(), input.begin(),
                             std::next(input.begin(), store.capacity())))
          << to_string(store);
    } else {
      EXPECT_TRUE(std::equal(store.begin(), store.end(), input.begin(), input.end()))
          << to_string(store);
    }
  }
}

TYPED_TEST(StoreTest, AppendInsertReturnsIteratorToInsertionLocation) {
  if constexpr (TypeParam::insertion_policy() == InsertionPolicy::APPEND) {
    static constexpr int VALUE{10};
    TypeParam store{};
    const auto insertion_location{store.insert(VALUE)};

    EXPECT_NE(store.end(), insertion_location);
    EXPECT_EQ(VALUE, *insertion_location);
  }
}

// InsertionPolicy::SORTED Tests

TYPED_TEST(StoreTest, MaintainsSortedOrderOnRandomInserts) {
  using Store = TypeParam;
  if constexpr (Store::insertion_policy() == InsertionPolicy::SORTED) {
    static constexpr std::array<typename Store::value_type, 4> input{4, 1, 3, 2};
    static constexpr std::array<typename Store::value_type, 4> expected_sorted{1, 2, 3, 4};

    Store store{};
    for (int val : input) {
      if (store.size() < store.max_capacity()) {
        const auto insertion_location{store.insert(val)};
        EXPECT_EQ(val, *insertion_location);
        EXPECT_TRUE(std::ranges::is_sorted(store.begin(), store.end()))
            << "size: " << (int)store.size() << ", begin_index: " << (int)store.begin_index()
            << ", end_index: " << (int)store.end_index() << ", store: " << to_string(store);
      }
    }

    if (input.size() <= store.capacity()) {
      EXPECT_TRUE(
          std::equal(store.begin(), store.end(), expected_sorted.begin(), expected_sorted.end()))
          << to_string(store);
    }
  }
}

TYPED_TEST(StoreTest, InsertReturnsIteratorToInsertedPosition) {
  using Store = TypeParam;
  if constexpr (Store::insertion_policy() == InsertionPolicy::SORTED and
                Store::max_capacity() >= 6) {
    Store store{};
    store.insert(10);
    store.insert(30);
    EXPECT_TRUE(std::ranges::is_sorted(store.begin(), store.end()));

    {
      // Store contains [10, 30]
      const auto inserted_location{store.insert(20)};

      EXPECT_EQ(20, *inserted_location);
      EXPECT_TRUE(std::ranges::is_sorted(store.begin(), store.end()));
    }

    {
      // Store contains [10, 20, 30]
      const auto inserted_location{store.insert(40)};

      EXPECT_EQ(40, *inserted_location);
      EXPECT_TRUE(std::ranges::is_sorted(store.begin(), store.end()));
    }

    {
      // Store contains [10, 20, 30, 40]
      const auto inserted_location{store.insert(0)};

      EXPECT_EQ(0, *inserted_location);
      EXPECT_TRUE(std::ranges::is_sorted(store.begin(), store.end()));
    }

    {
      // Store contains [0, 10, 20, 30, 40]
      const auto inserted_location{store.insert(25)};

      EXPECT_EQ(25, *inserted_location);
      EXPECT_TRUE(std::ranges::is_sorted(store.begin(), store.end()));
    }
  }
}

// OverflowPolicy::REJECT Tests

TYPED_TEST(StoreTest, RejectsElementsWhenFull) {
  using Store = TypeParam;
  if constexpr (Store::overflow_policy() == OverflowPolicy::REJECT) {
    static constexpr std::array<int, 16> input{1, 2,  3,  4,  5,  6,  7,  8,
                                               9, 10, 11, 12, 13, 14, 15, 16};
    static constexpr int REJECTED_VALUE{99};

    Store store{};
    for (int val : input) {
      if (store.size() < store.max_capacity()) {
        store.insert(val);
      }
    }

    if (store.size() == store.max_capacity()) {
      const auto insertion_point = store.insert(REJECTED_VALUE);

      EXPECT_EQ(store.end(), insertion_point);
      EXPECT_EQ(store.max_capacity(), store.size());
      EXPECT_EQ(store.end(), std::find(store.begin(), store.end(), REJECTED_VALUE));
    }
  }
}

}  // namespace tvsc::buffer
