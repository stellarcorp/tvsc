#include "proto/proto_file_reader.h"

#include <fcntl.h>
#include <unistd.h>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message_lite.h"

namespace tvsc::proto {

ProtoFileReader::ProtoFileReader(const std::string& filename) {
  const int fd{open(filename.c_str(), O_RDONLY)};
  if (fd < 0) {
    throw std::runtime_error("Failed to open file for reading.");
  }
  stream_ = std::make_unique<google::protobuf::io::FileInputStream>(fd);
  stream_->SetCloseOnDelete(true);
}

bool ProtoFileReader::read_message(google::protobuf::MessageLite& message) {
  google::protobuf::io::CodedInputStream coded_stream(stream_.get());

  uint32_t size;
  if (!coded_stream.ReadVarint32(&size)) {
    return false;
  }

  const void* data{nullptr};
  int32_t available_bytes{};
  if (coded_stream.GetDirectBufferPointer(&data, &available_bytes) &&
      static_cast<uint32_t>(available_bytes) >= size) {
    if (!message.ParseFromArray(data, size)) {
      return false;
    }
    coded_stream.Skip(size);
    return true;
  }

  // Fallback: If a direct buffer isn't available, use the standard ReadRaw method.
  std::vector<uint8_t> buffer(size);
  if (!coded_stream.ReadRaw(buffer.data(), size)) {
    return false;
  }

  return message.ParseFromArray(buffer.data(), size);
}

}  // namespace tvsc::proto
