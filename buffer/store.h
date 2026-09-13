#pragma once

#include <concepts>
#include <iterator>
#include <type_traits>

namespace tvsc::buffer {

enum class InsertionPolicy {
  APPEND,
  SORTED,
};

enum class OverflowPolicy {
  // Reject the attempt to add the new item.
  REJECT,
  // Evict the element at the front (index 0 or similar).
  DROP_FRONT,
  // Evict the element at the back.
  DROP_BACK,
};

template <typename I>
concept IsStoreIterator = std::random_access_iterator<I>;

template <typename S>
concept IsStore = std::is_integral_v<typename S::size_type> and    //
                  IsStoreIterator<typename S::iterator> and        //
                  IsStoreIterator<typename S::const_iterator> and  //
                  requires {
                    { S::insertion_policy() } -> std::same_as<InsertionPolicy>;
                    { S::overflow_policy() } -> std::same_as<OverflowPolicy>;
                    { S::capacity() } -> std::same_as<typename S::size_type>;
                  } and  //
                  requires(const S& s) {
                    { s.size() } -> std::same_as<typename S::size_type>;
                    { s.begin() } -> std::same_as<typename S::const_iterator>;
                    { s.end() } -> std::same_as<typename S::const_iterator>;
                  } and  //
                  true;

}  // namespace tvsc::buffer
