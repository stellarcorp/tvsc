/**
 * Composite iterator, akin to a zip iterator, and associated proxy reference handle for lockstep
 * range traversal.
 *
 * This header defines `MultiIterator` and its associated proxy reference type `MultiReference`.
 * Together, they allow two distinct iterators to be used in parallel as a single, logical range.
 *
 * Key Components:
 * ---------------
 * - MultiReference<FirstRef, SecondRef>:
 *     A proxy reference wrapper holding reference handles (`FirstRef`, `SecondRef`) to the
 *     underlying elements of both iterators.
 *     - Provides non-template `const` copy and move assignment operators to override
 * compiler-deleted defaults, enabling proxy mutation during reordering operations.
 *     - Implements explicit value-swapping semantics (member `swap` and ADL `swap` friend) that
 * mutate the underlying referenced elements in lockstep.
 *     - Supports conversions, comparisons (`==`, `<=>`), and assignments to/from `std::pair`.
 *
 * - MultiIterator<Iter1, Iter2>:
 *     A C++20-compatible composite iterator that wraps two underlying iterators and advances them
 *     simultaneously.
 *     - Dereferences into a `MultiReference` proxy.
 *     - Satisfies C++20 iterator concepts allowing `MultiIterator` ranges to be used with ranges
 * algorithms such as `std::ranges::sort`, `std::ranges::reverse`, and `std::ranges::find`.
 */
#pragma once

#include <concepts>
#include <iterator>
#include <utility>

namespace tvsc::iterator {

// Class to bridge between std::ranges algorithms for proxy classes like MultiIterator below and
// C++20 standard library types.
template <typename FirstRef, typename SecondRef>
struct MultiReference {
  FirstRef first;
  SecondRef second;

  template <typename T1, typename T2>
  constexpr operator std::pair<T1, T2>() const
      noexcept(noexcept(T1(first)) && noexcept(T2(second))) {
    return {first, second};
  }

  // Non-template copy & move assignment, replaces the deleted implicit operators.
  constexpr const MultiReference& operator=(const MultiReference& rhs) const noexcept {
    first = rhs.first;
    second = rhs.second;
    return *this;
  }

  constexpr const MultiReference& operator=(MultiReference&& rhs) const noexcept {
    first = rhs.first;
    second = rhs.second;
    return *this;
  }

  template <typename T1, typename T2>
  constexpr const MultiReference& operator=(const std::pair<T1, T2>& p) const noexcept {
    first = p.first;
    second = p.second;
    return *this;
  }

  template <typename T1, typename T2>
  constexpr const MultiReference& operator=(std::pair<T1, T2>&& p) const noexcept {
    first = std::move(p.first);
    second = std::move(p.second);
    return *this;
  }

  // Templated assignment for heterogenous MultiReference types
  template <typename R1, typename R2>
  constexpr const MultiReference& operator=(const MultiReference<R1, R2>& rhs) const noexcept {
    first = rhs.first;
    second = rhs.second;
    return *this;
  }

  constexpr void swap(const MultiReference& other) const
      noexcept(noexcept(std::ranges::swap(first, other.first)) &&
               noexcept(std::ranges::swap(second, other.second))) {
    std::ranges::swap(first, other.first);
    std::ranges::swap(second, other.second);
  }

  friend constexpr void swap(const MultiReference& lhs,
                             const MultiReference& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
  }

  // Equality comparison
  template <typename U1, typename U2>
  constexpr bool operator==(const std::pair<U1, U2>& rhs) const noexcept {
    return first == rhs.first and second == rhs.second;
  }

  template <typename R1, typename R2>
  constexpr bool operator==(const MultiReference<R1, R2>& rhs) const noexcept {
    return first == rhs.first and second == rhs.second;
  }

  // Three-way comparison
  template <typename U1, typename U2>
  constexpr auto operator<=>(const std::pair<U1, U2>& rhs) const noexcept {
    return std::tie(first, second) <=> std::tie(rhs.first, rhs.second);
  }

