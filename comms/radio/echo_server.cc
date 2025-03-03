#include "comms/radio/rf69hcw.h"
#include "comms/radio/transceiver.h"
#include "hal/gpio/pins.h"
#include "time/clock.h"
#include "random/random.h"

int main() {
  tvsc::random::initialize_seed();
  tvsc::hal::gpio::initialize_gpio();

  tvsc::comms::radio::Transceiver<tvsc::comms::radio::RF69HCW> transceiver{};

  transceiver.print_configuration();

  tvsc::time::Clock clock{};
  while (true) {
    transceiver.iterate(clock.current_time_millis());
  }

  return 0;
}
