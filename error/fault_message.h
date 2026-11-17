#pragma once

#include <array>
#include <charconv>
#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>

namespace tvsc::error {

template <std::size_t TOTAL_SIZE, typename CharT = char, typename Traits = std::char_traits<CharT>>
class FaultMessage final {
 private:
  static constexpr std::size_t CAPACITY{TOTAL_SIZE - sizeof(std::size_t)};
  std::size_t size_{};
  std::array<CharT, CAPACITY> buffer_{};

 public:
  static constexpr std::size_t capacity() noexcept { return CAPACITY; }

  constexpr FaultMessage() noexcept = default;
  constexpr FaultMessage(const char* const msg) noexcept { append(msg); }

  constexpr void clear() noexcept {
    size_ = 0;
    buffer_[0] = CharT{};
  }

  constexpr FaultMessage& append(CharT c) noexcept {
    if (size_ + 1 < CAPACITY) {
      buffer_[size_++] = c;
      buffer_[size_] = CharT{};
    }
    return *this;
  }

  constexpr FaultMessage& append(std::basic_string_view<CharT, Traits> sv) noexcept {
    if (size_ + 1 < CAPACITY) {
      std::size_t available{CAPACITY - 1 - size_};
      std::size_t to_copy = std::min(sv.size(), available);
      Traits::copy(buffer_.data() + size_, sv.data(), to_copy);
      size_ += to_copy;
      buffer_[size_] = CharT{};
    }
    return *this;
  }

  template <std::integral T>
  constexpr FaultMessage& append(T val) noexcept {
    if (size_ + 1 < CAPACITY) {
      // Target the available capacity, leaving at least one byte for null-termination.
      CharT* first{buffer_.data() + size_};
      CharT* last{buffer_.data() + CAPACITY - 1};

      auto [ptr, ec] = std::to_chars(first, last, val);
      if (ec == std::errc{}) {
        size_ += static_cast<std::size_t>(ptr - first);
        buffer_[size_] = CharT{};
      }
    }
    return *this;
  }

  constexpr operator std::basic_string_view<CharT, Traits>() const noexcept {
    return std::basic_string_view<CharT, Traits>(buffer_.data(), size_);
  }

  constexpr const CharT* data() const noexcept { return buffer_.data(); }
  constexpr std::size_t size() const noexcept { return size_; }
};

extern FaultMessage<256> fault_message;

}  // namespace tvsc::error
