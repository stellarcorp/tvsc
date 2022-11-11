#include "io/looping_file_reader.h"

#include <array>
#include <cstring>

#include "buffer/buffer.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

namespace tvsc::io {

TEST(LoopingFileReaderTest, CanReadFromFileViaCArray) {
  LoopingFileReader<char> reader{"io/lorem_ipsum.txt"};
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

TEST(LoopingFileReaderTest, CanReadFromFileViaStdArray) {
  LoopingFileReader<char> reader{"io/lorem_ipsum.txt"};
  constexpr size_t BUFFER_SIZE{128};
  std::array<char, BUFFER_SIZE> buffer{};
  buffer[0] = 0;
  size_t count = reader.read(BUFFER_SIZE, buffer);
  EXPECT_EQ(BUFFER_SIZE, count);
  EXPECT_NE(buffer[0], 0);
}

TEST(LoopingFileReaderTest, CanReadFromFileViaBuffer) {
  LoopingFileReader<char> reader{"io/lorem_ipsum.txt"};
  constexpr size_t BUFFER_SIZE{128};
  buffer::Buffer<char, BUFFER_SIZE> buffer{};
  buffer[0] = 0;
  size_t count = reader.read(buffer);
  EXPECT_EQ(BUFFER_SIZE, count);
  EXPECT_NE(buffer[0], 0);
}

TEST(LoopingFileReaderTest, LoopsAtEof) {
  LoopingFileReader<char> reader{"io/lorem_ipsum.txt"};
  constexpr size_t BUFFER_SIZE{128};
  char buffer[BUFFER_SIZE + 1];
  buffer[0] = 0;
  buffer[BUFFER_SIZE] = 0;
  size_t total_bytes_read{0};
  size_t bytes_read{0};
  do {
    bytes_read = reader.read(BUFFER_SIZE, buffer);
    total_bytes_read += bytes_read;
  } while (total_bytes_read < 32768);

  // We are at EOF.
  ASSERT_EQ(reader.file_size(), reader.tell());

  bytes_read = reader.read(BUFFER_SIZE, buffer);
  EXPECT_EQ(BUFFER_SIZE, bytes_read);
}

}  // namespace tvsc::io
