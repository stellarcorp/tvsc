#pragma once

#include <array>
#include <concepts>
#include <functional>
#include <iterator>
#include <list>
#include <memory>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

namespace tvsc::iterator {

// Primary template declaration for RingIterator
template <std::forward_iterator Iterator>
class RingIterator;

// =================================================================================================
// Specialization for forward & bidirectional iterators.
// Works with std::unordered_set, std::unordered_map, std::hive, and similar containers that do not
// provide random access to elements.
// =================================================================================================
template <std::forward_iterator Iterator>
class RingIterator final {
 public:
  using iterator_category =
      std::conditional_t<std::bidirectional_iterator<Iterator>, std::bidirectional_iterator_tag,
                         std::forward_iterator_tag>;
  using iterator_concept = iterator_category;

  using value_type = std::iter_value_t<Iterator>;
  using difference_type = std::iter_difference_t<Iterator>;
  using pointer = std::add_pointer_t<std::iter_reference_t<Iterator>>;
  using reference = std::iter_reference_t<Iterator>;

 private:
  Iterator base_begin_{};
  Iterator base_end_{};
  Iterator curr_{};
  difference_type pos_{0};

 public:
  constexpr RingIterator() noexcept = default;

  constexpr RingIterator(Iterator base_begin, Iterator base_end, difference_type pos = 0)
      : base_begin_(base_begin), base_end_(base_end), pos_(pos) {
    if (base_begin_ != base_end_) {
      difference_type capacity{std::distance(base_begin_, base_end_)};
      difference_type offset{pos_ % capacity};
      if (offset < 0) {
        offset += capacity;
      }
      curr_ = std::next(base_begin_, offset);
    } else {
      curr_ = base_begin_;
    }
  }

  // Converting constructor (e.g., iterator -> const_iterator)
  template <typename OtherIt>
    requires(!std::same_as<Iterator, OtherIt> && std::convertible_to<OtherIt, Iterator>)
  constexpr RingIterator(const RingIterator<OtherIt>& other) noexcept
      : base_begin_(other.base_begin()),
        base_end_(other.base_end()),
        curr_(other.curr()),
        pos_(other.pos()) {}

  [[nodiscard]] constexpr Iterator base_begin() const noexcept { return base_begin_; }
  [[nodiscard]] constexpr Iterator base_end() const noexcept { return base_end_; }
  [[nodiscard]] constexpr Iterator curr() const noexcept { return curr_; }
  [[nodiscard]] constexpr difference_type pos() const noexcept { return pos_; }

  [[nodiscard]] constexpr reference operator*() const noexcept { return *curr_; }
  [[nodiscard]] constexpr pointer operator->() const noexcept { return std::addressof(*curr_); }

  constexpr RingIterator& operator++() noexcept {
    ++curr_;
    if (curr_ == base_end_) {
      curr_ = base_begin_;
    }
    ++pos_;
    return *this;
  }

  [[nodiscard]] constexpr RingIterator operator++(int) noexcept {
    auto tmp{*this};
    ++(*this);
    return tmp;
  }

  constexpr RingIterator& operator--() noexcept
    requires std::bidirectional_iterator<Iterator>
  {
    if (curr_ == base_begin_) {
      curr_ = base_end_;
    }
    --curr_;
    --pos_;
    return *this;
  }

  [[nodiscard]] constexpr RingIterator operator--(int) noexcept
    requires std::bidirectional_iterator<Iterator>
  {
    auto tmp{*this};
    --(*this);
    return tmp;
  }

  constexpr friend bool operator==(const RingIterator& lhs, const RingIterator& rhs) noexcept {
    return lhs.pos_ == rhs.pos_;
  }