  template <typename R1, typename R2>
  constexpr auto operator<=>(const MultiReference<R1, R2>& rhs) const noexcept {
    return std::tie(first, second) <=> std::tie(rhs.first, rhs.second);
  }
};

template <typename FirstT, typename SecondT>
  requires std::input_or_output_iterator<FirstT> and std::input_or_output_iterator<SecondT>
class MultiIterator final {
 public:
  using First = FirstT;
  using Second = SecondT;

 private:
  static consteval auto compute_iterator_concept() {
    if constexpr (std::random_access_iterator<First> && std::random_access_iterator<Second>) {
      return std::random_access_iterator_tag{};
    } else if constexpr (std::bidirectional_iterator<First> &&
                         std::bidirectional_iterator<Second>) {
      return std::bidirectional_iterator_tag{};
    } else if constexpr (std::forward_iterator<First> && std::forward_iterator<Second>) {
      return std::forward_iterator_tag{};
    } else {
      return std::input_iterator_tag{};
    }
  }

 public:
  using value_type = std::pair<std::iter_value_t<First>, std::iter_value_t<Second>>;
  using reference = MultiReference<std::iter_reference_t<First>, std::iter_reference_t<Second>>;
  using difference_type =
      std::common_type_t<std::iter_difference_t<First>, std::iter_difference_t<Second>>;
  using pointer = void;

  using iterator_concept = decltype(compute_iterator_concept());

  // Note: must use ranges to get access to most algorithms. std::sort(), std::lower_bound(), etc.,
  // will not work as they require actual lvalue references which this class cannot provide.
  using iterator_category = std::input_iterator_tag;

 private:
  First first_{};
  Second second_{};

 public:
  constexpr MultiIterator() noexcept = default;
  constexpr MultiIterator(First first, Second second) noexcept
      : first_(std::move(first)), second_(std::move(second)) {}
  constexpr MultiIterator(const MultiIterator& rhs) noexcept = default;

  constexpr MultiIterator& operator=(const MultiIterator& rhs) noexcept = default;

  constexpr auto operator<=>(const MultiIterator& rhs) const noexcept = default;

  constexpr MultiIterator& operator++() noexcept {
    ++first_;
    ++second_;
    return *this;
  }
  constexpr MultiIterator operator++(int) noexcept { return {first_++, second_++}; }

  constexpr MultiIterator& operator--() noexcept
    requires std::bidirectional_iterator<First> and std::bidirectional_iterator<Second>
  {
    --first_;
    --second_;
    return *this;
  }
  constexpr MultiIterator operator--(int) noexcept
    requires std::bidirectional_iterator<First> and std::bidirectional_iterator<Second>
  {
    return {first_--, second_--};
  }

  constexpr MultiIterator& operator+=(difference_type n) noexcept
    requires std::random_access_iterator<First> and std::random_access_iterator<Second>
  {
    first_ += n;
    second_ += n;
    return *this;
  }
  constexpr MultiIterator& operator-=(difference_type n) noexcept
    requires std::random_access_iterator<First> and std::random_access_iterator<Second>
  {
    first_ -= n;
    second_ -= n;
    return *this;
  }

  constexpr reference operator*() noexcept { return {*first_, *second_}; }
  constexpr reference operator*() const noexcept { return {*first_, *second_}; }
  constexpr reference operator[](difference_type n) noexcept
    requires std::random_access_iterator<First> and std::random_access_iterator<Second>
  {
    return {first_[n], second_[n]};
  }
  constexpr reference operator[](difference_type n) const noexcept
    requires std::random_access_iterator<First> and std::random_access_iterator<Second>
  {
    return {first_[n], second_[n]};
  }

  constexpr auto first() noexcept { return first_; }
  constexpr auto first() const noexcept { return first_; }
  constexpr auto second() noexcept { return second_; }
  constexpr auto second() const noexcept { return second_; }

