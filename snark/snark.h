#pragma once

#include <cstddef>

namespace tvsc::snark {

const char* random_snark();
const char* snark(size_t index);
size_t num_snarks();

}  // namespace tvsc::snark
