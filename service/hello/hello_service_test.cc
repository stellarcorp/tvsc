#include <chrono>
#include <memory>
#include <string>

#include "gmock/gmock.h"
#include "grpcpp/grpcpp.h"
#include "service/hello/client/client.h"
#include "service/hello/common/hello.grpc.pb.h"
#include "service/hello/common/hello.pb.h"
#include "service/hello/server/server.h"
#include "service/utility/service_runner.h"
#include "service/utility/status_testing.h"

namespace tvsc::service::hello {

using namespace std::literals::chrono_literals;
using Clock = std::chrono::system_clock;

class HelloServiceTest : public ::testing::Test {
 public:
  tvsc::service::utility::ServiceRunner service_runner{};
  HelloServiceImpl service{};

  HelloServiceTest() {
    service_runner.add_service<Hello, HelloServiceImpl>("Hello World Service", service);
  }

  void SetUp() override { service_runner.start(/* advertise_services */ false); }

  void TearDown() override { service_runner.stop(); }
};

TEST_F(HelloServiceTest, CanCallHello) {
  HelloClient client{service_runner.bind_address()};

  HelloReply response{};
  EXPECT_OK(client.call("Some message", &response));
}

TEST_F(HelloServiceTest, CanGetHelloResponse) {
  constexpr const char NAME[] = "Fred";

  HelloClient client{service_runner.bind_address()};

  HelloRequest request{};
  request.set_name(NAME);
  HelloReply response{};
  grpc::ClientContext context{};

  EXPECT_OK(client.call(&context, request, &response));
  EXPECT_TRUE(response.msg().find(request.name()) != std::string::npos);
}

}  // namespace tvsc::service::hello
