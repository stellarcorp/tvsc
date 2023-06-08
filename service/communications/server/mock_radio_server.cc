#include "service/communications/server/mock_radio_server.h"

#include <chrono>
#include <condition_variable>
#include <ctime>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>

#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "random/random.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/common/communications.pb.h"

namespace tvsc::service::communications {

using namespace std::literals::chrono_literals;

MockRadioCommunicationsService::MockRadioCommunicationsService() {
}

grpc::Status MockRadioCommunicationsService::list_radios(grpc::ServerContext* context,
                                                    const EmptyMessage* request, Radios* reply) {
  LOG(INFO) << "list_radios() called.";
  return grpc::Status::OK;
}

grpc::Status MockRadioCommunicationsService::transmit(grpc::ServerContext* context,
                                                 const Message* request, SuccessResult* reply) {
  LOG(INFO) << "transmit() called. Message: " << request->message();
  return grpc::Status::OK;
}

grpc::Status MockRadioCommunicationsService::receive(grpc::ServerContext* context,
                                                const EmptyMessage* /*request*/,
                                                grpc::ServerWriter<Message>* writer) {
  using namespace std::literals::chrono_literals;
  LOG(INFO) << "MockRadioCommunicationsService::receive()";
  return grpc::Status::OK;
}

}  // namespace tvsc::service::communications
