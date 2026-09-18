#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "base/except.h"
#include "buffer/store.h"
#include "iterator/ring_iterator.h"

namespace tvsc::buffer {

template <typename Store, bool is_const = false>
class RandomAccessStoreIterator;

template <typename Value, std::unsigned_integral Size, Size CAPACITY,
          InsertionPolicy INSERTION_POLICY, OverflowPolicy OVERFLOW_POLICY>
class ArrayStore;

template <typename Store>
using OverflowHandler = std::function<bool(Store, typename Store::value_type&)>;

template <typename Value, std::unsigned_integral Size, Size CAPACITY,
          InsertionPolicy INSERTION_POLICY, OverflowPolicy OVERFLOW_POLICY>
class ArrayStore final {
 private:
  static constexpr bool is_ring_buffer{OVERFLOW_POLICY == OverflowPolicy::DROP_FRONT};
  static constexpr bool has_overflow_handler{OVERFLOW_POLICY == OverflowPolicy::OVERFLOW_HANDLER};
  static constexpr bool index_modification_allowed{INSERTION_POLICY != InsertionPolicy::SORTED};

 public:
  using value_type = Value;
  using size_type = Size;
  using container_type = std::array<value_type, CAPACITY>;

  using raw_iterator = RandomAccessStoreIterator<ArrayStore, /* is_const */ false>;
  using const_iterator = RandomAccessStoreIterator<ArrayStore, /* is_const */ true>;
  using iterator = std::conditional_t<index_modification_allowed, raw_iterator, const_iterator>;

  using raw_reference = std::iter_reference_t<raw_iterator>;
  using reference = std::iter_reference_t<iterator>;
  using const_reference = std::iter_reference_t<const_iterator>;

 private:
  // head and tail are logically managed as a queue. New items enter at the back (tail) of the
  // queue. Items leave from the head of the queue. These values are monotonically increasing. When
  // adding a new element, tail gets incremented. When removing an element, head gets incremented.
  // Neither value ever decreases. This puts a constraint on the lifetime maximum number of elements
  // of this store. See the incrementing logic in the add() method for details on how this is
  // handled.
  struct RingPosition final {
    size_type head{};
    size_type tail{};
  };

  using SizeStorage = std::conditional_t<is_ring_buffer, RingPosition, size_type>;

  struct Empty final {};
  using OverflowHandlerStorage =
      std::conditional_t<has_overflow_handler, OverflowHandler<ArrayStore>, Empty>;

  // size_ is either the head and tail tracking indices for a ring buffer, or it is just the
  // size_type. Note that by not including a single API behind these concepts, we induce compiler
  // errors when code attempts to use one when it was meant to use the other.
  SizeStorage size_{};

  // When the overflow_handler_ is not needed it is an instance of Empty and takes no space due to
  // [[no_unique_address]].
  [[no_unique_address]] OverflowHandlerStorage overflow_handler_{};

  container_type elements_{};

  friend raw_iterator;
  friend iterator;
  friend const_iterator;

  [[nodiscard]] raw_iterator raw_begin() noexcept {
    if constexpr (is_ring_buffer) {
      return raw_iterator{*this, size_.head};
    } else {
      return raw_iterator{*this, 0};
    }
  }

  [[nodiscard]] raw_iterator raw_end() noexcept {
    if constexpr (is_ring_buffer) {
      return raw_iterator{*this, size_.tail};
    } else {
      return raw_iterator{*this, size_};
    }
  }

  [[nodiscard]] constexpr const_reference raw_get(size_type n) const noexcept { return (*this)[n]; }

  [[nodiscard]] constexpr raw_reference raw_get(size_type n) noexcept {
    return const_cast<raw_reference>(std::as_const(*this)[n]);
  }

  [[nodiscard]] constexpr bool handle_overflow(const value_type& value) noexcept {
    if constexpr (IsExpandableCapacityStore<ArrayStore>) {
      if (capacity() < max_capacity()) {
        reserve(std::min(max_capacity(), 2 * capacity()));
      }
    }
    if constexpr (has_overflow_handler) {
      const bool insert_allowed{overflow_handler_(*this, value)};
      return insert_allowed and size() < capacity();
    } else if constexpr (is_ring_buffer) {
      ++size_.head;
      return true;
    } else if constexpr (OVERFLOW_POLICY == OverflowPolicy::REJECT) {
      return false;
    } else {
      static_assert(false, "OverflowPolicy not implemented in handle_overflow.");
    }
  }

 public:
  constexpr ArrayStore() noexcept
    requires(!has_overflow_handler)
  = default;

