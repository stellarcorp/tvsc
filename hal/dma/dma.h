#pragma once

namespace tvsc::hal::dma {

class Dma {
 public:
  virtual void start_circular_transfer() = 0;

  virtual void handle_interrupt() = 0;
};

}  // namespace tvsc::hal::dma
