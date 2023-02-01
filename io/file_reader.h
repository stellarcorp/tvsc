#pragma once

#include <array>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string_view>

#include "buffer/buffer.h"

namespace tvsc::io {

template <typename ElementT>
class FileReader final {
 private:
  std::filesystem::path filename_;
  std::FILE* file_;

 public:
  FileReader(std::filesystem::path filename)
      : filename_(filename), file_(std::fopen(filename_.c_str(), "rb")) {}

  ~FileReader() {
    if (file_) {
      std::fclose(file_);
    }
  }

  size_t file_size() const { return std::filesystem::file_size(filename_) / sizeof(ElementT); }
  size_t tell() const { return std::ftell(file_) / sizeof(ElementT); }
  void seek(size_t position) { std::fseek(file_, position * sizeof(ElementT), SEEK_SET); }
  void rewind() { std::rewind(file_); }

  bool eof() const { return tell() == file_size(); }

  bool operator()() const { return file_ != nullptr and !eof(); }

  std::string error_message() const {
    return std::strerror(errno);
  }

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
    return std::fread(buffer, sizeof(ElementT), count, file_);
  }
};

}  // namespace tvsc::io