  explicit constexpr ArrayStore(OverflowHandler<ArrayStore> overflow_handler) noexcept
    requires(has_overflow_handler)
      : overflow_handler_(std::move(overflow_handler)) {}

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
  [[nodiscard]] static constexpr size_type min_capacity() noexcept { return CAPACITY; }
  [[nodiscard]] static constexpr size_type max_capacity() noexcept { return CAPACITY; }

  constexpr void reserve(size_type new_capacity) noexcept
    requires(IsExpandableCapacityStore<ArrayStore>)
  {
    elements_.reserve(std::min(new_capacity, max_capacity()));
  }

  [[nodiscard]] constexpr size_type size() const noexcept {
    if constexpr (is_ring_buffer) {
      return size_.tail - size_.head;
    } else {
      return size_;
    }
  }

  [[nodiscard]] constexpr bool empty() const noexcept {
    if constexpr (is_ring_buffer) {
      return size_.tail == size_.head;
    } else {
      return size_ == 0;
    }
  }

  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    if constexpr (is_ring_buffer) {
      return const_iterator{*this, size_.head};
    } else {
      return const_iterator(*this, 0);
    }
  }
  [[nodiscard]] constexpr iterator begin() noexcept {
    if constexpr (is_ring_buffer) {
      return iterator{*this, size_.head};
    } else {
      return iterator(*this, 0);
    }
  }

  [[nodiscard]] constexpr const_iterator end() const noexcept {
    if constexpr (is_ring_buffer) {
      return const_iterator{*this, size_.tail};
    } else {
      return const_iterator(*this, size_);
    }
  }
  [[nodiscard]] constexpr iterator end() noexcept {
    if constexpr (is_ring_buffer) {
      return iterator{*this, size_.tail};
    } else {
      return iterator(*this, size_);
    }
  }

  [[nodiscard]] constexpr size_type begin_index() const noexcept {
    if constexpr (is_ring_buffer) {
      return size_.head;
    } else {
      return 0;
    }
  }

  [[nodiscard]] constexpr size_type end_index() const noexcept {
    if constexpr (is_ring_buffer) {
      return size_.tail;
    } else {
      return size_;
    }
  }

  [[nodiscard]] constexpr const_reference front() const noexcept {
    if (empty()) {
      except<std::out_of_range>("Attempt to get front() element of empty store.");
    }
    if constexpr (is_ring_buffer) {
      return elements_[size_.head % capacity()];
    } else {
      return elements_[0];
    }
  }

  [[nodiscard]] constexpr reference front() noexcept
    requires(index_modification_allowed)
  {
    return const_cast<reference>(std::as_const(*this).front());
  }

  [[nodiscard]] constexpr const_reference back() const noexcept {
    if (empty()) {
      except<std::out_of_range>("Attempt to get back() element of empty store.");
    }
    if constexpr (is_ring_buffer) {
      return elements_[(size_.tail - 1) % capacity()];
    } else {
      return elements_[size_ - 1];
    }
  }

  [[nodiscard]] constexpr reference back() noexcept
    requires(index_modification_allowed)
  {
    return const_cast<reference>(std::as_const(*this).back());
  }

  [[nodiscard]] constexpr const_reference at(size_type n) const noexcept {
    if constexpr (is_ring_buffer) {
      if (n >= size_.tail or n < size_.head) {
        except<std::out_of_range>(
            "Attempt to fetch element (via at()) of empty store. (is_ring_buffer)");
      }
      return elements_[n % capacity()];
    } else {
      if (n >= size_) {
        except<std::out_of_range>("Attempt to fetch element (via at()) of empty store.");
      }
      return elements_[n];
    }
  }

  [[nodiscard]] constexpr reference at(size_type n) noexcept
    requires(index_modification_allowed)
  {
    return const_cast<reference>(std::as_const(*this).at(n));
  }

  [[nodiscard]] constexpr const_reference operator[](size_type n) const noexcept {
    if constexpr (is_ring_buffer) {
      return elements_[n % capacity()];
    } else {
      return elements_[n];
    }
  }

  [[nodiscard]] constexpr reference operator[](size_type n) noexcept
    requires(index_modification_allowed)
  {
    return const_cast<reference>(std::as_const(*this)[n]);
  }

