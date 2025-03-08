#include "io/temp_filename.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace tvsc::io {

std::string generate_temp_filename(const std::string& prefix, const std::string& suffix) {
  // Get current local time
  const std::time_t t = std::time(nullptr);
  std::tm localTime{};
  localtime_r(&t, &localTime);

  // Format timestamp
  std::ostringstream timestamp{};
  timestamp << std::put_time(&localTime, "%Y%m%d_%H%M%S");

  // Create unique subdirectory under /tmp
  const std::string dir_template{"/tmp/tvsc_" + timestamp.str() + "_XXXXXX"};
  char dir_path[dir_template.size() + 1];
  std::strcpy(dir_path, dir_template.c_str());

  if (mkdtemp(dir_path) == nullptr) {
    std::string msg{"Failed to create temporary directory -- '"};
    msg.append(dir_path).append("'");
    throw std::runtime_error(msg);
  }

  // Generate filename
  std::ostringstream filename;
  filename << dir_path << "/" << prefix << "_" << timestamp.str() << suffix;
  return filename.str();
}

}  // namespace tvsc::io
