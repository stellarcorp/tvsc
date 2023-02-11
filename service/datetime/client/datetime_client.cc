#include <chrono>
#include <ctime>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>

#include "discovery/service_resolver.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/datetime/client/client.h"
#include "service/datetime/common/datetime.grpc.pb.h"
#include "service/datetime/common/datetime_utils.h"

DEFINE_bool(continuous_update, false,
            "Update the time continuously? If true, will stream the time from the server.");

namespace tvsc::service::datetime {

template <typename Clock>
void print_time(const DatetimeReply& reply) {
  std::chrono::time_point<Clock> some_time{as_time_point<Clock>(reply.datetime(), reply.unit())};
  std::time_t as_time_t{std::chrono::system_clock::to_time_t(some_time)};
  std::cout << std::put_time(std::localtime(&as_time_t), "%F %T") << "\n";
}

void get_and_print_time() {
  using Clock = std::chrono::system_clock;

  DatetimeClient client{};
  grpc::ClientContext context{};
  DatetimeRequest request{};
  DatetimeReply reply{};
  grpc::Status status{};

  request.set_precision(TimeUnit::MILLISECOND);

  if (FLAGS_continuous_update) {
    request.set_period_count(10);
    request.set_period_unit(TimeUnit::SECOND);
    std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> stream{
        client.stream(&context, request)};
    bool success{true};
    while (success) {
      success = stream->Read(&reply);
      if (success) {
        print_time<Clock>(reply);
      }
    }
    status = stream->Finish();
  } else {
    status = client.call(&context, request, &reply);
    if (status.ok()) {
      print_time<Clock>(reply);
    }
  }

  if (!status.ok()) {
    using std::to_string;
    std::string msg{"RPC failed -- " + to_string(status.error_code()) + ": " +
                    status.error_message()};
    LOG(ERROR) << msg;
    throw std::runtime_error(msg);
  }
}

}  // namespace tvsc::service::datetime

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::discovery::register_mdns_grpc_resolver();

  tvsc::service::datetime::get_and_print_time();

  return 0;
}
