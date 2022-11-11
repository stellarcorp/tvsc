#pragma once

#include <cstdio>
#include <filesystem>

#include "buffer/ring_buffer.h"
#include "io/file_reader.h"

namespace tvsc::io {

template <typename ElementT, size_t BUFFER_SIZE, size_t NUM_BUFFERS>
class FileDataSource final : public buffer::RingBuffer<ElementT, BUFFER_SIZE, NUM_BUFFERS>::DataSource {
 private:
  FileReader<ElementT> file_reader_;
  buffer::Buffer<ElementT, BUFFER_SIZE> buffer_{};
  bool data_needed_{false};

 public:
  FileDataSource(std::string_view filename) : file_reader_(filename) {}

  void signal_data_needed() override { data_needed_ = true; }
  bool data_needed() const { return data_needed_; }

  size_t try_write(size_t num_elements) {
    num_elements = std::min(num_elements, std::min(buffer_.size(), this->ring_buffer()->max_buffered_elements()));
    size_t total_elements_written{0};
    if (data_needed_) {
      data_needed_ = false;
      num_elements = file_reader_.read(num_elements, buffer_);
      if (num_elements == 0) {
        // Try to rewind and read once more, but only once more.
        file_reader_.rewind();
        num_elements = file_reader_.read(num_elements, buffer_);
      }

      total_elements_written = this->ring_buffer()->write(num_elements, buffer_.data());
    }

    return total_elements_written;
  }
};

}  // namespace tvsc::io
