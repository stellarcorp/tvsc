#include "App.h"
#include "base/initializer.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "http/static_file_server.h"

DEFINE_int32(port, 8080, "Port to listen on.");
DEFINE_string(doc_root, ".", "Location of static files.");

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  uWS::App app{};

  tvsc::http::serve_static_files("/*", app);

  app.listen(FLAGS_port,
             [](auto* listen_socket) {
               if (listen_socket) {
                 LOG(INFO) << "Listening on port " << FLAGS_port;
               }
             })
      .run();

  return 0;
}
