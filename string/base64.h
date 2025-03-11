#pragma once

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace tvsc::string {

class Base64 final {
 private:
  static bool valid_character(char c) noexcept {
    if (c >= 'A' && c <= 'Z') {
      return true;
    } else if (c >= 'a' && c <= 'z') {
      return true;
    } else if (c >= '0' && c <= '9') {
      return true;
    } else if (c == '-') {
      return true;
    } else if (c == '_') {
      return true;
    } else {
      return false;
    }
  }

 public:
  // 11 bytes is the maximum size for a 64-bit encoded as a string in base 64. A uint64_t has 64
  // bits; a base-64 byte holds 6 bits. 64/6 = 10.67, so we round up to 11 bytes.
  static constexpr size_t MAX_ENCODED_STRING_SIZE{11};

  static std::string encode(uint64_t value) noexcept {
    static const std::string base64_chars{
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789-_"};

    std::string encoded{};
    // Reserve the maximum possible string size. Note that this
    // prevents repeated reallocations, assuming no small string optimization (SSO). Under SSO, we
    // will likely have 15 bytes or so reserved already, so this reserve() call would be a no-op.
    // TODO(james): Replace with constexpr check when we can support constexpr std::string.
    if (encoded.capacity() < MAX_ENCODED_STRING_SIZE) {
      encoded.reserve(MAX_ENCODED_STRING_SIZE);
    }
    if (value == 0) {
      encoded = "A";
    } else {
      while (value > 0) {
        encoded = base64_chars[value % 64] + encoded;
        value /= 64;
      }
    }
    return encoded;
  }

  static uint64_t decode(const std::string& str) {
    static const std::array<int8_t, 128> base64_map = []() {
      std::array<int8_t, 128> map{};
      std::fill(map.begin(), map.end(), -1);
      for (int8_t i = 0; i < 64; ++i) {
        map[static_cast<int8_t>(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789-_"[i])] = i;
      }
      return map;
    }();

    uint64_t value = 0;
    for (char c : str) {
      if (!valid_character(c)) {
        throw std::invalid_argument("Invalid Base64 character: '" + std::string(1, c) +
                                    "' (full encoded value: '" + str + "')");
      }
      value = 64 * value + base64_map[c];
    }
    return value;
  }
};  // namespace tvsc::string

}  // namespace tvsc::string
