#include <chrono>
#include <thread>

#include "discovery/service_advertiser.h"
#include "discovery/service_descriptor.pb.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

namespace tvsc::discovery {
void callback(AdvertisementResult result) {
  LOG(INFO) << "callback() -- result: "
            << static_cast<std::underlying_type_t<AdvertisementResult>>(result);
}

void advertise_test_service() {
  using namespace std::chrono_literals;

  ServiceAdvertiser advertiser{};

  ServiceSet service{};
  service.set_canonical_name("TVSC Test Service");
  ServiceDescriptor* descriptor = service.add_services();
  descriptor->set_service_type("_echo._tcp");
  descriptor->set_port(50053);

  advertiser.advertise_service(service, callback);

  std::this_thread::sleep_for(10s);
}

}  // namespace tvsc::discovery

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::discovery::advertise_test_service();

  return 0;
}
