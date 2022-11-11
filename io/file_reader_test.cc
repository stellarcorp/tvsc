#include "io/file_reader.h"

#include <array>
#include <cstring>

#include "buffer/buffer.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

namespace tvsc::io {

TEST(FileReaderTest, CanReadFromFileViaCArray) {
  FileReader<char> reader{"io/lorem_ipsum.txt"};
  constexpr size_t BUFFER_SIZE{128};
  char buffer[BUFFER_SIZE + 1];
  buffer[0] = 0;
  buffer[BUFFER_SIZE] = 0;
  size_t count = reader.read(BUFFER_SIZE, buffer);
  LOG(INFO) << "buffer: " << buffer;
  EXPECT_EQ(BUFFER_SIZE, count);
  EXPECT_NE(buffer[0], 0);
  EXPECT_EQ(BUFFER_SIZE, std::strlen(buffer));
  EXPECT_STREQ(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore "
      "magna aliqua. Auct",
      buffer);
}

TEST(FileReaderTest, CanReadFromFileViaStdArray) {
  FileReader<char> reader{"io/lorem_ipsum.txt"};
  constexpr size_t BUFFER_SIZE{128};
  std::array<char, BUFFER_SIZE> buffer{};
  buffer[0] = 0;
  size_t count = reader.read(BUFFER_SIZE, buffer);
  EXPECT_EQ(BUFFER_SIZE, count);
  EXPECT_NE(buffer[0], 0);
}

TEST(FileReaderTest, CanReadFromFileViaBuffer) {
  FileReader<char> reader{"io/lorem_ipsum.txt"};
  constexpr size_t BUFFER_SIZE{128};
  buffer::Buffer<char, BUFFER_SIZE> buffer{};
  buffer[0] = 0;
  size_t count = reader.read(buffer);
  EXPECT_EQ(BUFFER_SIZE, count);
  EXPECT_NE(buffer[0], 0);
}

TEST(FileReaderTest, CanReadFullFile) {
  FileReader<char> reader{"io/lorem_ipsum.txt"};
  constexpr size_t BUFFER_SIZE{128};
  char buffer[BUFFER_SIZE + 1];
  buffer[0] = 0;
  buffer[BUFFER_SIZE] = 0;
  size_t total_bytes_read{0};
  size_t bytes_read{0};
  do {
    bytes_read = reader.read(BUFFER_SIZE, buffer);
    total_bytes_read += bytes_read;
  } while (bytes_read == BUFFER_SIZE);

  EXPECT_EQ(32768, total_bytes_read);
}

TEST(FileReaderTest, CanReadFullFileWithRelativePrimeBufferSize) {
  FileReader<char> reader{"io/lorem_ipsum.txt"};
  constexpr size_t BUFFER_SIZE{129};
  char buffer[BUFFER_SIZE + 1];
  buffer[0] = 0;
  buffer[BUFFER_SIZE] = 0;
  size_t total_bytes_read{0};
  size_t bytes_read{0};
  do {
    bytes_read = reader.read(BUFFER_SIZE, buffer);
    total_bytes_read += bytes_read;
  } while (bytes_read == BUFFER_SIZE);

  EXPECT_EQ(32768, total_bytes_read);
}

}  // namespace tvsc::io
