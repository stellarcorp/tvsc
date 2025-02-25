#include "hal/rcc/rcc_noop.h"

namespace tvsc::hal::rcc {

void RccNoop::set_clock_to_max_speed() {}

void RccNoop::set_clock_to_energy_efficient_speed() {}

void RccNoop::set_clock_to_min_speed() {}

void RccNoop::restore_clock_speed() {}

}  // namespace tvsc::hal::rcc
