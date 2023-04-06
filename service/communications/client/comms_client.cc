#include <exception>
#include <future>
#include <iostream>
#include <memory>
#include <string>

#include "discovery/service_resolver.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/communications/client/client.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/common/communications.pb.h"

namespace tvsc::service::communications {

void print_message(const Message& reply) {
  std::cout << "\n" << reply.message() << "\n> " << std::flush;
}

void receive_messages() {
  CommunicationsClient client{};
  grpc::ClientContext context{};
  Message reply{};
  grpc::Status status{};

  std::unique_ptr<grpc::ClientReaderInterface<Message>> stream{client.receive(&context)};
  bool success{true};
  while (success) {
    success = stream->Read(&reply);
    if (success) {
      print_message(reply);
    }
  }

  status = stream->Finish();

  if (!status.ok()) {
    using std::to_string;
    std::string msg{"Receive RPC failed -- " + to_string(status.error_code()) + ": " +
                    status.error_message()};
    LOG(ERROR) << msg;
    throw std::runtime_error(msg);
  }
}

void send_stdin() {
  while (true) {
    CommunicationsClient client{};
    std::string message{};
    SuccessResult reply{};
    grpc::Status status{};

    std::cout << "> " << std::flush;
    std::cin >> message;

    status = client.transmit(message, &reply);
    if (!status.ok()) {
      using std::to_string;
      std::string msg{"Transmit RPC failed -- " + to_string(status.error_code()) + ": " +
                      status.error_message()};
      LOG(ERROR) << msg;
      throw std::runtime_error(msg);
    }
  }
}

}  // namespace tvsc::service::communications

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::discovery::register_mdns_grpc_resolver();

  std::async(std::launch::async, tvsc::service::communications::receive_messages);

  tvsc::service::communications::send_stdin();

  return 0;
}
