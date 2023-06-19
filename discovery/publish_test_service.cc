#include <chrono>
#include <string>
#include <thread>

#include "base/initializer.h"
#include "discovery/service_advertiser.h"
#include "discovery/service_descriptor.pb.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_int32(duration_seconds, 10,
             "How much time in seconds to maintain the test service advertisement. Use -1 to stay "
             "active until stopped manually. Defaults to 10 seconds.");
DEFINE_string(service_type, "_echo._tcp", "Service type to advertise. Defaults to '_echo._tcp'.");

namespace tvsc::discovery {

constexpr char TEST_SERVICE_NAME[] = "TVSC Test Service";

void advertise_test_service() {
  ServiceAdvertiser advertiser{};

  constexpr char domain[] = "local";
  constexpr int port{50053};
  advertiser.advertise_service(TEST_SERVICE_NAME, FLAGS_service_type, domain, port);

  if (FLAGS_duration_seconds < 0) {
    while (true) {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(1s);
    }
  } else {
    std::this_thread::sleep_for(std::chrono::seconds(FLAGS_duration_seconds));
  }
}

}  // namespace tvsc::discovery

int main(int argc, char** argv) {
  tvsc::initialize(&argc, &argv);
  tvsc::discovery::advertise_test_service();

  return 0;
}
