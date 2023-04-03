#include <cstdint>
#include <functional>

#include "base/except.h"
#include "hal/gpio/interrupts.h"
#include "wiringPi.h"

namespace tvsc::hal::gpio {

constexpr uint16_t NUMBER_INTERRUPTS{70};
std::array<std::function<void()>, NUMBER_INTERRUPTS> functions{};

template <int PIN>
void vector_interrupt() {
  std::function<void()>& fn = functions[PIN];
  if (fn) {
    fn();
  }
}

void attach_interrupt(uint8_t pin, std::function<void()> fn) {
  functions[pin] = std::move(fn);
  switch (pin) {
    case 0:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<0>);
      break;
    case 1:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<1>);
      break;
    case 2:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<2>);
      break;
    case 3:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<3>);
      break;
    case 4:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<4>);
      break;
    case 5:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<5>);
      break;
    case 6:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<6>);
      break;
    case 7:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<7>);
      break;
    case 8:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<8>);
      break;
    case 9:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<9>);
      break;
    case 10:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<10>);
      break;
    case 11:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<11>);
      break;
    case 12:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<12>);
      break;
    case 13:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<13>);
      break;
    case 14:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<14>);
      break;
    case 15:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<15>);
      break;
    case 16:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<16>);
      break;
    case 17:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<17>);
      break;
    case 18:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<18>);
      break;
    case 19:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<19>);
      break;
    case 20:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<20>);
      break;
    case 21:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<21>);
      break;
    case 22:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<22>);
      break;
    case 23:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<23>);
      break;
    case 24:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<24>);
      break;
    case 25:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<25>);
      break;
    case 26:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<26>);
      break;
    case 27:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<27>);
      break;
    case 28:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<28>);
      break;
    case 29:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<29>);
      break;
    case 30:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<30>);
      break;
    case 31:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<31>);
      break;
    case 32:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<32>);
      break;
    case 33:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<33>);
      break;
    case 34:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<34>);
      break;
    case 35:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<35>);
      break;
    case 36:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<36>);
      break;
    case 37:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<37>);
      break;
    case 38:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<38>);
      break;
    case 39:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<39>);
      break;
    case 40:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<40>);
      break;
    case 41:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<41>);
      break;
    case 42:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<42>);
      break;
    case 43:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<43>);
      break;
    case 44:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<44>);
      break;
    case 45:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<45>);
      break;
    case 46:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<46>);
      break;
    case 47:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<47>);
      break;
    case 48:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<48>);
      break;
    case 49:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<49>);
      break;
    case 50:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<50>);
      break;
    case 51:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<51>);
      break;
    case 52:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<52>);
      break;
    case 53:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<53>);
      break;
    case 54:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<54>);
      break;
    case 55:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<55>);
      break;
    case 56:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<56>);
      break;
    case 57:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<57>);
      break;
    case 58:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<58>);
      break;
    case 59:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<59>);
      break;
    case 60:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<60>);
      break;
    case 61:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<61>);
      break;
    case 62:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<62>);
      break;
    case 63:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<63>);
      break;
    case 64:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<64>);
      break;
    case 65:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<65>);
      break;
    case 66:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<66>);
      break;
    case 67:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<67>);
      break;
    case 68:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<68>);
      break;
    case 69:
      wiringPiISR(pin, INT_EDGE_RISING, vector_interrupt<69>);
      break;
    default:
      except<std::domain_error>("Interrupt number exceeds number of interrupts configured.");
  }
}

}  // namespace tvsc::hal::gpio
