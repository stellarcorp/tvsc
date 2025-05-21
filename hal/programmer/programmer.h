#pragma once

#include <chrono>

#include "hal/peripheral.h"

namespace tvsc::hal::programmer {

using namespace std::chrono_literals;

class Programmer;

class ProgrammerPeripheral : public Peripheral<ProgrammerPeripheral, Programmer> {
 private:
  virtual void enable() = 0;
  virtual void disable() = 0;

  virtual void initiate_target_reset() = 0;
  virtual void conclude_target_reset() = 0;

  virtual void initiate_target_bootloader_boot() = 0;
  virtual void conclude_target_bootloader_boot() = 0;

  void reset_target() {}

  friend class Programmer;

 public:
  virtual ~ProgrammerPeripheral() = default;
};

class Programmer final : public Functional<ProgrammerPeripheral, Programmer> {
 private:
  Programmer(ProgrammerPeripheral& peripheral)
      : Functional<ProgrammerPeripheral, Programmer>(peripheral) {}

  friend class Peripheral<ProgrammerPeripheral, Programmer>;

 public:
  Programmer() = default;

  static constexpr std::chrono::milliseconds RESET_HOLD_TIME{5s};
  void initiate_target_reset() { return peripheral_->initiate_target_reset(); }
  void conclude_target_reset() { return peripheral_->conclude_target_reset(); }

  static constexpr std::chrono::milliseconds BOOT0_HOLD_TIME{50ms};
  void initiate_target_bootloader_boot() { return peripheral_->initiate_target_bootloader_boot(); }
  void conclude_target_bootloader_boot() { return peripheral_->conclude_target_bootloader_boot(); }
};

}  // namespace tvsc::hal::programmer
