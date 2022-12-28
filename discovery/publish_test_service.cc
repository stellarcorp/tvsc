#include <chrono>
#include <thread>

#include "discovery/service_advertiser.h"
#include "discovery/service_descriptor.pb.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_int32(duration_seconds, 10,
             "How much time in seconds to maintain the test service advertisement. Use -1 to stay "
             "active until stopped manually. Defaults to 10 seconds.");

namespace tvsc::discovery {

void callback(AdvertisementResult result) {
  if (result != AdvertisementResult::SUCCESS) {
    LOG(INFO) << "callback() -- result: "
              << static_cast<std::underlying_type_t<AdvertisementResult>>(result);
  }
}

void advertise_test_service() {
  ServiceAdvertiser advertiser{};

  ServiceSet service{};
  service.set_canonical_name("TVSC Test Service");
  ServiceDescriptor* descriptor = service.add_services();
  descriptor->set_service_type("_echo._tcp");
  descriptor->set_domain(".local");
  descriptor->set_port(50053);

  advertiser.advertise_service(service, callback);

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
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::discovery::advertise_test_service();

  return 0;
}
