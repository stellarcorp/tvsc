#include "proto/proto_file_writer.h"

#include <fcntl.h>
#include <unistd.h>

#include <filesystem>
#include <memory>
#include <mutex>

#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message_lite.h"

namespace tvsc::proto {

ProtoFileWriter::ProtoFileWriter(const std::filesystem::path& filename) {
  const int fd{open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644)};
  if (fd < 0) {
    throw std::runtime_error("Failed to open file for writing.");
  }
  stream_ = std::make_unique<google::protobuf::io::FileOutputStream>(fd);
  stream_->SetCloseOnDelete(true);
}

bool ProtoFileWriter::write_message(const google::protobuf::MessageLite& message) {
  std::lock_guard lock{m_};
  google::protobuf::io::CodedOutputStream coded_stream(stream_.get());

  size_t size = message.ByteSizeLong();
  coded_stream.WriteVarint32(size);

  return message.SerializeToCodedStream(&coded_stream);
}

}  // namespace tvsc::proto
