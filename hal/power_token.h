#pragma once

#include <functional>

namespace tvsc::hal {

class PowerToken final {
 private:
  std::function<void()> callback_;

 public:
  PowerToken(std::function<void()> turn_off) : callback_(turn_off) {}
  ~PowerToken() {
    if (callback_) {
      callback_();
    }
  }
};

}  // namespace tvsc::hal
