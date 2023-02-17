#include "service/echo/server/server.h"

#include <string>

#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/echo/common/echo.pb.h"

namespace tvsc::service::echo {

grpc::Status EchoServiceImpl::echo(grpc::ServerContext* context, const EchoRequest* request,
                                   EchoReply* reply) {
  const std::string& msg{request->msg()};
  reply->set_msg(msg);
  LOG(INFO) << "Received msg: '" << msg << "'";
  return grpc::Status::OK;
}

}  // namespace tvsc::service::echo
