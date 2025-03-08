#pragma once

#include <string>

namespace tvsc::io {

std::string generate_temp_filename(const std::string& prefix, const std::string& suffix);

}  // namespace tvsc::io
