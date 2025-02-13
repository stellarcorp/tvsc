#include <random>

#include "random.h"

namespace tvsc::random {

std::default_random_engine& engine() {
  static std::default_random_engine engine{};
  return engine;
}

}  // namespace tvsc::random
