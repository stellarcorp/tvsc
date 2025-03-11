#pragma once

#include <filesystem>
#include <memory>
#include <mutex>

#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message_lite.h"

namespace tvsc::proto {

class ProtoFileWriter {
 private:
  std::unique_ptr<google::protobuf::io::FileOutputStream> stream_;
  std::mutex m_{};

 public:
  explicit ProtoFileWriter(const std::filesystem::path& filename);
  bool write_message(const google::protobuf::MessageLite& message);
};

}  // namespace tvsc::proto
