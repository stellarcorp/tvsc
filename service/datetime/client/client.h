#pragma once

#include <memory>
#include <string>

#include "discovery/service_types.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/datetime/common/datetime.grpc.pb.h"
#
namespace tvsc::service::datetime {

class DatetimeClient {
 public:
  DatetimeClient() : DatetimeClient(tvsc::discovery::service_url<Datetime>()) {}

  DatetimeClient(const std::string& bind_addr)
      : stub_(
            Datetime::NewStub(grpc::CreateChannel(bind_addr, grpc::InsecureChannelCredentials()))) {
    DLOG(INFO) << "DatetimeClient::DatetimeClient(std::string)";
  }

  grpc::Status call(DatetimeReply* reply) { return call(TimeUnit::MILLISECOND, reply); }

  grpc::Status call(TimeUnit precision, DatetimeReply* reply) {
    grpc::ClientContext context{};
    DatetimeRequest request{};
    request.set_precision(precision);
    return call(&context, request, reply);
  }

  grpc::Status call(grpc::ClientContext* context, const DatetimeRequest& request,
                    DatetimeReply* reply) {
    return stub_->get_datetime(context, request, reply);
  }

  std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> stream(
      grpc::ClientContext* context, const DatetimeRequest& request) {
    return stub_->stream_datetime(context, request);
  }

  void stream(grpc::ClientContext* context, const DatetimeRequest* request,
              grpc::ClientReadReactor<DatetimeReply>* reactor) {
    stub_->async()->stream_datetime(context, request, reactor);
  }

 private:
  std::unique_ptr<Datetime::Stub> stub_;
};

}  // namespace tvsc::service::datetime
