#include <iostream>
#include <string>

#include "base/initializer.h"
#include "gflags/gflags.h"
#include "hal/simulation/simulation.pb.h"
#include "proto/proto_file_reader.h"

using namespace tvsc::hal::simulation;

DEFINE_string(log_file_name, "", "Path to log file");

void print(const Event& msg) {
  using std::to_string;
  std::string result{};
  result.append(to_string(msg.timestamp_sec())).append(": ");
  if (msg.has_fn()) {
    const Function& fn{msg.fn()};
    result.append(fn.name())
        .append(" (")
        .append(fn.source_file())
        .append(":")
        .append(to_string(fn.line_number()))
        .append(")");
  } else if (msg.has_irq()) {
    const Irq& irq{msg.irq()};
    result.append(to_string(irq.irq_number()));
  }
  std::cout << result << "\n";
}

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  tvsc::proto::ProtoFileReader reader{FLAGS_log_file_name};
  Event msg{};
  while (reader.read_message(msg)) {
    print(msg);
    msg.Clear();
  }
}
