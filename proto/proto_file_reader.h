#pragma once

#include <filesystem>
#include <memory>

#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message_lite.h"

namespace tvsc::proto {

class ProtoFileReader {
 private:
  std::unique_ptr<google::protobuf::io::FileInputStream> stream_{};

 public:
  explicit ProtoFileReader(const std::filesystem::path& filename);
  bool read_message(google::protobuf::MessageLite& message);
};

}  // namespace tvsc::proto
