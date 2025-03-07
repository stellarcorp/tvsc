#include <fcntl.h>
#include <unistd.h>

#include "google/protobuf/arena.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

namespace tvsc::proto {

template <typename ProtoType>
class ProtobufFileWriter {
 private:
  int fd_;
  google::protobuf::io::FileOutputStream* stream_;
  google::protobuf::Arena arena_{};  // Arena for efficient memory allocation

 public:
  explicit ProtobufFileWriter(const std::string& filename)
      : fd_(open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644)) {
    if (fd_ < 0) {
      throw std::runtime_error("Failed to open file for writing.");
    }
    stream_ = new google::protobuf::io::FileOutputStream(fd_);
  }

  ~ProtobufFileWriter() {
    if (stream_) {
      stream_->Flush();
      delete stream_;
    }
    if (fd_ >= 0) {
      close(fd_);
    }
  }

  void WriteMessage(const ProtoType& message) {
    if (!message.SerializeToZeroCopyStream(stream_)) {
      throw std::runtime_error("Failed to serialize protobuf message.");
    }
  }

  ProtoType* CreateMessage() { return Arena::CreateMessage<ProtoType>(&arena_); }
};

}  // namespace tvsc::proto
