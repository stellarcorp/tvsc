#include <memory>
#include <string>

#include "discovery/service_types.h"
#include "grpcpp/grpcpp.h"
#include "services/datetime/common/datetime.grpc.pb.h"

namespace tvsc::service::datetime {

class DatetimeClient {
 public:
  DatetimeClient() : DatetimeClient(tvsc::discovery::default_bind_address<Datetime>()) {}

  DatetimeClient(const std::string& bind_addr)
      : stub_(
            Datetime::NewStub(grpc::CreateChannel(bind_addr, grpc::InsecureChannelCredentials()))) {
  }

  grpc::Status call(DatetimeReply* reply) { return call(DatetimeRequest::MILLISECOND, reply); }

  grpc::Status call(DatetimeRequest::Precision precision, DatetimeReply* reply) {
    grpc::ClientContext context{};
    DatetimeRequest request{};
    request.set_precision(precision);
    return call(&context, request, reply);
  }

  grpc::Status call(grpc::ClientContext* context, const DatetimeRequest& request,
                    DatetimeReply* reply) {
    return stub_->get_datetime(context, request, reply);
  }

 private:
  std::unique_ptr<Datetime::Stub> stub_;
};

}  // namespace tvsc::service::datetime
