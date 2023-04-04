#include <cstdint>

#include "hal/output/output.h"
#include "hal/time/time.h"
#include "random/random.h"

int main() {
  tvsc::random::initialize_seed();

  while (true) {
    uint64_t value{tvsc::random::generate_random_value<uint64_t>()};
    tvsc::hal::output::println(value);
    tvsc::hal::time::delay_ms(500);
  }

  return 0;
}