  constexpr friend difference_type operator-(const RingIterator& lhs,
                                             const RingIterator& rhs) noexcept {
    return lhs.pos_ - rhs.pos_;
  }
};

// =================================================================================================
// Specialization for random-access iterators.
// Works with std::array, std::vector, std::deque, and other containers that allow random access to
// their elements.
// =================================================================================================
template <std::random_access_iterator Iterator>
class RingIterator<Iterator> final {
 public:
  using iterator_category = std::random_access_iterator_tag;
  using iterator_concept = std::random_access_iterator_tag;

  using value_type = std::iter_value_t<Iterator>;
  using difference_type = std::iter_difference_t<Iterator>;
  using pointer = std::add_pointer_t<std::iter_reference_t<Iterator>>;
  using reference = std::iter_reference_t<Iterator>;

 private:
  Iterator base_begin_{};
  difference_type capacity_{0};
  difference_type pos_{0};

 public:
  constexpr RingIterator() noexcept = default;

  constexpr RingIterator(Iterator base_begin, difference_type capacity,
                         difference_type pos = 0) noexcept
      : base_begin_(base_begin), capacity_(capacity), pos_(pos) {}

  constexpr RingIterator(Iterator base_begin, Iterator base_end, difference_type pos = 0) noexcept
      : base_begin_(base_begin), capacity_(std::distance(base_begin, base_end)), pos_(pos) {}

  // Converting constructor (e.g., iterator -> const_iterator)
  template <typename OtherIt>
    requires(!std::same_as<Iterator, OtherIt> && std::convertible_to<OtherIt, Iterator>)
  constexpr RingIterator(const RingIterator<OtherIt>& other) noexcept
      : base_begin_(other.base_begin()), capacity_(other.capacity()), pos_(other.pos()) {}

  [[nodiscard]] constexpr Iterator base_begin() const noexcept { return base_begin_; }
  [[nodiscard]] constexpr difference_type capacity() const noexcept { return capacity_; }
  [[nodiscard]] constexpr difference_type pos() const noexcept { return pos_; }

  [[nodiscard]] constexpr reference operator*() const noexcept {
    difference_type offset{pos_ % capacity_};
    if (offset < 0) {
      offset += capacity_;
    }
    return *(base_begin_ + offset);
  }

  [[nodiscard]] constexpr pointer operator->() const noexcept { return std::addressof(*(*this)); }

  constexpr RingIterator& operator++() noexcept {
    ++pos_;
    return *this;
  }

  [[nodiscard]] constexpr RingIterator operator++(int) noexcept {
    auto tmp{*this};
    ++pos_;
    return tmp;
  }

  constexpr RingIterator& operator--() noexcept {
    --pos_;
    return *this;
  }

  [[nodiscard]] constexpr RingIterator operator--(int) noexcept {
    auto tmp{*this};
    --pos_;
    return tmp;
  }

  constexpr RingIterator& operator+=(difference_type n) noexcept {
    pos_ += n;
    return *this;
  }

  constexpr RingIterator& operator-=(difference_type n) noexcept {
    pos_ -= n;
    return *this;
  }

  constexpr friend RingIterator operator+(RingIterator it, difference_type n) noexcept {
    it += n;
    return it;
  }

  constexpr friend RingIterator operator+(difference_type n, RingIterator it) noexcept {
    it += n;
    return it;
  }

  constexpr friend RingIterator operator-(RingIterator it, difference_type n) noexcept {
    it -= n;
    return it;
  }

  constexpr friend difference_type operator-(const RingIterator& lhs,
                                             const RingIterator& rhs) noexcept {
    return lhs.pos_ - rhs.pos_;
  }

  constexpr reference operator[](difference_type n) const noexcept { return *(*this + n); }

  constexpr friend auto operator<=>(const RingIterator& lhs, const RingIterator& rhs) noexcept {
    return lhs.pos_ <=> rhs.pos_;
  }

