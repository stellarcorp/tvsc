#include <chrono>
#include <string>
#include <memory>

#include "gmock/gmock.h"
#include "grpcpp/grpcpp.h"
#include "service/echo/client/client.h"
#include "service/echo/common/echo.grpc.pb.h"
#include "service/echo/common/echo.pb.h"
#include "service/echo/server/server.h"
#include "service/utility/service_runner.h"
#include "service/utility/status_testing.h"

namespace tvsc::service::echo {

using namespace std::literals::chrono_literals;
using Clock = std::chrono::system_clock;

class EchoServiceTest : public ::testing::Test {
 public:
  tvsc::service::utility::ServiceRunner service_runner{};
  EchoServiceImpl service{};

  EchoServiceTest() { service_runner.add_service<Echo, EchoServiceImpl>("Echo Service", service); }

  void SetUp() override { service_runner.start(); }

  void TearDown() override { service_runner.stop(); }
};

TEST_F(EchoServiceTest, CanCallEcho) {
  EchoClient client{service_runner.bind_address()};

  EchoReply response{};
  EXPECT_OK(client.call("Some message", &response));
}

TEST_F(EchoServiceTest, CanGetEchoResponse) {
  constexpr const char MESSAGE[] = "Hi, everybody!";

  EchoClient client{service_runner.bind_address()};

  EchoRequest request{};
  request.set_msg(MESSAGE);
  EchoReply response{};

  EXPECT_OK(client.call(request, &response));
  EXPECT_EQ(request.msg(), response.msg());
}

}  // namespace tvsc::service::echo
