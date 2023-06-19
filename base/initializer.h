#pragma once

#include <string>

namespace tvsc {

/**
 * Consolidated initialization appropriate for most executables. This function sets up logging,
 * command line flags, and similar systems.
 *
 * Usually this is called as the first line in main() as:
 *
 *   tvsc::initialize(&argc, &argv);
 *
 */
void initialize(int* argc, char** argv[], const std::string& log_dir = "/var/log/tvsc");

}  // namespace tvsc
