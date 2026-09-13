#pragma once

#include <algorithm>
#include <array>
#include <type_traits>
#include <utility>

#include "buffer/store.h"
#include "iterator/ring_iterator.h"

namespace tvsc::buffer {

template <typename Value, std::unsigned_integral Size, Size CAPACITY,
          InsertionPolicy INSERTION_POLICY, OverflowPolicy OVERFLOW_POLICY>
  requires(std::in_range<Size>(CAPACITY))
class ArrayStore final {
 public:
  using value_type = Value;
  using size_type = Size;
  using container_type = std::array<value_type, CAPACITY>;
  using reference = container_type::reference;
  using const_reference = container_type::const_reference;
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;

 private:
  size_type size_{};
  container_type elements_{};

  constexpr bool handle_overflow() noexcept {
    if constexpr (OVERFLOW_POLICY == OverflowPolicy::REJECT) {
      return false;
    }
  }

 public:
  constexpr ArrayStore() noexcept = default;
  constexpr ArrayStore(const ArrayStore& rhs) noexcept = default;
  constexpr ArrayStore(ArrayStore&& rhs) noexcept = default;

  constexpr ArrayStore& operator=(const ArrayStore& rhs) noexcept = default;
  constexpr ArrayStore& operator=(ArrayStore&& rhs) noexcept = default;

  [[nodiscard]] static constexpr InsertionPolicy insertion_policy() noexcept {
    return INSERTION_POLICY;
  }

  [[nodiscard]] static constexpr OverflowPolicy overflow_policy() noexcept {
    return OVERFLOW_POLICY;
  }

  [[nodiscard]] static constexpr size_type capacity() noexcept { return CAPACITY; }

  [[nodiscard]] constexpr size_type size() const noexcept { return size_; }

  [[nodiscard]] constexpr IsStoreIterator auto begin() const noexcept { return elements_.begin(); }
  [[nodiscard]] constexpr IsStoreIterator auto begin() noexcept { return elements_.begin(); }

  [[nodiscard]] constexpr IsStoreIterator auto end() const noexcept {
    return elements_.begin() + size_;
  }
  [[nodiscard]] constexpr IsStoreIterator auto end() noexcept { return elements_.begin() + size_; }

  constexpr IsStoreIterator auto add(value_type value) noexcept {
    if (size_ == CAPACITY) {
      const bool can_insert{handle_overflow()};
      if (!can_insert) {
        return end();
      }
    }
    if constexpr (INSERTION_POLICY == InsertionPolicy::APPEND) {
      const auto insertion_point{end()};
      *insertion_point = value;
      ++size_;
      return insertion_point;
    } else if constexpr (INSERTION_POLICY == InsertionPolicy::SORTED) {
      const auto insertion_point{std::lower_bound(begin(), end(), value)};
      std::copy(insertion_point, end(), insertion_point + 1);
      *insertion_point = value;
      ++size_;
      return insertion_point;
    }
  }

  constexpr void clear() noexcept { size_ = 0; }
};

template <typename Value, std::unsigned_integral Size, Size CAPACITY,
          InsertionPolicy INSERTION_POLICY>
class ArrayStore<Value, Size, CAPACITY, INSERTION_POLICY, OverflowPolicy::DROP_FRONT> final {
 public:
  using value_type = Value;
  using size_type = Size;
  using container_type = std::array<value_type, CAPACITY>;
  using reference = container_type::reference;
  using const_reference = container_type::const_reference;
  using iterator = typename iterator::RingIterator<typename container_type::iterator>;
  using const_iterator = typename iterator::RingIterator<typename container_type::const_iterator>;
  using difference_type = std::iter_difference_t<iterator>;

 private:
  static constexpr auto OVERFLOW_POLICY{OverflowPolicy::DROP_FRONT};

  // head_ and tail_ are logically managed as a queue. New items enter at the back (tail) of the
  // queue. Items leave from the head of the queue.
  difference_type head_{};
  difference_type tail_{};
  container_type elements_{};

  constexpr bool handle_overflow() noexcept {
    ++head_;
    return true;
  }

 public:
  constexpr ArrayStore() noexcept = default;
  constexpr ArrayStore(const ArrayStore& rhs) noexcept = default;
  constexpr ArrayStore(ArrayStore&& rhs) noexcept = default;

  constexpr ArrayStore& operator=(const ArrayStore& rhs) noexcept = default;
  constexpr ArrayStore& operator=(ArrayStore&& rhs) noexcept = default;

  [[nodiscard]] static constexpr InsertionPolicy insertion_policy() noexcept {
    return INSERTION_POLICY;
  }

  [[nodiscard]] static constexpr OverflowPolicy overflow_policy() noexcept {
    return OVERFLOW_POLICY;
  }

  [[nodiscard]] static constexpr size_type capacity() noexcept { return CAPACITY; }

  [[nodiscard]] constexpr size_type size() const noexcept {
    return static_cast<size_type>(tail_ - head_);
  }

  [[nodiscard]] constexpr IsStoreIterator auto begin() const noexcept {
    return const_iterator{elements_.begin(), elements_.end(), head_};
  }
  [[nodiscard]] constexpr IsStoreIterator auto begin() noexcept {
    return iterator{elements_.begin(), elements_.end(), head_};
  }

  [[nodiscard]] constexpr IsStoreIterator auto end() const noexcept {
    return const_iterator{elements_.begin(), elements_.end(), tail_};
  }
  [[nodiscard]] constexpr IsStoreIterator auto end() noexcept {
    return iterator{elements_.begin(), elements_.end(), tail_};
  }

  constexpr IsStoreIterator auto add(value_type value) noexcept {
    if (size() == CAPACITY) {
      const bool can_insert{handle_overflow()};
      if (!can_insert) {
        return end();
      }
    }
    if constexpr (INSERTION_POLICY == InsertionPolicy::APPEND) {
      const auto insertion_point{end()};
      *insertion_point = value;
      ++tail_;
      return insertion_point;
    }
  }

  constexpr void clear() noexcept { head_ = tail_; }
};

namespace concept_checks {
using SmallArrayStore = ArrayStore<int, size_t, 4, InsertionPolicy::SORTED, OverflowPolicy::REJECT>;
static_assert(IsStore<SmallArrayStore>);
}  // namespace concept_checks

}  // namespace tvsc::buffer
