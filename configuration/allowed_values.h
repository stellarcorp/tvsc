#pragma once

#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "configuration/types.h"

namespace tvsc::configuration {

class AllowedValues final {
 public:
 private:
  std::vector<DiscreteValue> enumerated_{};
  std::vector<RangedValue> ranged_{};

 public:
  AllowedValues() = default;

  AllowedValues(std::initializer_list<int32_t> allowed_values) {
    for (auto v : allowed_values) {
      enumerated_.emplace_back(v);
    }
  }

  AllowedValues(std::initializer_list<int64_t> allowed_values) {
    for (auto v : allowed_values) {
      enumerated_.emplace_back(v);
    }
  }

  AllowedValues(std::initializer_list<float> allowed_values) {
    for (auto v : allowed_values) {
      enumerated_.emplace_back(v);
    }
  }

  AllowedValues(std::initializer_list<double> allowed_values) {
    for (auto v : allowed_values) {
      enumerated_.emplace_back(v);
    }
  }

  AllowedValues(std::initializer_list<std::string_view> allowed_values) {
    for (auto v : allowed_values) {
      enumerated_.emplace_back(std::string{v});
    }
  }

  AllowedValues(std::initializer_list<ValueRange<int32_t>> allowed_values) {
    for (const auto& v : allowed_values) {
      ranged_.emplace_back(v);
    }
  }

  AllowedValues(std::initializer_list<ValueRange<int64_t>> allowed_values) {
    for (const auto& v : allowed_values) {
      ranged_.emplace_back(v);
    }
  }

  AllowedValues(std::initializer_list<ValueRange<float>> allowed_values) {
    for (const auto& v : allowed_values) {
      ranged_.emplace_back(v);
    }
  }

  AllowedValues(std::initializer_list<ValueRange<double>> allowed_values) {
    for (const auto& v : allowed_values) {
      ranged_.emplace_back(v);
    }
  }

  template <typename EnumT>
  AllowedValues(std::initializer_list<EnumT> allowed_values) : enumerated_(), ranged_() {
    for (auto v : allowed_values) {
      enumerated_.emplace_back(as_int(v));
    }
  }

  AllowedValues(const AllowedValues& rhs) = default;
  AllowedValues(AllowedValues&& rhs) = default;
  AllowedValues& operator=(const AllowedValues& rhs) = default;
  AllowedValues& operator=(AllowedValues&& rhs) = default;

  bool operator==(const AllowedValues& rhs) const {
    return enumerated_ == rhs.enumerated_ && ranged_ == rhs.ranged_;
  }

  bool is_allowed(const DiscreteValue& value) const {
    return std::visit([this](auto&& arg) -> bool { return this->is_allowed(arg); }, value);
  }

  bool is_allowed(int32_t value) const {
    for (const auto& v : enumerated_) {
      if (std::get_if<int32_t>(&v) != nullptr && *std::get_if<int32_t>(&v) == value) {
        return true;
      }
    }
    for (const auto& v : ranged_) {
      if (std::get_if<ValueRange<int32_t>>(&v) != nullptr &&
          std::get_if<ValueRange<int32_t>>(&v)->first <= value &&
          std::get_if<ValueRange<int32_t>>(&v)->second >= value) {
        return true;
      }
    }
    return is_allowed(static_cast<int64_t>(value));
  }

  bool is_allowed(int64_t value) const {
    for (const auto& v : enumerated_) {
      if (std::get_if<int64_t>(&v) != nullptr && *std::get_if<int64_t>(&v) == value) {
        return true;
      }
    }
    for (const auto& v : ranged_) {
      if (std::get_if<ValueRange<int64_t>>(&v) != nullptr &&
          std::get_if<ValueRange<int64_t>>(&v)->first <= value &&
          std::get_if<ValueRange<int64_t>>(&v)->second >= value) {
        return true;
      }
    }
    return false;
  }

  bool is_allowed(float value) const {
    for (const auto& v : enumerated_) {
      if (std::get_if<float>(&v) != nullptr && *std::get_if<float>(&v) == value) {
        return true;
      }
    }
    for (const auto& v : ranged_) {
      if (std::get_if<ValueRange<float>>(&v) != nullptr &&
          std::get_if<ValueRange<float>>(&v)->first <= value &&
          std::get_if<ValueRange<float>>(&v)->second > value) {
        return true;
      }
    }
    return is_allowed(static_cast<double>(value));
  }

  bool is_allowed(double value) const {
    for (const auto& v : enumerated_) {
      if (std::get_if<double>(&v) != nullptr && *std::get_if<double>(&v) == value) {
        return true;
      }
    }
    for (const auto& v : ranged_) {
      if (std::get_if<ValueRange<double>>(&v) != nullptr &&
          std::get_if<ValueRange<double>>(&v)->first <= value &&
          std::get_if<ValueRange<double>>(&v)->second > value) {
        return true;
      }
    }
    return false;
  }

  bool is_allowed(std::string_view value) const {
    for (const auto& v : enumerated_) {
      if (std::get_if<std::string>(&v) != nullptr && *std::get_if<std::string>(&v) == value) {
        return true;
      }
    }
    return false;
  }

  bool is_allowed(const std::string& value) const {
    for (const auto& v : enumerated_) {
      if (std::get_if<std::string>(&v) != nullptr && *std::get_if<std::string>(&v) == value) {
        return true;
      }
    }
    return false;
  }

  template <typename ValueT>
  bool is_allowed(ValueT value) const {
    static_assert(std::is_enum<ValueT>::value);
    for (const auto& v : enumerated_) {
      if (std::get_if<std::underlying_type_t<ValueT>>(&v) != nullptr &&
          *std::get_if<std::underlying_type_t<ValueT>>(&v) ==
              std::underlying_type_t<ValueT>(value)) {
        return true;
      }
    }
    return false;
  }

  constexpr const std::vector<DiscreteValue>& enumerated_values() const { return enumerated_; }
  void add_enumerated_value(const DiscreteValue& value) { enumerated_.push_back(value); }

  constexpr const std::vector<RangedValue>& ranged_values() const { return ranged_; }
  void add_ranged_value(const RangedValue& value) { ranged_.push_back(value); }
};

std::string to_string(const AllowedValues& values);

}  // namespace tvsc::configuration
