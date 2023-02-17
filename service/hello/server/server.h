#pragma once

#include "grpcpp/grpcpp.h"
#include "service/hello/common/hello.grpc.pb.h"
#include "service/hello/common/hello.pb.h"

namespace tvsc::service::hello {

class HelloServiceImpl final : public Hello::Service {
  grpc::Status say_hello(grpc::ServerContext* context, const HelloRequest* request,
                         HelloReply* reply) override;
};

}  // namespace tvsc::service::hello
