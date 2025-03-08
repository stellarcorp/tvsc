#pragma once

#ifdef __has_include
#if __has_include(<source_location>)
#include <source_location>
#endif
#endif

#include "hal/simulation/simulation.pb.h"
#include "io/temp_filename.h"
#include "proto/proto_file_writer.h"

namespace tvsc::hal::simulation {

template <typename ClockType>
class Logger final {
 private:
  tvsc::proto::ProtoFileWriter writer_;
  const std::string filename_;

 public:
  Logger(const std::string& filename) : writer_(filename), filename_(filename) {}

  Logger() : Logger(tvsc::io::generate_temp_filename("sim", ".log.pb")) {}

  const std::string& log_file_name() const noexcept { return filename_; }

#if __cpp_lib_source_location >= 201907L

  void log_fn(const std::source_location& location = std::source_location::current()) {
    Event msg{};
    msg.set_timestamp(ClockType::now().time_since_epoch().count());
    Function* fn = msg.mutable_fn();
    fn->set_name(location.function_name());
    fn->set_source_file(location.file_name());
    fn->set_line_number(location.line_number());
    writer_.write_message(msg);
  }

#else

  void log_fn(const char* filename, uint32_t line_number, const char* function_name) {
    Event msg{};
    msg.set_timestamp(ClockType::now().time_since_epoch().count());
    Function* fn = msg.mutable_fn();
    fn->set_name(function_name);
    fn->set_source_file(filename);
    fn->set_line_number(line_number);
    writer_.write_message(msg);
  }

#endif
};

}  // namespace tvsc::hal::simulation
