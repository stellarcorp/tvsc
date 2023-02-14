#include <iostream>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "radio/soapy.h"

namespace tvsc::radio {

void list_devices() {
  Soapy soapy{};

  std::cout << "Soapy modules:\n";
  for (const auto& module : soapy.modules()) {
    std::cout << "\t" << module << "\n";
  }

  std::cout << "Soapy devices:\n";
  for (const auto& device : soapy.device_names()) {
    std::cout << "\t" << device << "\n";
  }
}

}  // namespace tvsc::radio

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::radio::list_devices();

  return 0;
}
