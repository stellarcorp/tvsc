#pragma once

#include <array>
#include <cstring>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

#include "base/except.h"

namespace tvsc::buffer {

/**
 * std::array-like buffer type with bounds checking and bulk operations for trivially copyable
 * (https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable) types. If a type is trivially
 * copyable, it can be copied with memcpy(3) and similar operations. It does not need to be move or
 * copy constructed.
 *
 * Buffer is a simple buffer type, similar to std::array, but with bulk read/write operations and
 * optimizations for trivially copyable types.
 *
 * BufferT has two template overloads, one for trivially copyable element types, and one where the
 * element type is not trivially copyable. The non-trivial element implementation uses loops to move
 * data. The implementation for elements that are trivially copyable is based on memcpy.
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
      except<std::out_of_range>("Invalid index " + to_string(index) +
                                " (NUM_ELEMENTS: " + to_string(NUM_ELEMENTS));
    }
  }

  template <typename, size_t, bool>
  friend class BufferT;

 public:
  constexpr size_t size() const { return NUM_ELEMENTS; }
  constexpr size_t max_size() const { return NUM_ELEMENTS; }

  void clear() {
    ElementT default_element{};
    for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
      operator[](i) = default_element;
    }
  }

  const ElementT& read(size_t index) const { return operator[](index); }

  void read_array(size_t offset, size_t count, ElementT dest[]) const {
    validate_index(offset);
    validate_index(offset + count - 1);
    for (size_t i = 0; i < count; ++i) {
      dest[i] = operator[](i + offset);
    }
  }

  template <typename DestT>
  void read(size_t offset, size_t count, DestT& dest) const {
    validate_index(offset);
    validate_index(offset + count - 1);
    for (size_t i = 0; i < count; ++i) {
      dest[i] = operator[](i + offset);
    }
  }

  void write(size_t index, const ElementT& element) { operator[](index) = element; }

  void write(size_t index, ElementT&& element) { operator[](index) = std::move(element); }

  void write_array(size_t offset, size_t count, const ElementT src[]) {
    validate_index(offset);
    validate_index(offset + count - 1);
    for (size_t i = 0; i < count; ++i) {
      operator[](i + offset) = src[i];
    }
  }

  template <typename SrcT>
  void write(size_t offset, size_t count, const SrcT& src) {
    validate_index(offset);
    validate_index(offset + count - 1);
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

  template <size_t RHS_NUM_ELEMENTS>
  int compare(const BufferT<ElementT, RHS_NUM_ELEMENTS, is_trivially_copyable>& rhs,
              size_t count = std::numeric_limits<size_t>::max()) const {
    if constexpr (RHS_NUM_ELEMENTS < NUM_ELEMENTS) {
      for (size_t i = 0; i < std::min(RHS_NUM_ELEMENTS, count); ++i) {
        if (elements_[i] < rhs.elements_[i]) {
          return -1;
        } else if (!(elements_[i] == rhs.elements_[i])) {
          return 1;
        }
      }
      return 0;
    } else {
      for (size_t i = 0; i < std::min(NUM_ELEMENTS, count); ++i) {
        if (elements_[i] < rhs.elements_[i]) {
          return -1;
        } else if (!(elements_[i] == rhs.elements_[i])) {
          return 1;
        }
      }
      return 0;
    }
  }

  template <size_t RHS_NUM_ELEMENTS>
  bool is_equal(const BufferT<ElementT, RHS_NUM_ELEMENTS, is_trivially_copyable>& rhs,
                size_t count = std::numeric_limits<size_t>::max()) const {
    return compare(rhs, count) == 0;
  }

  template <size_t RHS_NUM_ELEMENTS>
  bool operator==(const BufferT<ElementT, RHS_NUM_ELEMENTS, is_trivially_copyable>& rhs) const {
    return compare(rhs) == 0;
  }

  template <size_t RHS_NUM_ELEMENTS>
  bool operator!=(const BufferT<ElementT, RHS_NUM_ELEMENTS, is_trivially_copyable>& rhs) const {
    return compare(rhs) != 0;
  }

  constexpr ElementT* data() noexcept { return elements_; }
  constexpr const ElementT* data() const noexcept { return elements_; }

  constexpr std::string_view as_string_view() const {
    return std::string_view(reinterpret_cast<const char*>(elements_),
                            NUM_ELEMENTS * sizeof(ElementT));
  }

  constexpr std::string_view as_string_view(size_t count) const {
    validate_index(count - 1);
    return std::string_view(reinterpret_cast<const char*>(elements_), count * sizeof(ElementT));
  }

  constexpr std::string_view as_string_view(size_t offset, size_t count) const {
    validate_index(offset + count - 1);
    return std::string_view(reinterpret_cast<const char*>(elements_ + offset),
                            count * sizeof(ElementT));
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
      except<std::out_of_range>("Invalid index " + to_string(index) +
                                " (NUM_ELEMENTS: " + to_string(NUM_ELEMENTS) + ")");
    }
  }

 public:
  constexpr size_t size() const { return NUM_ELEMENTS; }
  constexpr size_t max_size() const { return NUM_ELEMENTS; }

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

  void read_array(size_t offset, size_t count, ElementT dest[]) const {
    validate_index(offset);
    validate_index(offset + count - 1);
    std::memcpy(dest, elements_ + offset, count * sizeof(ElementT));
  }

  template <typename DestT>
  void read(size_t offset, size_t count, DestT& dest) const {
    validate_index(offset);
    validate_index(offset + count - 1);
    if (dest.max_size() < count) {
      using std::to_string;
      except<std::overflow_error>("dest has insufficient space (" + to_string(count) + " vs " +
                                  to_string(dest.max_size()) + ")");
    }

    std::memcpy(dest.data(), elements_ + offset, count * sizeof(ElementT));
  }

  void write(size_t index, const ElementT& element) { operator[](index) = element; }

  void write(size_t index, ElementT&& element) { operator[](index) = std::move(element); }

  void write_array(size_t offset, size_t count, const ElementT src[]) {
    validate_index(offset);
    validate_index(offset + count - 1);
    std::memcpy(elements_ + offset, src, count * sizeof(ElementT));
  }

  template <typename SrcT>
  void write(size_t offset, size_t count, const SrcT& src) {
    validate_index(offset);
    validate_index(offset + count - 1);
    if (src.max_size() < count) {
      using std::to_string;
      except<std::overflow_error>("src has insufficient space (" + to_string(count) + " vs " +
                                  to_string(src.max_size()) + ")");
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

  template <size_t RHS_NUM_ELEMENTS>
  int compare(const BufferT<ElementT, RHS_NUM_ELEMENTS, true>& rhs,
              size_t count = std::numeric_limits<size_t>::max()) const {
    if constexpr (RHS_NUM_ELEMENTS < NUM_ELEMENTS) {
      return std::memcmp(elements_, rhs.elements_,
                         std::min(RHS_NUM_ELEMENTS, count) * sizeof(ElementT));
    } else {
      return std::memcmp(elements_, rhs.elements_,
                         std::min(NUM_ELEMENTS, count) * sizeof(ElementT));
    }
  }

  template <size_t RHS_NUM_ELEMENTS>
  bool is_equal(const BufferT<ElementT, RHS_NUM_ELEMENTS, true>& rhs,
                size_t count = std::numeric_limits<size_t>::max()) const {
    return compare(rhs, count) == 0;
  }

  template <size_t RHS_NUM_ELEMENTS>
  bool operator==(const BufferT<ElementT, RHS_NUM_ELEMENTS, true>& rhs) const {
    return compare(rhs) == 0;
  }

  template <size_t RHS_NUM_ELEMENTS>
  bool operator!=(const BufferT<ElementT, RHS_NUM_ELEMENTS, true>& rhs) const {
    return compare(rhs) != 0;
  }

  constexpr ElementT* data() noexcept { return elements_; }
  constexpr const ElementT* data() const noexcept { return elements_; }

  constexpr std::string_view as_string_view() const {
    return std::string_view(reinterpret_cast<const char*>(elements_),
                            NUM_ELEMENTS * sizeof(ElementT));
  }

  constexpr std::string_view as_string_view(size_t count) const {
    validate_index(count - 1);
    return std::string_view(reinterpret_cast<const char*>(elements_), count * sizeof(ElementT));
  }

  constexpr std::string_view as_string_view(size_t offset, size_t count) const {
    validate_index(offset + count - 1);
    return std::string_view(reinterpret_cast<const char*>(elements_ + offset),
                            count * sizeof(ElementT));
  }
};

template <typename ElementT, size_t NUM_ELEMENTS, bool is_trivially_copyable>
std::string to_string(const BufferT<ElementT, NUM_ELEMENTS, is_trivially_copyable>& buffer) {
  using std::to_string;
  std::string result{};
  static constexpr size_t ROW_SIZE{10};
  for (size_t i = 0; i < NUM_ELEMENTS; i += ROW_SIZE) {
    for (size_t j = 0; (j < ROW_SIZE) && (i * ROW_SIZE + j < NUM_ELEMENTS); ++j) {
      result.append(to_string(buffer[i * ROW_SIZE + j])).append(" ");
    }
    result.append("\n");
  }
  return result;
}

template <size_t NUM_ELEMENTS>
std::string to_string(const BufferT<uint8_t, NUM_ELEMENTS, true>& buffer) {
  std::ostringstream ss;
  static constexpr size_t ROW_SIZE{10};
  for (size_t i = 0; i < NUM_ELEMENTS; i += ROW_SIZE) {
    for (size_t j = 0; (j < ROW_SIZE) && (i + j < NUM_ELEMENTS); ++j) {
      ss << std::hex << "0x" << std::setw(2) << std::setfill('0')
         << static_cast<uint32_t>(buffer[i + j]) << " ";
    }
    ss << "\n";
  }

  return ss.str();
}

template <typename ElementT, size_t NUM_ELEMENTS>
using Buffer = BufferT<ElementT, NUM_ELEMENTS, std::is_trivially_copyable<ElementT>::value>;

}  // namespace tvsc::buffer
