#pragma once

#ifdef __has_include
#if __has_include(<source_location>)
#include <source_location>
#endif
#endif

#include <cstdint>
#include <filesystem>
#include <string>

#include "hal/simulation/simulation.pb.h"
#include "io/session_directory.h"
#include "proto/proto_file_writer.h"

namespace tvsc::hal::simulation {

template <typename ClockType>
class Logger final {
 private:
  const std::filesystem::path filename_;
  tvsc::proto::ProtoFileWriter writer_{filename_};

 public:
  Logger(io::SessionDirectory& log_directory, const std::string& filename)
      : filename_(log_directory.contextualize_filename(filename)) {}

  Logger(io::SessionDirectory& log_directory)
      : filename_(log_directory.create_temp_filename("sim_", ".log.pb")) {}

  const std::filesystem::path& log_file_name() const noexcept { return filename_; }

#if __cpp_lib_source_location >= 201907L

  void log_fn(const std::source_location& location = std::source_location::current()) {
    Event msg{};
    const int64_t current_time_us{
        std::chrono::duration_cast<std::chrono::microseconds>(ClockType::now().time_since_epoch())
            .count()};
    msg.set_timestamp_sec(current_time_us / 1'000'000.);
    Function* fn = msg.mutable_fn();
    fn->set_name(location.function_name());
    fn->set_source_file(location.file_name());
    fn->set_line_number(location.line());
    writer_.write_message(msg);
  }

#else

  void log_fn(const char* filename, uint32_t line_number, const char* function_name) {
    Event msg{};
    const int64_t current_time_us{
        std::chrono::duration_cast<std::chrono::microseconds>(ClockType::now().time_since_epoch())
            .count()};
    msg.set_timestamp_sec(current_time_us / 1'000'000.);
    Function* fn = msg.mutable_fn();
    fn->set_name(function_name);
    fn->set_source_file(filename);
    fn->set_line_number(line_number);
    writer_.write_message(msg);
  }

#endif

  void log_irq(int irq, const char* name) {
    Event msg{};
    const int64_t current_time_us{
        std::chrono::duration_cast<std::chrono::microseconds>(ClockType::now().time_since_epoch())
            .count()};
    msg.set_timestamp_sec(current_time_us / 1'000'000.);
    Irq* irq_proto = msg.mutable_irq();
    irq_proto->set_irq_number(irq);
    irq_proto->set_irq_name(name);
    writer_.write_message(msg);
  }
};

}  // namespace tvsc::hal::simulation
