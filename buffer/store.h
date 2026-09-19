/**
 * This file defines several basic container adapters, such as a RingBuffer and a Queue. These
 * containers are implemented on top of a general purpose adapter named Store along with an
 * iterator and other supporting types.
 *
 * This approach started as an experiment. Could a single class handle the use cases of a large
 * number of container-based data structures? The result is a trade-off.
 *
 * For a small team of C++ experts, this solution is fantastic. All of the code is centralized and
 * well-tested. Different container implementations will not drift out of sync with each other. The
 * implementation details are contained in a small number of files, so no one is opening many
 * different files to read the different implementations of each container.
 *
 * For a large team, this approach would be a disaster. It requires deep knowledge of C++. The
 * number of if-blocks (usually, constexpr if) creates a unique form of spaghetti code. Parsing
 * through the different combinations is a headache. And in the likely event of an error, the
 * compiler messages are inscrutable.
 */
#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "base/except.h"
#include "buffer/store_interface.h"

namespace tvsc::buffer {

/**
 * Signature for callback functions that will be called when a Store overflows. Note that not all
 * Store specializations support an overflow callback.
 */
template <typename Store>
using OverflowHandler = std::function<bool(Store&, typename Store::value_type&)>;

namespace internal {

template <typename Store, bool is_const = false>
class RandomAccessStoreIterator;

template <typename Value, std::unsigned_integral Size, Size MIN_CAPACITY, Size MAX_CAPACITY,
          InsertionPolicy INSERTION_POLICY, OverflowPolicy OVERFLOW_POLICY,
          typename Container = std::array<Value, MAX_CAPACITY>>
  requires(MIN_CAPACITY > 0) and std::default_initializable<Value>
class Store;

/**
 * General Store class that provides a unified implementation of several common data structures.
 * Primarly, this class is a template for container adapters with various policies for insertion and
 * overflow. This template is not intended to be used directly. Various specialized adapters that
 * can be used directly are configured at the bottom of this file. Use these specialized adapters.
 *
 * Note that this template and its specializations only support value types that are default
 * initializable. This constraint could be relaxed, but so far, that has not been necessary.
 */
template <typename Value, std::unsigned_integral Size, Size MIN_CAPACITY, Size MAX_CAPACITY,
          InsertionPolicy INSERTION_POLICY, OverflowPolicy OVERFLOW_POLICY, typename Container>
  requires(MIN_CAPACITY > 0) and std::default_initializable<Value>
class Store final {
 private:
  static constexpr bool is_ring_buffer{OVERFLOW_POLICY == OverflowPolicy::DROP_OLDEST};
  static constexpr bool index_modification_allowed{INSERTION_POLICY != InsertionPolicy::SORTED};

  // TODO(james): Add a template parameter for an overflow handler and update this conditional to a
  // check if that parameter is a valid, callable functor.
  static constexpr bool has_overflow_handler{false};

 public:
  using value_type = Value;
  using size_type = Size;
  using container_type = Container;

