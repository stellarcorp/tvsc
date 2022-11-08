#include <iostream>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "services/radio/server/soapy.h"

namespace tvsc::services::radio::server {

void list_devices() {
  Soapy soapy{};

  std::cout << "Soapy modules:\n";
  for (const auto& module : soapy.modules()) {
    std::cout << "\t" << module << "\n";
  }

  std::cout << "Soapy devices:\n";
  for (const auto& device : soapy.devices()) {
    std::cout << "\t" << device << "\n";
  }

  LOG_IF(ERROR, !soapy.contains_module("libdummy_radio.so")) << "'dummy_radio' module not found";

  LOG_IF(ERROR, !soapy.has_device("dummy_receiver")) << "'dummy_receiver' device not found";
}

}  // namespace tvsc::services::radio::server

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::services::radio::server::list_devices();

  return 0;
}
