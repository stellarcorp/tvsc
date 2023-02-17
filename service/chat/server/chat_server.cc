#include "gflags/gflags.h"
#include "glog/logging.h"
#include "service/chat/common/chat.grpc.pb.h"
#include "service/chat/server/server.h"
#include "service/utility/service_runner.h"

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  using namespace tvsc::service::chat;
  tvsc::service::utility::run_single_service<Chat, ChatServiceImpl>("TVSC Chat Service");

  return 0;
}