  [[nodiscard]] friend constexpr MultiIterator operator+(const MultiIterator& iter,
                                                         difference_type n) noexcept
    requires std::random_access_iterator<First> and std::random_access_iterator<Second>
  {
    return {iter.first_ + n, iter.second_ + n};
  }

  [[nodiscard]] friend constexpr MultiIterator operator+(difference_type n,
                                                         const MultiIterator& iter) noexcept
    requires std::random_access_iterator<First> and std::random_access_iterator<Second>
  {
    return {iter.first_ + n, iter.second_ + n};
  }

  [[nodiscard]] friend constexpr MultiIterator operator-(const MultiIterator& iter,
                                                         difference_type n) noexcept
    requires std::random_access_iterator<First> and std::random_access_iterator<Second>
  {
    return {iter.first_ - n, iter.second_ - n};
  }

  [[nodiscard]] friend constexpr difference_type operator-(const MultiIterator& lhs,
                                                           const MultiIterator& rhs) noexcept
    requires std::random_access_iterator<First>
  {
    return lhs.first_ - rhs.first_;
  }

  friend constexpr auto iter_move(const MultiIterator& iter) noexcept {
    return std::pair<std::iter_rvalue_reference_t<First>, std::iter_rvalue_reference_t<Second>>(
        std::ranges::iter_move(iter.first_), std::ranges::iter_move(iter.second_));
  }

  friend constexpr void iter_swap(const MultiIterator& lhs, const MultiIterator& rhs) noexcept {
    std::ranges::iter_swap(lhs.first_, rhs.first_);
    std::ranges::iter_swap(lhs.second_, rhs.second_);
  }
};

template <typename First, typename Second>
MultiIterator(First, Second) -> MultiIterator<First, Second>;

}  // namespace tvsc::iterator

namespace std {

template <typename Ref1, typename Ref2, typename Val1, typename Val2, template <class> class XQual,
          template <class> class YQual>
struct basic_common_reference<tvsc::iterator::MultiReference<Ref1, Ref2>, std::pair<Val1, Val2>,
                              XQual, YQual> {
  using type = std::pair<Val1, Val2>;
};

template <typename Val1, typename Val2, typename Ref1, typename Ref2, template <class> class XQual,
          template <class> class YQual>
struct basic_common_reference<std::pair<Val1, Val2>, tvsc::iterator::MultiReference<Ref1, Ref2>,
                              XQual, YQual> {
  using type = std::pair<Val1, Val2>;
};

}  // namespace std

namespace tvsc::iterator::concept_checks {

using ZipIter = MultiIterator<int*, int*>;
using Ref = typename ZipIter::reference;
using Val = typename ZipIter::value_type;

// Proxy reference requirements
static_assert(std::convertible_to<Ref, Val>);
static_assert(std::common_reference_with<Ref&&, Val&>);
static_assert(std::assignable_from<const Ref&, Val>);
static_assert(std::assignable_from<const Ref&, const Ref&>);
static_assert(std::equality_comparable<Ref>);
static_assert(std::equality_comparable_with<Ref, Val>);
static_assert(std::totally_ordered<Ref>);
static_assert(std::totally_ordered_with<Ref, Val>);
static_assert(std::swappable_with<const Ref&, const Ref&>);

// Iterator concepts and algorithm constraints
static_assert(std::input_iterator<ZipIter>);
static_assert(std::forward_iterator<ZipIter>);
static_assert(std::bidirectional_iterator<ZipIter>);
static_assert(std::random_access_iterator<ZipIter>);
static_assert(std::indirectly_readable<ZipIter>);
static_assert(std::indirectly_writable<ZipIter, Val>);
static_assert(std::indirectly_swappable<ZipIter, ZipIter>);
static_assert(std::permutable<ZipIter>);
static_assert(std::sentinel_for<ZipIter, ZipIter>);
static_assert(std::indirect_strict_weak_order<std::ranges::less, ZipIter>);

}  // namespace tvsc::iterator::concept_checks
