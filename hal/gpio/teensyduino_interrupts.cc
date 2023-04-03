#include <cstdint>
#include <functional>

#include "Arduino.h"
#include "base/except.h"
#include "hal/gpio/interrupts.h"

namespace tvsc::hal::gpio {

constexpr uint16_t NUMBER_INTERRUPTS{70};
std::array<std::function<void()>, NUMBER_INTERRUPTS> functions{};

template <int INTERRUPT_NUMBER>
void vector_interrupt() {
  std::function<void()>& fn = functions[INTERRUPT_NUMBER];
  if (fn) {
    fn();
  }
}

void attach_interrupt(uint8_t pin, std::function<void()> fn) {
  uint16_t interrupt_number = digitalPinToInterrupt(pin);
  functions[interrupt_number] = std::move(fn);
  switch (interrupt_number) {
    case 0:
      attachInterrupt(interrupt_number, vector_interrupt<0>, RISING);
      break;
    case 1:
      attachInterrupt(interrupt_number, vector_interrupt<1>, RISING);
      break;
    case 2:
      attachInterrupt(interrupt_number, vector_interrupt<2>, RISING);
      break;
    case 3:
      attachInterrupt(interrupt_number, vector_interrupt<3>, RISING);
      break;
    case 4:
      attachInterrupt(interrupt_number, vector_interrupt<4>, RISING);
      break;
    case 5:
      attachInterrupt(interrupt_number, vector_interrupt<5>, RISING);
      break;
    case 6:
      attachInterrupt(interrupt_number, vector_interrupt<6>, RISING);
      break;
    case 7:
      attachInterrupt(interrupt_number, vector_interrupt<7>, RISING);
      break;
    case 8:
      attachInterrupt(interrupt_number, vector_interrupt<8>, RISING);
      break;
    case 9:
      attachInterrupt(interrupt_number, vector_interrupt<9>, RISING);
      break;
    case 10:
      attachInterrupt(interrupt_number, vector_interrupt<10>, RISING);
      break;
    case 11:
      attachInterrupt(interrupt_number, vector_interrupt<11>, RISING);
      break;
    case 12:
      attachInterrupt(interrupt_number, vector_interrupt<12>, RISING);
      break;
    case 13:
      attachInterrupt(interrupt_number, vector_interrupt<13>, RISING);
      break;
    case 14:
      attachInterrupt(interrupt_number, vector_interrupt<14>, RISING);
      break;
    case 15:
      attachInterrupt(interrupt_number, vector_interrupt<15>, RISING);
      break;
    case 16:
      attachInterrupt(interrupt_number, vector_interrupt<16>, RISING);
      break;
    case 17:
      attachInterrupt(interrupt_number, vector_interrupt<17>, RISING);
      break;
    case 18:
      attachInterrupt(interrupt_number, vector_interrupt<18>, RISING);
      break;
    case 19:
      attachInterrupt(interrupt_number, vector_interrupt<19>, RISING);
      break;
    case 20:
      attachInterrupt(interrupt_number, vector_interrupt<20>, RISING);
      break;
    case 21:
      attachInterrupt(interrupt_number, vector_interrupt<21>, RISING);
      break;
    case 22:
      attachInterrupt(interrupt_number, vector_interrupt<22>, RISING);
      break;
    case 23:
      attachInterrupt(interrupt_number, vector_interrupt<23>, RISING);
      break;
    case 24:
      attachInterrupt(interrupt_number, vector_interrupt<24>, RISING);
      break;
    case 25:
      attachInterrupt(interrupt_number, vector_interrupt<25>, RISING);
      break;
    case 26:
      attachInterrupt(interrupt_number, vector_interrupt<26>, RISING);
      break;
    case 27:
      attachInterrupt(interrupt_number, vector_interrupt<27>, RISING);
      break;
    case 28:
      attachInterrupt(interrupt_number, vector_interrupt<28>, RISING);
      break;
    case 29:
      attachInterrupt(interrupt_number, vector_interrupt<29>, RISING);
      break;
    case 30:
      attachInterrupt(interrupt_number, vector_interrupt<30>, RISING);
      break;
    case 31:
      attachInterrupt(interrupt_number, vector_interrupt<31>, RISING);
      break;
    case 32:
      attachInterrupt(interrupt_number, vector_interrupt<32>, RISING);
      break;
    case 33:
      attachInterrupt(interrupt_number, vector_interrupt<33>, RISING);
      break;
    case 34:
      attachInterrupt(interrupt_number, vector_interrupt<34>, RISING);
      break;
    case 35:
      attachInterrupt(interrupt_number, vector_interrupt<35>, RISING);
      break;
    case 36:
      attachInterrupt(interrupt_number, vector_interrupt<36>, RISING);
      break;
    case 37:
      attachInterrupt(interrupt_number, vector_interrupt<37>, RISING);
      break;
    case 38:
      attachInterrupt(interrupt_number, vector_interrupt<38>, RISING);
      break;
    case 39:
      attachInterrupt(interrupt_number, vector_interrupt<39>, RISING);
      break;
    case 40:
      attachInterrupt(interrupt_number, vector_interrupt<40>, RISING);
      break;
    case 41:
      attachInterrupt(interrupt_number, vector_interrupt<41>, RISING);
      break;
    case 42:
      attachInterrupt(interrupt_number, vector_interrupt<42>, RISING);
      break;
    case 43:
      attachInterrupt(interrupt_number, vector_interrupt<43>, RISING);
      break;
    case 44:
      attachInterrupt(interrupt_number, vector_interrupt<44>, RISING);
      break;
    case 45:
      attachInterrupt(interrupt_number, vector_interrupt<45>, RISING);
      break;
    case 46:
      attachInterrupt(interrupt_number, vector_interrupt<46>, RISING);
      break;
    case 47:
      attachInterrupt(interrupt_number, vector_interrupt<47>, RISING);
      break;
    case 48:
      attachInterrupt(interrupt_number, vector_interrupt<48>, RISING);
      break;
    case 49:
      attachInterrupt(interrupt_number, vector_interrupt<49>, RISING);
      break;
    case 50:
      attachInterrupt(interrupt_number, vector_interrupt<50>, RISING);
      break;
    case 51:
      attachInterrupt(interrupt_number, vector_interrupt<51>, RISING);
      break;
    case 52:
      attachInterrupt(interrupt_number, vector_interrupt<52>, RISING);
      break;
    case 53:
      attachInterrupt(interrupt_number, vector_interrupt<53>, RISING);
      break;
    case 54:
      attachInterrupt(interrupt_number, vector_interrupt<54>, RISING);
      break;
    case 55:
      attachInterrupt(interrupt_number, vector_interrupt<55>, RISING);
      break;
    case 56:
      attachInterrupt(interrupt_number, vector_interrupt<56>, RISING);
      break;
    case 57:
      attachInterrupt(interrupt_number, vector_interrupt<57>, RISING);
      break;
    case 58:
      attachInterrupt(interrupt_number, vector_interrupt<58>, RISING);
      break;
    case 59:
      attachInterrupt(interrupt_number, vector_interrupt<59>, RISING);
      break;
    case 60:
      attachInterrupt(interrupt_number, vector_interrupt<60>, RISING);
      break;
    case 61:
      attachInterrupt(interrupt_number, vector_interrupt<61>, RISING);
      break;
    case 62:
      attachInterrupt(interrupt_number, vector_interrupt<62>, RISING);
      break;
    case 63:
      attachInterrupt(interrupt_number, vector_interrupt<63>, RISING);
      break;
    case 64:
      attachInterrupt(interrupt_number, vector_interrupt<64>, RISING);
      break;
    case 65:
      attachInterrupt(interrupt_number, vector_interrupt<65>, RISING);
      break;
    case 66:
      attachInterrupt(interrupt_number, vector_interrupt<66>, RISING);
      break;
    case 67:
      attachInterrupt(interrupt_number, vector_interrupt<67>, RISING);
      break;
    case 68:
      attachInterrupt(interrupt_number, vector_interrupt<68>, RISING);
      break;
    case 69:
      attachInterrupt(interrupt_number, vector_interrupt<69>, RISING);
      break;
    default:
      except<std::domain_error>("Interrupt number exceeds number of interrupts configured.");
  }
}

}  // namespace tvsc::hal::gpio
