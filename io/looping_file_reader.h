#pragma once

#include <array>
#include <cstdio>
#include <filesystem>
#include <string_view>

#include "buffer/buffer.h"
#include "io/file_reader.h"

namespace tvsc::io {

/**
 * Techniques for looping over the contents of a file to make the file appear to be infinitely long.
 * Useful for simulating streams, such as captured waveforms.
 */
template <typename ElementT>
class LoopingFileReader final {
 private:
  FileReader<ElementT> file_reader_;

 public:
  LoopingFileReader(std::filesystem::path filename) : file_reader_(filename) {}

  size_t file_size() const { return file_reader_.file_size(); }

  size_t tell() const { return file_reader_.tell(); }
  void seek(size_t position) { file_reader_.seek(position); }
  void rewind() { file_reader_.rewind(); }

  template <size_t NUM_ELEMENTS>
  size_t read(buffer::Buffer<ElementT, NUM_ELEMENTS>& buffer) {
    return read(buffer.size(), buffer.data());
  }

  template <size_t NUM_ELEMENTS>
  size_t read(size_t count, buffer::Buffer<ElementT, NUM_ELEMENTS>& buffer) {
    count = std::min(count, buffer.size());
    return read(count, buffer.data());
  }

  template <size_t NUM_ELEMENTS>
  size_t read(size_t count, std::array<ElementT, NUM_ELEMENTS>& buffer) {
    count = std::min(count, buffer.size());
    return read(count, buffer.data());
  }

  size_t read(size_t count, ElementT* buffer) {
    size_t elements_read = file_reader_.read(count, buffer);
    if (elements_read == 0 && file_reader_.eof()) {
      rewind();
      elements_read = file_reader_.read(count, buffer);
    }
    return elements_read;
  }
};

}  // namespace tvsc::io
