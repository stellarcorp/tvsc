#include "iterator/multi_iterator.h"

#include <algorithm>
#include <array>
#include <concepts>
#include <iterator>
#include <numeric>
#include <utility>

#include "gtest/gtest.h"

namespace tvsc::iterator {

using Array5 = std::array<int, 5>;
using ArrayIter = Array5::iterator;
using ConstArrayIter = Array5::const_iterator;
using ZipIter = MultiIterator<ArrayIter, ArrayIter>;
using ConstZipIter = MultiIterator<ConstArrayIter, ConstArrayIter>;

// ============================================================================
// C++20 Concept Verification
// ============================================================================

static_assert(std::input_or_output_iterator<ZipIter>);
static_assert(std::input_iterator<ZipIter>);
static_assert(std::forward_iterator<ZipIter>);
static_assert(std::bidirectional_iterator<ZipIter>);
static_assert(std::random_access_iterator<ZipIter>);
static_assert(std::sized_sentinel_for<ZipIter, ZipIter>);
static_assert(std::indirectly_swappable<ZipIter, ZipIter>);

TEST(MultiIteratorTest, ConceptSatisfying) {
  // Runtime sanity check that iterator tags are correctly computed
  static_assert(
      std::is_same_v<typename ZipIter::iterator_concept, std::random_access_iterator_tag>);
  static_assert(std::is_same_v<typename ZipIter::iterator_category, std::input_iterator_tag>);
}

// ============================================================================
// Basic Navigation & Arithmetic
// ============================================================================

TEST(MultiIteratorTest, IncrementDecrementAndOffset) {
  Array5 a1{10, 20, 30, 40, 50};
  Array5 a2{100, 200, 300, 400, 500};

  ZipIter begin{a1.begin(), a2.begin()};
  ZipIter end{a1.end(), a2.end()};

  // Pre-increment
  ZipIter it = begin;
  ++it;
  EXPECT_EQ((*it).first, 20);
  EXPECT_EQ((*it).second, 200);

  // Post-increment
  ZipIter prev = it++;
  EXPECT_EQ((*prev).first, 20);
  EXPECT_EQ((*it).first, 30);

  // Pre-decrement / Post-decrement
  --it;
  EXPECT_EQ((*it).first, 20);

  // Compound addition and subtraction
  it += 3;
  EXPECT_EQ((*it).first, 50);
  EXPECT_EQ((*it).second, 500);

  it -= 2;
  EXPECT_EQ((*it).first, 30);

  // Binary + / - with difference_type
  ZipIter offset_it = begin + 3;
  EXPECT_EQ((*offset_it).first, 40);

  ZipIter offset_lhs = 2 + begin;
  EXPECT_EQ((*offset_lhs).first, 30);

  ZipIter offset_back = offset_it - 1;
  EXPECT_EQ((*offset_back).first, 30);

  // Distance computation
  EXPECT_EQ(end - begin, 5);
  EXPECT_EQ(begin - end, -5);
}

TEST(MultiIteratorTest, SubscriptingAndMemberAccess) {
  Array5 a1{1, 2, 3, 4, 5};
  Array5 a2{10, 20, 30, 40, 50};

  ZipIter begin{a1.begin(), a2.begin()};

  auto [v1, v2] = begin[2];
  EXPECT_EQ(v1, 3);
  EXPECT_EQ(v2, 30);

  // Access via first() and second() underlying iterators
  EXPECT_EQ(*begin.first(), 1);
  EXPECT_EQ(*begin.second(), 10);
}

// ============================================================================
// Comparisons & Ordering
// ============================================================================

TEST(MultiIteratorTest, ComparisonOperators) {
  Array5 a1{1, 2, 3, 4, 5};
  Array5 a2{10, 20, 30, 40, 50};

  ZipIter it1{a1.begin(), a2.begin()};
  ZipIter it2{a1.begin() + 2, a2.begin() + 2};

  EXPECT_TRUE(it1 == it1);
  EXPECT_TRUE(it1 != it2);
  EXPECT_TRUE(it1 < it2);
  EXPECT_TRUE(it1 <= it2);
  EXPECT_TRUE(it2 > it1);
  EXPECT_TRUE(it2 >= it1);
}

// ============================================================================
// Proxy Reference Mutation, iter_swap, and iter_move
// ============================================================================

TEST(MultiIteratorTest, MutationViaProxyReference) {
  Array5 a1{1, 2, 3, 4, 5};
  Array5 a2{10, 20, 30, 40, 50};

  ZipIter it{a1.begin(), a2.begin()};

  // Modify underlying elements through proxy pair reference
  auto ref = *it;
  ref.first = 99;
  ref.second = 999;

  EXPECT_EQ(a1[0], 99);
  EXPECT_EQ(a2[0], 999);
}

TEST(MultiIteratorTest, IterSwapAndIterMove) {
  Array5 a1{10, 20, 30, 40, 50};
  Array5 a2{100, 200, 300, 400, 500};

  ZipIter it1{a1.begin(), a2.begin()};
  ZipIter it2{a1.begin() + 3, a2.begin() + 3};

  // Explicit ADL call to iter_swap
  std::ranges::iter_swap(it1, it2);

  EXPECT_EQ(a1[0], 40);
  EXPECT_EQ(a1[3], 10);
  EXPECT_EQ(a2[0], 400);
  EXPECT_EQ(a2[3], 100);

  // Explicit ADL call to iter_move
  auto moved_pair = std::ranges::iter_move(it1);
  EXPECT_EQ(moved_pair.first, 40);
  EXPECT_EQ(moved_pair.second, 400);
}

// ============================================================================
// std::ranges Algorithms Compatibility
// ============================================================================

TEST(MultiIteratorTest, RangesFindAndLowerBound) {
  Array5 a1{10, 20, 30, 40, 50};
  Array5 a2{1, 2, 3, 4, 5};

  ZipIter begin{a1.begin(), a2.begin()};
  ZipIter end{a1.end(), a2.end()};

  // std::ranges::find
  auto target = std::pair{30, 3};
  auto found = std::ranges::find(begin, end, target);

  ASSERT_NE(found, end);
  EXPECT_EQ(found - begin, 2);

  // std::ranges::lower_bound
  auto lb_target = std::pair{35, 0};  // Compares lexicographically: first 35 > 30
  auto lb = std::ranges::lower_bound(begin, end, lb_target);

  ASSERT_NE(lb, end);
  EXPECT_EQ((*lb).first, 40);
}

TEST(MultiIteratorTest, RangesReverse) {
  Array5 a1{1, 2, 3, 4, 5};
  Array5 a2{10, 20, 30, 40, 50};

  ZipIter begin{a1.begin(), a2.begin()};
  ZipIter end{a1.end(), a2.end()};

  std::ranges::reverse(begin, end);

  EXPECT_EQ(a1, (Array5{5, 4, 3, 2, 1}));
  EXPECT_EQ(a2, (Array5{50, 40, 30, 20, 10}));
}

TEST(MultiIteratorTest, RangesSortLockstep) {
  // Sort pairs in lockstep based on std::pair's lexicographical operator<
  Array5 a1{50, 10, 40, 20, 30};
  Array5 a2{5, 1, 4, 2, 3};

  ZipIter begin{a1.begin(), a2.begin()};
  ZipIter end{a1.end(), a2.end()};

  static_assert(std::sortable<ZipIter>);

  std::ranges::sort(begin, end);

  EXPECT_EQ(a1, (Array5{10, 20, 30, 40, 50}));
  EXPECT_EQ(a2, (Array5{1, 2, 3, 4, 5}));
}

}  // namespace tvsc::iterator
