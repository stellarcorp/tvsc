#pragma once

namespace tvsc::service::configuration {

// TODO(james): Issue 13 (https://gitlab.com/tvsc/tvsc/-/issues/13) -- Publish service location via Avahi or similar
// broadcast discovery system.
enum class DefaultPort : int {
  PROXY_SERVICE = 50050,
  HELLO_SERVICE = 50051,
  ECHO_SERVICE = 50052,
  VIRTUAL_TOWER_UI = 50053,
  DATETIME_SERVICE = 50054,
};

}  // namespace tvsc::service::configuration
