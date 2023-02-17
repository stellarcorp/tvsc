#pragma once

#include "grpcpp/grpcpp.h"
#include "service/echo/common/echo.grpc.pb.h"
#include "service/echo/common/echo.pb.h"

namespace tvsc::service::echo {

class EchoServiceImpl final : public Echo::Service {
  grpc::Status echo(grpc::ServerContext* context, const EchoRequest* request,
                    EchoReply* reply) override;
};

}  // namespace tvsc::service::echo
