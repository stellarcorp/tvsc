#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>

#include "discovery/service_browser.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_string(service_types, "_http._tcp",
              "Comma-separated list of service types to listen for. Defaults to '_http._tcp'. "
              "Example values include '_http._tcp,_echo._tcp', '_device-info._tcp', "
              "'_echo._tcp,_device-info._tcp,_soapy._tcp', etc.");

namespace tvsc::discovery {

std::unordered_set<std::string> parse_service_types_flag() {
  std::unordered_set<std::string> result{};
  std::size_t begin{0};
  for (std::size_t end = 0; true /* End of loop check in the loop */; ++end) {
    begin = end;
    end = FLAGS_service_types.find(",", begin);
    const auto service_type{FLAGS_service_types.substr(begin, end - begin)};
    result.emplace(service_type);
    if (end == std::string::npos) {
      break;
    }
  }

  return result;
}

void browse() {
  auto service_types{parse_service_types_flag()};
  std::mutex m{};
  std::condition_variable condition{};
  ServiceBrowser browser{};
  for (const auto& service_type : service_types) {
    browser.add_service_type(
        service_type, [&m, &condition, &service_types](const std::string& service_type) {
          std::unique_lock lock{m};
          auto iter = service_types.find(service_type);
          if (iter != service_types.end()) {
            service_types.erase(iter);
            if (service_types.empty()) {
              lock.unlock();
              condition.notify_one();
            }
          }
        });
  }

  std::unique_lock lock{m};
  condition.wait(lock);

  std::cout << "Discovered services:\n";
  for (const auto& entry : browser.all_discovered_services()) {
    std::cout << entry.second.DebugString() << "\n";
  }
}

}  // namespace tvsc::discovery

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::discovery::browse();

  return 0;
}
