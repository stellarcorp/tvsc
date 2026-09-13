#include "iterator/ring_iterator.h"

#include <array>
#include <concepts>
#include <forward_list>
#include <iterator>
#include <list>
#include <ranges>
#include <type_traits>
#include <vector>

#include "gtest/gtest.h"

namespace tvsc::iterator {

template <typename T>
struct ContainerTraits;

template <typename T, std::size_t N>
struct ContainerTraits<std::array<T, N>> {
  static std::array<T, N> make() { return std::array<T, N>{10, 20, 30, 40, 50}; }
};

template <typename T>
struct ContainerTraits<std::vector<T>> {
  static std::vector<T> make() { return std::vector<T>{10, 20, 30, 40, 50}; }
};

template <typename T>
struct ContainerTraits<std::list<T>> {
  static std::list<T> make() { return std::list<T>{10, 20, 30, 40, 50}; }
};

template <typename T>
struct ContainerTraits<std::forward_list<T>> {
  static std::forward_list<T> make() { return std::forward_list<T>{10, 20, 30, 40, 50}; }
};

template <typename Container>
class RingIteratorTest : public ::testing::Test {
 protected:
  Container container_ = ContainerTraits<Container>::make();

  auto make_ring_at_head(std::ptrdiff_t pos = 0) {
    return RingIterator<typename Container::iterator>(std::begin(container_), std::end(container_),
                                                      pos);
  }
};

using TestContainers =
    ::testing::Types<std::array<int, 5>, std::vector<int>, std::list<int>, std::forward_list<int>>;

TYPED_TEST_SUITE(RingIteratorTest, TestContainers);

// Constrained helper function demonstrating requires-clause feature gate
template <typename RingIter>
  requires std::random_access_iterator<RingIter>
void test_random_access_indexing(RingIter it) {
  EXPECT_EQ(it[0], 10);
  EXPECT_EQ(it[1], 20);
  EXPECT_EQ(it[4], 50);
  EXPECT_EQ(it[5], 10);
  EXPECT_EQ(it[7], 30);
}

TYPED_TEST(RingIteratorTest, ForwardIterationAndWrapAround) {
  auto it = this->make_ring_at_head(0);

  EXPECT_EQ(*it, 10);
  ++it;
  EXPECT_EQ(*it, 20);
  ++it;
  EXPECT_EQ(*it, 30);
  ++it;
  EXPECT_EQ(*it, 40);
  ++it;
  EXPECT_EQ(*it, 50);

  // Wrap around back to the beginning of the underlying container
  ++it;
  EXPECT_EQ(*it, 10);
  EXPECT_EQ(it.pos(), 5);
}

TYPED_TEST(RingIteratorTest, DistanceAndEquality) {
  auto it1 = this->make_ring_at_head(0);
  auto it2 = this->make_ring_at_head(0);
  auto it3 = this->make_ring_at_head(5);

  EXPECT_EQ(it1, it2);
  EXPECT_NE(it1, it3);
  EXPECT_EQ(it3 - it1, 5);
}

TYPED_TEST(RingIteratorTest, BidirectionalOperations) {
  using UnderlyingIter = typename TypeParam::iterator;

  if constexpr (std::bidirectional_iterator<UnderlyingIter>) {
    auto it = this->make_ring_at_head(0);

    // Backward step wrapping around to end of container
    --it;
    EXPECT_EQ(*it, 50);
    EXPECT_EQ(it.pos(), -1);

    --it;
    EXPECT_EQ(*it, 40);

    ++it;
    ++it;
    EXPECT_EQ(*it, 10);
    EXPECT_EQ(it.pos(), 0);
  }
}

TYPED_TEST(RingIteratorTest, RandomAccessArithmetic) {
  using UnderlyingIter = typename TypeParam::iterator;

  if constexpr (std::random_access_iterator<UnderlyingIter>) {
    auto it = this->make_ring_at_head(0);

    // Call helper constrained via requires clause
    test_random_access_indexing(it);

    // Inline random-access arithmetic checks
    EXPECT_EQ(*(it + 3), 40);
    EXPECT_EQ(*(it + 8), 40);

    it += 7;
    EXPECT_EQ(*it, 30);
    EXPECT_EQ(it.pos(), 7);

    it -= 2;
    EXPECT_EQ(*it, 10);
    EXPECT_EQ(it.pos(), 5);

    auto ahead = it + 4;
    EXPECT_TRUE(it < ahead);
    EXPECT_TRUE(ahead > it);
  }
}

TYPED_TEST(RingIteratorTest, ConstHeterogeneousComparison) {
  auto mut_it = this->make_ring_at_head(2);

  using const_iterator = typename TypeParam::const_iterator;
  const const_iterator const_begin{std::cbegin(this->container_)};
  const const_iterator const_end{std::cend(this->container_)};

  auto const_it{RingIterator<const_iterator>(const_begin, const_end, 2)};

  EXPECT_EQ(mut_it, const_it);
  EXPECT_EQ(const_it, mut_it);

  ++mut_it;
  EXPECT_NE(mut_it, const_it);

  using UnderlyingIter = typename TypeParam::iterator;
  if constexpr (std::random_access_iterator<UnderlyingIter>) {
    EXPECT_LT(const_it, mut_it);
  }
}

TYPED_TEST(RingIteratorTest, RangeBasedForLoopIteration) {
  // Range starting at head (pos=0) and stopping after 5 elements (pos=5)
  auto ring_begin = this->make_ring_at_head(0);
  auto ring_end = this->make_ring_at_head(5);

  const std::ranges::subrange range{ring_begin, ring_end};

  std::vector<int> visited;
  for (const auto& val : range) {
    visited.push_back(val);
  }

  EXPECT_EQ(visited, (std::vector<int>{10, 20, 30, 40, 50}));
}

TYPED_TEST(RingIteratorTest, RangeBasedForLoopWithWrapAround) {
  // Start iteration midway at physical index 3 (value 40) for 5 elements
  // Monotonic pos runs from 3 to 8, wrapping physically across container bounds
  auto ring_begin = this->make_ring_at_head(3);
  auto ring_end = this->make_ring_at_head(8);

  const std::ranges::subrange range{ring_begin, ring_end};

  std::vector<int> visited;
  for (const auto& val : range) {
    visited.push_back(val);
  }

  // Validates correct physical wrapping while loop termination uses monotonic distance
  EXPECT_EQ(visited, (std::vector<int>{40, 50, 10, 20, 30}));
}

}  // namespace tvsc::iterator
