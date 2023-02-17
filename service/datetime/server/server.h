#pragma once

#include "grpcpp/grpcpp.h"
#include "service/datetime/common/datetime.grpc.pb.h"
#include "service/datetime/common/datetime.pb.h"

namespace tvsc::service::datetime {

class DatetimeServiceImpl final : public Datetime::Service {
  grpc::Status get_datetime(grpc::ServerContext* context, const DatetimeRequest* request,
                      DatetimeReply* reply) override;

  grpc::Status stream_datetime(grpc::ServerContext* context, const DatetimeRequest* request,
                         grpc::ServerWriter<DatetimeReply>* writer) override;
};

}  // namespace tvsc::service::datetime
