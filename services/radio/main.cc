#include "gflags/gflags.h"
#include "glog/logging.h"
#include "soapy_server.h"

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  return run_soapy_server();
}