  constexpr iterator insert(const value_type& value) {
    if (size() == capacity()) {
      if (!handle_overflow(value)) {
        return end();
      }
    }

    if constexpr (INSERTION_POLICY == InsertionPolicy::APPEND) {
      const auto insertion_point{end()};
      *insertion_point = value;

      if constexpr (is_ring_buffer) {
        if (++size_.tail == 0) [[unlikely]] {
          except<std::overflow_error>("Overflow on tail index value. (APPEND)");
        }
      } else {
        if (++size_ == 0) [[unlikely]] {
          except<std::overflow_error>("Overflow on size index value. (APPEND)");
        }
      }
      return insertion_point;
    } else if constexpr (INSERTION_POLICY == InsertionPolicy::SORTED) {
      // Under the sorted insertion policy, the iterators from begin() and end() are actually
      // const_iterators. What they point to cannot be modifed. If it could be modified, this
      // store would likely no longer be sorted. But this also means that we can't just use the
      // begin() and end() iterators here, since we are modifying the store. To work around this
      // issue, without requiring a custom binary search implementation, we use the raw_iterator
      // to insert the value and then translate that to an iterator instance before returning.
      const auto insertion_point{std::lower_bound(raw_begin(), raw_end(), value)};

      std::copy_backward(insertion_point, raw_end(), std::next(raw_end(), 1));
      *insertion_point = value;

      if constexpr (is_ring_buffer) {
        if (++size_.tail == 0) [[unlikely]] {
          except<std::overflow_error>("Overflow on tail index value. (APPEND)");
        }
      } else {
        if (++size_ == 0) [[unlikely]] {
          except<std::overflow_error>("Overflow on size index value. (APPEND)");
        }
      }
      return iterator{insertion_point};
    }
  }

  constexpr iterator erase(const_iterator pos) {
    raw_iterator raw_pos{raw_iterator::as_non_const(pos)};
    if (empty()) [[unlikely]] {
      except<std::out_of_range>("Attempt to erase element from empty store.");
    } else {
      if constexpr (is_ring_buffer) {
        std::copy_backward(raw_begin(), raw_pos, std::next(raw_pos, 1));
        ++size_.head;
        return std::next(raw_pos, 1);
      } else {
        std::copy(std::next(raw_pos, 1), raw_end(), raw_pos);
        --size_;
        return raw_pos;
      }
    }
  }

  constexpr void push_back(const value_type& value) noexcept
    requires(index_modification_allowed)
  {
    if (size() == capacity()) {
      if (!handle_overflow(value)) {
        return;
      }
    }

    if constexpr (is_ring_buffer) {
      elements_[size_.tail % capacity()] = value;
      if (++size_.tail == 0) [[unlikely]] {
        except<std::overflow_error>("Overflow on tail index value. (APPEND)");
      }
    } else {
      elements_[size_] = value;
      if (++size_ == 0) [[unlikely]] {
        except<std::overflow_error>("Overflow on size index value. (APPEND)");
      }
    }
  }

  constexpr void pop_front() noexcept {
    if constexpr (is_ring_buffer) {
      ++size_.head;
    } else {
      std::copy(std::next(raw_begin(), 1), raw_end(), raw_begin());
      --size_;
    }
  }

  constexpr void clear() noexcept {
    if constexpr (is_ring_buffer) {
      // Note that we bring the head_ up to the tail_ so that head_ and tail_ are both
      // monotonically increasing for the lifetime of this container.
      size_.head = size_.tail;
    } else {
      size_ = 0;
    }
  }
};

template <typename Store, bool is_const>
class RandomAccessStoreIterator final {
 public:
  using iterator_category = std::random_access_iterator_tag;
  using iterator_concept = std::random_access_iterator_tag;
  using value_type = typename Store::value_type;
  using difference_type = std::ptrdiff_t;
  using pointer = std::conditional_t<is_const, const value_type*, value_type*>;
  using reference = std::conditional_t<is_const, const value_type&, value_type&>;
  using size_type = typename Store::size_type;

 private:
  using StorePtr = std::conditional_t<is_const, const Store*, Store*>;
  using StoreRef = std::conditional_t<is_const, const Store&, Store&>;

  StorePtr store_{nullptr};
  size_type pos_{};

  constexpr RandomAccessStoreIterator(StorePtr store, size_type pos) noexcept
      : store_{store}, pos_{pos} {}

 public:
  constexpr RandomAccessStoreIterator() noexcept = default;

  constexpr RandomAccessStoreIterator(StoreRef store, size_type pos) noexcept
      : store_{&store}, pos_{pos} {}

  constexpr RandomAccessStoreIterator(const RandomAccessStoreIterator&) noexcept = default;
  constexpr RandomAccessStoreIterator(RandomAccessStoreIterator&&) noexcept = default;
  constexpr RandomAccessStoreIterator& operator=(const RandomAccessStoreIterator&) noexcept =
      default;
  constexpr RandomAccessStoreIterator& operator=(RandomAccessStoreIterator&&) noexcept = default;

