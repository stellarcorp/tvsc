#pragma once

#include <utility>

#include "control/parameter_domain.h"

namespace tvsc::control {

template <typename T>
class RangedParameterDomain final : public ParameterDomain<T> {
 private:
  T low_{};
  T high_{};
  bool include_low_{true};
  bool include_high_{true};

 public:
  RangedParameterDomain() = default;
  RangedParameterDomain(const RangedParameterDomain& rhs)
      : low_(rhs.low_),
        high_(rhs.high_),
        include_low_(rhs.include_low_),
        include_high_(rhs.include_high_) {}
  RangedParameterDomain(RangedParameterDomain&& rhs)
      : low_(std::move(rhs.low_)),
        high_(std::move(rhs.high_)),
        include_low_(rhs.include_low_),
        include_high_(rhs.include_high_) {}

  RangedParameterDomain(const T& low_value, const T& high_value, bool include_low = true,
                        bool include_high = true)
      : low_(low_value),
        high_(high_value),
        include_low_(include_low),
        include_high_(include_high) {}
  RangedParameterDomain(T&& low_value, T&& high_value, bool include_low = true,
                        bool include_high = true)
      : low_(std::move(low_value)),
        high_(std::move(high_value)),
        include_low_(include_low),
        include_high_(include_high) {}

  RangedParameterDomain& operator=(const RangedParameterDomain& rhs) {
    low_ = rhs.low_;
    high_ = rhs.high_;
    include_low_ = rhs.include_low_;
    include_high_ = rhs.include_high_;
    return *this;
  }
  RangedParameterDomain& operator=(RangedParameterDomain&& rhs) {
    low_ = std::move(rhs.low_);
    high_ = std::move(rhs.high_);
    include_low_ = rhs.include_low_;
    include_high_ = rhs.include_high_;
    return *this;
  }

  bool is_allowed(const T& value) const override {
    if (include_low_ && include_high_) {
      return value >= low_ && value <= high_;
    } else if (include_low_) {
      return value >= low_ && value < high_;
    } else if (include_high_) {
      return value > low_ && value <= high_;
    } else {
      return value > low_ && value < high_;
    }
  }
};

}  // namespace tvsc::control
