#include <iostream>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "radio/soapy.h"

namespace tvsc::service::radio::server {

void list_devices() {
  tvsc::radio::Soapy soapy{};

  std::cout << "Soapy modules:\n";
  for (const auto& module : soapy.modules()) {
    std::cout << "\t" << module << "\n";
  }

  std::cout << "Soapy devices:\n";
  for (const auto& device : soapy.device_names()) {
    std::cout << "\t" << device << "\n";
  }

  LOG_IF(ERROR, !soapy.contains_module("libdummy_radio.so")) << "'dummy_radio' module not found";

  LOG_IF(ERROR, !soapy.has_device("dummy_receiver")) << "'dummy_receiver' device not found";
}

}  // namespace tvsc::service::radio::server

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::service::radio::server::list_devices();

  return 0;
}