  // Allow implicit conversion from non-const iterator to const iterator.
  constexpr RandomAccessStoreIterator(const RandomAccessStoreIterator<Store, false>& other) noexcept
    requires is_const
      : store_{other.store_}, pos_{other.pos_} {}

  // Allow explicit conversion from const iterator to non-const iterator.
  static constexpr RandomAccessStoreIterator as_non_const(
      const RandomAccessStoreIterator<Store, true>& other) noexcept
    requires(!is_const)
  {
    return {const_cast<StorePtr>(other.store_), other.pos_};
  }

  constexpr RandomAccessStoreIterator& operator=(
      const RandomAccessStoreIterator<Store, false>& other) noexcept
    requires is_const
  {
    store_ = other.store_;
    pos_ = other.pos_;
    return *this;
  }

  [[nodiscard]] constexpr reference operator*() const noexcept { return store_->raw_get(pos_); }

  [[nodiscard]] constexpr pointer operator->() const noexcept { return &(store_->raw_get(pos_)); }

  [[nodiscard]] constexpr reference operator[](difference_type n) const noexcept {
    return store_->raw_get(static_cast<size_type>(static_cast<difference_type>(pos_) + n));
  }

  constexpr RandomAccessStoreIterator& operator++() noexcept {
    ++pos_;
    return *this;
  }

  [[nodiscard]] constexpr RandomAccessStoreIterator operator++(int) noexcept {
    RandomAccessStoreIterator tmp = *this;
    ++pos_;
    return tmp;
  }

  constexpr RandomAccessStoreIterator& operator--() noexcept {
    --pos_;
    return *this;
  }

  [[nodiscard]] constexpr RandomAccessStoreIterator operator--(int) noexcept {
    RandomAccessStoreIterator tmp = *this;
    --pos_;
    return tmp;
  }

  constexpr RandomAccessStoreIterator& operator+=(difference_type n) noexcept {
    pos_ = static_cast<size_type>(static_cast<difference_type>(pos_) + n);
    return *this;
  }

  constexpr RandomAccessStoreIterator& operator-=(difference_type n) noexcept {
    pos_ = static_cast<size_type>(static_cast<difference_type>(pos_) - n);
    return *this;
  }

  [[nodiscard]] friend constexpr RandomAccessStoreIterator operator+(RandomAccessStoreIterator it,
                                                                     difference_type n) noexcept {
    it += n;
    return it;
  }

  [[nodiscard]] friend constexpr RandomAccessStoreIterator operator+(
      difference_type n, RandomAccessStoreIterator it) noexcept {
    it += n;
    return it;
  }

  [[nodiscard]] friend constexpr RandomAccessStoreIterator operator-(RandomAccessStoreIterator it,
                                                                     difference_type n) noexcept {
    it -= n;
    return it;
  }

  [[nodiscard]] friend constexpr difference_type operator-(
      const RandomAccessStoreIterator& lhs, const RandomAccessStoreIterator& rhs) noexcept {
    return static_cast<difference_type>(lhs.pos_) - static_cast<difference_type>(rhs.pos_);
  }

  [[nodiscard]] friend constexpr auto operator<=>(const RandomAccessStoreIterator& lhs,
                                                  const RandomAccessStoreIterator& rhs) noexcept {
    return lhs.pos_ <=> rhs.pos_;
  }

  [[nodiscard]] friend constexpr bool operator==(const RandomAccessStoreIterator& lhs,
                                                 const RandomAccessStoreIterator& rhs) noexcept {
    return lhs.pos_ == rhs.pos_;
  }

  template <typename, bool>
  friend class RandomAccessStoreIterator;

  // Accessor to help with debugging.
  [[nodiscard]] constexpr auto pos() const noexcept { return pos_; }
};

template <typename Value, size_t CAPACITY>
using RingBuffer =
    ArrayStore<Value, size_t, CAPACITY, InsertionPolicy::APPEND, OverflowPolicy::DROP_FRONT>;

template <typename Value, size_t CAPACITY>
using Queue = ArrayStore<Value, size_t, CAPACITY, InsertionPolicy::APPEND, OverflowPolicy::REJECT>;

namespace concept_checks {
using ExampleRingBuffer = RingBuffer<int, 4>;
static_assert(IsRingBuffer<ExampleRingBuffer>);
using ExampleQueue = Queue<int, 4>;
static_assert(IsQueue<ExampleQueue>);
}  // namespace concept_checks

}  // namespace tvsc::buffer
