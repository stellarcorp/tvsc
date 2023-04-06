#pragma once

#include "grpcpp/grpcpp.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/common/communications.pb.h"

namespace tvsc::service::communications {

class CommunicationsServiceImpl final : public CommunicationsService::Service {
 public:
  grpc::Status list_radios(grpc::ServerContext* context, const EmptyMessage* request,
                           Radios* reply) override;

  grpc::Status transmit(grpc::ServerContext* context, const Message* request,
                        SuccessResult* reply) override;

  grpc::Status receive(grpc::ServerContext* context, const EmptyMessage* request,
                       grpc::ServerWriter<Message>* writer) override;

  // grpc::Status get_radio_settings(grpc::ServerContext* context,
  //                                 const tvsc::radio::RadioIdentification* request,
  //                                 tvsc::radio::Radio* reply) override;

  // grpc::Status modify_radio_settings(grpc::ServerContext* context,
  //                                    const tvsc::radio::RadioSettings* request,
  //                                    SuccessResult* reply) override;

  // grpc::Status reset_radio(grpc::ServerContext* context,
  //                          const tvsc::radio::RadioIdentification* request,
  //                          SuccessResult* reply) override;
};

}  // namespace tvsc::service::communications