  using raw_iterator = RandomAccessStoreIterator<Store, /* is_const */ false>;
  using const_iterator = RandomAccessStoreIterator<Store, /* is_const */ true>;
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
      std::conditional_t<has_overflow_handler, OverflowHandler<Store>, Empty>;

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
    if constexpr (IsExpandableCapacityStore<Store>) {
      if (capacity() < max_capacity()) {
        reserve(2 * capacity());
        return true;
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
  constexpr Store() noexcept
    requires(!has_overflow_handler)
  = default;

  explicit constexpr Store(OverflowHandler<Store> overflow_handler) noexcept
    requires(has_overflow_handler)
      : overflow_handler_(std::move(overflow_handler)) {}

  constexpr Store(const Store& rhs) noexcept = default;
  constexpr Store(Store&& rhs) noexcept = default;

  constexpr Store& operator=(const Store& rhs) noexcept = default;
  constexpr Store& operator=(Store&& rhs) noexcept = default;

  [[nodiscard]] static constexpr InsertionPolicy insertion_policy() noexcept {
    return INSERTION_POLICY;
  }

  [[nodiscard]] static constexpr OverflowPolicy overflow_policy() noexcept {
    return OVERFLOW_POLICY;
  }

  [[nodiscard]] constexpr size_type capacity() const noexcept {
    if constexpr (IsConstantCapacityStore<Store>) {
      return MAX_CAPACITY;
    } else {
      return elements_.capacity();
    }
  }

  [[nodiscard]] static constexpr size_type min_capacity() noexcept { return MIN_CAPACITY; }
  [[nodiscard]] static constexpr size_type max_capacity() noexcept { return MAX_CAPACITY; }

  constexpr void reserve(size_type new_capacity) noexcept
    requires(IsExpandableCapacityStore<Store>)
  {
    static_assert(HasReserve<container_type>);
    new_capacity = std::clamp(new_capacity, min_capacity(), max_capacity());
    elements_.reserve(new_capacity);
    elements_.resize(elements_.capacity());
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
            "Attempt to fetch element (via at()) that is out of range of the store. "
            "(is_ring_buffer)");
      }
      return elements_[n % capacity()];
    } else {
      if (n >= size_) {
        except<std::out_of_range>(
            "Attempt to fetch element (via at()) that is out of range of the store.");
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

  constexpr iterator insert(value_type&& value) noexcept {
    if (size() == capacity()) {
      if (!handle_overflow(value)) {
        return end();
      }
    }

    if constexpr (INSERTION_POLICY == InsertionPolicy::APPEND) {
      if constexpr (is_ring_buffer) {
        if (++size_.tail == 0) [[unlikely]] {
          except<std::overflow_error>("Overflow on tail index value. (APPEND)");
        }
      } else {
        if (++size_ == 0) [[unlikely]] {
          except<std::overflow_error>("Overflow on size index value. (APPEND)");
        }
      }
      const auto insertion_point{std::prev(end())};
      *insertion_point = std::move(value);
      return insertion_point;
    } else if constexpr (INSERTION_POLICY == InsertionPolicy::SORTED) {
      // Under the sorted insertion policy, the iterators from begin() and end() are actually
      // const_iterators. What they point to cannot be modifed. If it could be modified, this
      // store would likely no longer be sorted. But this also means that we can't just use the
      // begin() and end() iterators here, since we are modifying the store. To work around this
      // issue, without requiring a custom binary search implementation, we use the raw_iterator
      // to insert the value and then translate that to an iterator instance before returning.
      const auto insertion_point{std::lower_bound(raw_begin(), raw_end(), value)};

      if constexpr (is_ring_buffer) {
        if (++size_.tail == 0) [[unlikely]] {
          except<std::overflow_error>("Overflow on tail index value. (SORTED)");
        }
      } else {
        if (++size_ == 0) [[unlikely]] {
          except<std::overflow_error>("Overflow on size index value. (SORTED)");
        }
      }

      std::copy_backward(insertion_point, std::prev(raw_end()), raw_end());
      *insertion_point = std::move(value);

      return iterator{insertion_point};
    }
  }

  constexpr iterator insert(const value_type& value) noexcept { return insert(value_type{value}); }

  constexpr iterator erase(const_iterator pos) noexcept {
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

  constexpr void push_back(value_type&& value) noexcept
    requires(index_modification_allowed)
  {
    if (size() == capacity()) {
      if (!handle_overflow(value)) {
        return;
      }
    }

    if constexpr (is_ring_buffer) {
      elements_[size_.tail % capacity()] = std::move(value);
      if (++size_.tail == 0) [[unlikely]] {
        except<std::overflow_error>("Overflow on tail index value. (APPEND)");
      }
    } else {
      elements_[size_] = std::move(value);
      if (++size_ == 0) [[unlikely]] {
        except<std::overflow_error>("Overflow on size index value. (APPEND)");
      }
    }
  }

  constexpr void push_back(const value_type& value) noexcept { push_back(value_type{value}); }

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

  template <bool OtherConst>
  [[nodiscard]] friend constexpr difference_type operator-(
      const RandomAccessStoreIterator& lhs,
      const RandomAccessStoreIterator<Store, OtherConst>& rhs) noexcept {
    return static_cast<difference_type>(lhs.pos_) - static_cast<difference_type>(rhs.pos_);
  }

  template <bool OtherConst>
  [[nodiscard]] friend constexpr bool operator==(
      const RandomAccessStoreIterator& lhs,
      const RandomAccessStoreIterator<Store, OtherConst>& rhs) noexcept {
    return lhs.pos_ == rhs.pos_;
  }

  template <bool OtherConst>
  [[nodiscard]] friend constexpr auto operator<=>(
      const RandomAccessStoreIterator& lhs,
      const RandomAccessStoreIterator<Store, OtherConst>& rhs) noexcept {
    return lhs.pos_ <=> rhs.pos_;
  }

  template <typename, bool>
  friend class RandomAccessStoreIterator;

  // Accessor to help with debugging.
  [[nodiscard]] constexpr auto pos() const noexcept { return pos_; }
};

}  // namespace internal

/**
 * Adapter to create a ring buffer out of a standard container.
 */
template <typename Value, size_t MIN_CAPACITY, size_t MAX_CAPACITY,
          typename Container = std::array<Value, MAX_CAPACITY>>
using RingBuffer = internal::Store<Value, size_t, MIN_CAPACITY, MAX_CAPACITY,
                                   InsertionPolicy::APPEND, OverflowPolicy::DROP_OLDEST, Container>;

/**
 * Adapter to create a queue out of a standard container.
 */
template <typename Value, size_t MIN_CAPACITY, size_t MAX_CAPACITY,
          typename Container = std::array<Value, MAX_CAPACITY>>
using Queue = internal::Store<Value, size_t, MIN_CAPACITY, MAX_CAPACITY, InsertionPolicy::APPEND,
                              OverflowPolicy::REJECT, Container>;

/**
 * Adapter to create a sorted queue, also known as a priority queue, from a standard container.
 */
template <typename Value, size_t MIN_CAPACITY, size_t MAX_CAPACITY,
          typename Container = std::array<Value, MAX_CAPACITY>>
using SortedQueue = internal::Store<Value, size_t, MIN_CAPACITY, MAX_CAPACITY,
                                    InsertionPolicy::SORTED, OverflowPolicy::REJECT, Container>;

// Check that the various types from the template above adhere to the intended concepts. These
// checks act as an early test for these types.
namespace concept_checks {

template <typename Value, size_t CAPACITY, InsertionPolicy INSERTION_POLICY,
          OverflowPolicy OVERFLOW_POLICY>
  requires std::default_initializable<Value>
using ArrayStore = internal::Store<Value, size_t, CAPACITY, CAPACITY, INSERTION_POLICY,
                                   OVERFLOW_POLICY, std::array<Value, CAPACITY>>;

using ExampleArrayStore = ArrayStore<int, 4, InsertionPolicy::APPEND, OverflowPolicy::REJECT>;
static_assert(IsConstantCapacityStore<ExampleArrayStore>);
static_assert(IsRandomAccessStore<ExampleArrayStore>);

template <typename Value, size_t MIN_CAPACITY, size_t MAX_CAPACITY,
          InsertionPolicy INSERTION_POLICY, OverflowPolicy OVERFLOW_POLICY>
using VectorStore = internal::Store<Value, size_t, MIN_CAPACITY, MAX_CAPACITY, INSERTION_POLICY,
                                    OVERFLOW_POLICY, std::vector<Value>>;

using ExampleVectorStore = VectorStore<int, 4, 64, InsertionPolicy::APPEND, OverflowPolicy::REJECT>;
static_assert(IsExpandableCapacityStore<ExampleVectorStore>);
static_assert(IsRandomAccessStore<ExampleVectorStore>);

using ExampleRingBuffer = RingBuffer<int, 4, 4>;
static_assert(IsRingBuffer<ExampleRingBuffer>);

using ExampleQueue = Queue<int, 4, 4>;
static_assert(IsQueue<ExampleQueue>);

using ExampleSortedQueue = SortedQueue<int, 4, 4>;
static_assert(IsStore<ExampleSortedQueue>);

}  // namespace concept_checks

}  // namespace tvsc::buffer
