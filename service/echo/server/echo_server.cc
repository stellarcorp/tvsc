#include <memory>
#include <string>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/echo/common/echo.grpc.pb.h"
#include "service/utility/service_runner.h"

namespace tvsc::service::echo {

class EchoServiceImpl final : public Echo::Service {
  grpc::Status echo(grpc::ServerContext* context, const EchoRequest* request,
                    EchoReply* reply) override {
    const std::string& msg{request->msg()};
    reply->set_msg(msg);
    LOG(INFO) << "Received msg: '" << msg << "'";
    return grpc::Status::OK;
  }
};

}  // namespace tvsc::service::echo

int main(int argc, char** argv) {
  using namespace tvsc::service::echo;

  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  using namespace tvsc::service::echo;
  tvsc::service::utility::run_single_service<Echo, EchoServiceImpl>("TVSC Echo Service");

  return 0;
}