  constexpr friend bool operator==(const RingIterator& lhs, const RingIterator& rhs) noexcept {
    return lhs.pos_ == rhs.pos_;
  }
};

namespace concept_checks {

// Random-access mutable iterator (std::array)
using MutableArrIt = std::array<int, 10>::iterator;
using RingArrIter = RingIterator<MutableArrIt>;
using Val = typename RingArrIter::value_type;
using Ref = typename RingArrIter::reference;

static_assert(std::input_or_output_iterator<RingArrIter>);
static_assert(std::input_iterator<RingArrIter>);
static_assert(std::forward_iterator<RingArrIter>);
static_assert(std::bidirectional_iterator<RingArrIter>);
static_assert(std::random_access_iterator<RingArrIter>);

static_assert(std::sentinel_for<RingArrIter, RingArrIter>);
static_assert(std::sized_sentinel_for<RingArrIter, RingArrIter>);
static_assert(std::equality_comparable<RingArrIter>);
static_assert(std::totally_ordered<RingArrIter>);

static_assert(std::indirectly_readable<RingArrIter>);
static_assert(std::indirectly_writable<RingArrIter, Val>);
static_assert(std::indirectly_swappable<RingArrIter, RingArrIter>);
static_assert(std::permutable<RingArrIter>);
static_assert(std::indirect_strict_weak_order<std::ranges::less, RingArrIter>);

// Random-access const iterator and heterogeneous interoperability
using ConstArrIt = std::array<int, 10>::const_iterator;
using ConstRingArrIter = RingIterator<ConstArrIt>;

static_assert(std::input_iterator<ConstRingArrIter>);
static_assert(std::forward_iterator<ConstRingArrIter>);
static_assert(std::bidirectional_iterator<ConstRingArrIter>);
static_assert(std::random_access_iterator<ConstRingArrIter>);

static_assert(std::convertible_to<RingArrIter, ConstRingArrIter>);

static_assert(std::sentinel_for<RingArrIter, ConstRingArrIter>);
static_assert(std::sentinel_for<ConstRingArrIter, RingArrIter>);
static_assert(std::sized_sentinel_for<RingArrIter, ConstRingArrIter>);
static_assert(std::sized_sentinel_for<ConstRingArrIter, RingArrIter>);
static_assert(std::equality_comparable_with<RingArrIter, ConstRingArrIter>);
static_assert(std::totally_ordered_with<RingArrIter, ConstRingArrIter>);

// Dynamic random-access iterator (std::vector)
using VecIter = std::vector<int>::iterator;
using RingVecIter = RingIterator<VecIter>;

static_assert(std::random_access_iterator<RingVecIter>);
static_assert(std::sized_sentinel_for<RingVecIter, RingVecIter>);
static_assert(std::indirectly_swappable<RingVecIter, RingVecIter>);

// Bidirectional iterator (std::list)
using ListIter = std::list<int>::iterator;
using RingListIter = RingIterator<ListIter>;

static_assert(std::input_iterator<RingListIter>);
static_assert(std::forward_iterator<RingListIter>);
static_assert(std::bidirectional_iterator<RingListIter>);
static_assert(!std::random_access_iterator<RingListIter>);

static_assert(std::sentinel_for<RingListIter, RingListIter>);
static_assert(std::sized_sentinel_for<RingListIter, RingListIter>);
static_assert(std::indirectly_readable<RingListIter>);
static_assert(std::indirectly_writable<RingListIter, int>);

// Forward iterator (std::unordered_set)
using SetIter = std::unordered_set<int>::iterator;
using RingSetIter = RingIterator<SetIter>;

static_assert(std::input_iterator<RingSetIter>);
static_assert(std::forward_iterator<RingSetIter>);
static_assert(!std::bidirectional_iterator<RingSetIter>);
static_assert(!std::random_access_iterator<RingSetIter>);

static_assert(std::sentinel_for<RingSetIter, RingSetIter>);
static_assert(std::sized_sentinel_for<RingSetIter, RingSetIter>);
static_assert(std::indirectly_readable<RingSetIter>);

}  // namespace concept_checks

}  // namespace tvsc::iterator
