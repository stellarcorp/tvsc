#include "service/hello/server/server.h"

#include <string>

#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/hello/common/hello.pb.h"

namespace tvsc::service::hello {

grpc::Status HelloServiceImpl::say_hello(grpc::ServerContext* context, const HelloRequest* request,
                                         HelloReply* reply) {
  const std::string name{request->name()};
  LOG(INFO) << "Request received -- name: '" << name << "'";
  reply->set_msg("Hello, " + name + "!");
  return grpc::Status::OK;
}

}  // namespace tvsc::service::hello
