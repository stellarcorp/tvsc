#pragma once

#include <array>
#include <cstdint>

namespace tvsc::message {

template <typename ElementT, size_t CAPACITY, bool PRIORITIZE_EXISTING_ELEMENTS = true>
class RingBuffer final {
 public:
  using ElementType = ElementT;

 private:
  size_t begin_{};
  size_t end_{};
  std::array<ElementType, CAPACITY> elements_{};

 public:
  bool is_empty() const { return end_ == begin_; }
  size_t size() const { return end_ - begin_; }
  constexpr size_t capacity() const { return elements_.size(); }

  [[nodiscard]] bool push(const ElementType& msg) {
    if constexpr (PRIORITIZE_EXISTING_ELEMENTS) {
      if (end_ - begin_ < elements_.size()) {
        elements_[end_ % elements_.size()] = msg;
        ++end_;
        return true;
      } else {
        return false;
      }
    } else {
      elements_[end_ % elements_.size()] = msg;
      ++end_;
      // This variation always succeeds.
      return true;
    }
  }

  const auto& peek(size_t index = 0) const {
    return elements_[(begin_ + index) % elements_.size()];
  }

  void pop() {
    if (end_ - begin_ > 0) {
      ++begin_;
    }
  }
};

}  // namespace tvsc::message
