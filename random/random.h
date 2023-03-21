#pragma once

#include <cstdint>
#include <limits>

namespace tvsc::random {

template <typename SeedT>
inline void set_seed(SeedT seed);

template <typename ResultT>
inline ResultT generate_random_value(ResultT minimum = std::numeric_limits<ResultT>::lowest(),
                              ResultT maximum = std::numeric_limits<ResultT>::max());

}  // namespace tvsc::random

// These headers define actual specializations of the templates above that are appropriate to the
// target platform.
#ifdef ARDUINO
#include "random/arduino_random.h"
#else
#include "random/std_cpp_random.h"
#endif
