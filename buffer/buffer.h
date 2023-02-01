#pragma once

#include <array>
#include <cstring>
#include <stdexcept>
#include <string>

namespace tvsc::buffer {

/**
 * std::array-like buffer type with bulk operations based on memcpy(3) for trivially copyable types.
 *
 * Buffer is a simple buffer type, similar to std::array, but with bulk read/write operations and
 * optimizations for trivially copyable types.
 *
 * BufferT has two template overloads, one for trivially copyable element types, and one where the
 * element type is not trivially copyable. The non-trivial element implementation uses loops to move
 * data. The implement for elements that are trivially copyable is based on memcpy.
 *
 * The Buffer type makes the syntax of choosing the correct BufferT template overload easier. It is
 * defined after the two template overloads.
 */
template <typename ElementT, size_t NUM_ELEMENTS, bool is_trivially_copyable>
class BufferT final {
 private:
  static_assert(NUM_ELEMENTS > 0, "Number of elements in the buffer must be positive");

  ElementT elements_[NUM_ELEMENTS]{};

  void validate_index(size_t index) const {
    if (index < 0 || index >= NUM_ELEMENTS) {
      using std::to_string;
      throw std::out_of_range("Invalid index " + to_string(index) +
                              " (NUM_ELEMENTS: " + to_string(NUM_ELEMENTS));
    }
  }

 public:
  constexpr size_t size() const { return NUM_ELEMENTS; }

  void clear() {
    ElementT default_element{};
    for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
      operator[](i) = default_element;
    }
  }

  const ElementT& read(size_t index) const { return operator[](index); }

  void read(size_t offset, size_t count, ElementT dest[]) const {
    validate_index(offset);
    validate_index(offset + count - 1);
    for (size_t i = 0; i < count; ++i) {
      dest[i] = operator[](i + offset);
    }
  }

  template <size_t ARRAY_SIZE>
  void read(size_t offset, size_t count, std::array<ElementT, ARRAY_SIZE>& dest) const {
    validate_index(offset);
    validate_index(offset + count - 1);
    if (ARRAY_SIZE < count) {
      using std::to_string;
      throw std::overflow_error("dest has insufficient space (" + to_string(count) + " vs " +
                                to_string(ARRAY_SIZE) + ")");
    }
    for (size_t i = 0; i < count; ++i) {
      dest[i] = operator[](i + offset);
    }
  }

  void write(size_t index, const ElementT& element) { operator[](index) = element; }

  void write(size_t index, ElementT&& element) { operator[](index) = std::move(element); }

  void write(size_t offset, size_t count, const ElementT src[]) {
    validate_index(offset);
    validate_index(offset + count - 1);
    for (size_t i = 0; i < count; ++i) {
      operator[](i + offset) = src[i];
    }
  }

  template <size_t ARRAY_SIZE>
  void write(size_t offset, size_t count, const std::array<ElementT, ARRAY_SIZE>& src) {
    validate_index(offset);
    validate_index(offset + count - 1);
    if (ARRAY_SIZE < count) {
      using std::to_string;
      throw std::overflow_error("src has insufficient space (" + to_string(count) + " vs " +
                                to_string(ARRAY_SIZE) + ")");
    }
    for (size_t i = 0; i < count; ++i) {
      operator[](i + offset) = src[i];
    }
  }

  const ElementT& operator[](size_t index) const {
    validate_index(index);
    return elements_[index];
  }

  ElementT& operator[](size_t index) {
    validate_index(index);
    return elements_[index];
  }

  constexpr ElementT* data() noexcept { return elements_; }
  constexpr const ElementT* data() const noexcept { return elements_; }

  constexpr std::string_view as_string_view() const {
    return std::string_view(elements_, NUM_ELEMENTS * sizeof(ElementT));
  }
};

// BufferT implementation for trivially copyable ElementT types.
template <typename ElementT, size_t NUM_ELEMENTS>
class BufferT<ElementT, NUM_ELEMENTS, true> final {
 private:
  static_assert(NUM_ELEMENTS > 0, "Number of elements in the buffer must be positive");

  ElementT elements_[NUM_ELEMENTS];

  void validate_index(size_t index) const {
    if (index < 0 || index >= NUM_ELEMENTS) {
      using std::to_string;
      throw std::out_of_range("Invalid index " + to_string(index) +
                              " (NUM_ELEMENTS: " + to_string(NUM_ELEMENTS) + ")");
    }
  }

 public:
  constexpr size_t size() const { return NUM_ELEMENTS; }

  void clear() {
    // ElementT is trivially copyable, but that does not mean that we can just memset the elements_
    // array to zero. We could however do increasingly sized memcpy's of a default initialized
    // element for an O(ln n) process. For now, we assume that an O(n) clear() is sufficient.
    ElementT default_element{};
    for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
      operator[](i) = default_element;
    }
  }

  const ElementT& read(size_t index) const { return operator[](index); }

  void read(size_t offset, size_t count, ElementT dest[]) const {
    validate_index(offset);
    validate_index(offset + count - 1);
    std::memcpy(dest, elements_ + offset, count * sizeof(ElementT));
  }

  template <size_t ARRAY_SIZE>
  void read(size_t offset, size_t count, std::array<ElementT, ARRAY_SIZE>& dest) const {
    validate_index(offset);
    validate_index(offset + count - 1);
    if (ARRAY_SIZE < count) {
      using std::to_string;
      throw std::overflow_error("dest has insufficient space (" + to_string(count) + " vs " +
                                to_string(ARRAY_SIZE) + ")");
    }
    std::memcpy(dest.data(), elements_ + offset, count * sizeof(ElementT));
  }

  void write(size_t index, const ElementT& element) { operator[](index) = element; }

  void write(size_t index, ElementT&& element) { operator[](index) = std::move(element); }

  void write(size_t offset, size_t count, const ElementT src[]) {
    validate_index(offset);
    validate_index(offset + count - 1);
    std::memcpy(elements_ + offset, src, count * sizeof(ElementT));
  }

  template <size_t ARRAY_SIZE>
  void write(size_t offset, size_t count, const std::array<ElementT, ARRAY_SIZE>& src) {
    validate_index(offset);
    validate_index(offset + count - 1);
    if (ARRAY_SIZE < count) {
      using std::to_string;
      throw std::overflow_error("src has insufficient space (" + to_string(count) + " vs " +
                                to_string(ARRAY_SIZE) + ")");
    }
    std::memcpy(elements_ + offset, src.data(), count * sizeof(ElementT));
  }

  const ElementT& operator[](size_t index) const {
    validate_index(index);
    return elements_[index];
  }

  ElementT& operator[](size_t index) {
    validate_index(index);
    return elements_[index];
  }

  constexpr ElementT* data() noexcept { return elements_; }
  constexpr const ElementT* data() const noexcept { return elements_; }

  constexpr std::string_view as_string_view() const {
    return std::string_view(elements_, NUM_ELEMENTS * sizeof(ElementT));
  }
};

template <typename ElementT, size_t NUM_ELEMENTS>
using Buffer = BufferT<ElementT, NUM_ELEMENTS, std::is_trivially_copyable<ElementT>::value>;

}  // namespace tvsc::buffer
