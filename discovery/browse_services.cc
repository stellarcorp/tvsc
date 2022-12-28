#include <chrono>
#include <string>
#include <thread>
#include <unordered_set>

#include "discovery/service_browser.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_int32(duration_seconds, 10,
             "How much time in seconds to give the browser time to discover services. Defaults to "
             "10 seconds.");

DEFINE_string(service_types, "_http._tcp",
              "Comma-separated list of service types to listen for. Defaults to '_http._tcp'. "
              "Example values include '_http._tcp,'");

namespace tvsc::discovery {

std::unordered_set<std::string> parse_service_types_flag() {
  std::unordered_set<std::string> result{};
  std::size_t begin{0};
  for (std::size_t end = 0; true /* End of loop check in the loop */; ++end) {
    begin = end;
    end = FLAGS_service_types.find(",", begin);
    const auto service_type{FLAGS_service_types.substr(begin, end - begin)};
    LOG(INFO) << "Found service type '" << service_type << "'";
    result.emplace(service_type);
    if (end == std::string::npos) {
      break;
    }
  }

  return result;
}

void browse() {
  ServiceBrowser browser{parse_service_types_flag()};
  LOG(INFO) << "Listening for services.";
  std::this_thread::sleep_for(std::chrono::seconds(FLAGS_duration_seconds));
  LOG(INFO) << "Discovered services:";
  for (const auto& name : browser.service_names()) {
    LOG(INFO) << name;
  }
}

}  // namespace tvsc::discovery

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::discovery::browse();

  return 0;
}
