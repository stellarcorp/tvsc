#pragma once

#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>

namespace tvsc::buffer {

enum class InsertionPolicy {
  APPEND,
  SORTED,
};

enum class OverflowPolicy {
  // Reject the attempt to add the new item.
  REJECT,
  // Evict the element at the front (index 0 or similar). This policy gives the behavior of a ring
  // buffer.
  DROP_FRONT,
  // Call a function provided by the user that can reduce the size of the store in some way. The
  // most likely use case is when multiple entries can be transformed into fewer entries using
  // statistical / numerical methods.
  OVERFLOW_HANDLER,
};

template <typename S>
concept IsStore =  //
    requires {
      { S::insertion_policy() } -> std::same_as<InsertionPolicy>;
      { S::overflow_policy() } -> std::same_as<OverflowPolicy>;
    } and  //

    requires(const S& s) {
      typename S::size_type;
      requires std::integral<typename S::size_type>;
      typename S::value_type;
    } and  //

    requires(const S& s) {
      { s.capacity() } -> std::convertible_to<typename S::size_type>;
      { s.min_capacity() } -> std::convertible_to<typename S::size_type>;
      { s.max_capacity() } -> std::convertible_to<typename S::size_type>;
    } and  //

    requires(const S& s) {
      { s.size() } -> std::convertible_to<typename S::size_type>;
      { s.empty() } -> std::convertible_to<bool>;
    } and  //

    requires(const S& sc, S& s, const S::value_type& v, S::const_iterator iter) {
      { s.front() } -> std::convertible_to<typename S::value_type>;
      { s.back() } -> std::convertible_to<typename S::value_type>;
      { sc.front() } -> std::convertible_to<typename S::value_type>;
      { sc.back() } -> std::convertible_to<typename S::value_type>;
      { s.insert(v) } -> std::convertible_to<typename S::iterator>;
      { s.erase(iter) } -> std::convertible_to<typename S::iterator>;
      { s.clear() } -> std::same_as<void>;
    } and  //

    std::ranges::common_range<S> and  //
    true;

template <typename S>
concept IsConstantCapacityStore =  //
    IsStore<S> and                 //
    requires(const S& s) {
      // Constant capacity stores have their capacity fixed at
      // compile-time.
      { s.capacity() } -> std::convertible_to<typename S::size_type>;
      { S::min_capacity() } -> std::convertible_to<typename S::size_type>;
      { S::max_capacity() } -> std::convertible_to<typename S::size_type>;
      requires(S::min_capacity() == S::max_capacity());
    } and  //
    true;

template <typename S>
concept IsExpandableCapacityStore =  //
    IsStore<S> and                   //
    requires(const S& s) {
      // Constant capacity stores have their capacity fixed at
      // compile-time.
      { s.capacity() } -> std::convertible_to<typename S::size_type>;
      { S::min_capacity() } -> std::convertible_to<typename S::size_type>;
      { S::max_capacity() } -> std::convertible_to<typename S::size_type>;
      requires(S::min_capacity() < S::max_capacity());
    } and  //
    true;

template <typename S>
concept IsRandomAccessStore =                //
    IsStore<S> and                           //
    std::ranges::sized_range<S> and          //
    std::ranges::random_access_range<S> and  //
    requires(const S& s, S::size_type i) {
      { s[i] } -> std::convertible_to<typename S::value_type>;
      { s.at(i) } -> std::convertible_to<typename S::value_type>;
    } and  //
    true;

template <typename S>
concept IsQueue =                    //
    IsStore<S> and                   //
    std::ranges::sized_range<S> and  //
    requires(S& s, const S::value_type& v) {
      // Note: return type on these functions is unspecified. May be void.
      s.pop_front();
      s.push_back(v);
    } and  //
    true;

template <typename S>
concept IsRingBuffer =                                               //
    IsQueue<S> and                                                   //
    requires {                                                       //
      requires(S::overflow_policy() == OverflowPolicy::DROP_FRONT);  //
    } and                                                            //
    true;

}  // namespace tvsc::buffer
