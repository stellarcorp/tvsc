#pragma once

#include <functional>

namespace tvsc::hal {

class EnableLock final {
 private:
  std::function<void()> callback_;

 public:
  EnableLock(std::function<void()> turn_off) : callback_(turn_off) {}
  ~EnableLock() {
    if (callback_) {
      callback_();
    }
  }
};

}  // namespace tvsc::hal
