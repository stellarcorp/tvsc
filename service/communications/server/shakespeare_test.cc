#include "service/communications/server/shakespeare.h"

#include <cstring>

#include "gtest/gtest.h"

namespace tvsc::service::communications {

constexpr const char FIRST_LINE[] = "A Midsummer Night's Dream\n";

TEST(ShakespeareTest, CanReadSingleLine) {
  Shakespeare text{};
  char buffer[1024]{};
  const size_t length = text.get_next_line(buffer, sizeof(buffer));
  ASSERT_EQ(std::strlen(FIRST_LINE), length);
  EXPECT_STREQ(FIRST_LINE, buffer);
  EXPECT_EQ('\n', buffer[length - 1]);
}

TEST(ShakespeareTest, CanReadMultipleLines) {
  Shakespeare text{};
  char buffer[1024]{};
  for (size_t i = 0; i < 5; ++i) {
    const size_t length = text.get_next_line(buffer, sizeof(buffer));
    EXPECT_GT(length, 0);
    EXPECT_EQ('\n', buffer[length - 1]);
  }
}

TEST(ShakespeareTest, CanReadManyLines) {
  Shakespeare text{};
  char buffer[1024]{};
  for (size_t i = 0; i < 100; ++i) {
    const size_t length = text.get_next_line(buffer, sizeof(buffer));
    EXPECT_GT(length, 0);
    EXPECT_EQ('\n', buffer[length - 1]);
  }
}

TEST(ShakespeareTest, CanReadAllTheLines) {
  Shakespeare text{};
  for (size_t i = 0; i < text.line_count(); ++i) {
    char buffer[1024]{};
    const size_t length = text.get_next_line(buffer, sizeof(buffer));
    EXPECT_GT(length, 0);
    EXPECT_EQ('\n', buffer[length - 1]);
  }
}

TEST(ShakespeareTest, ResetsPositionAfterReadingAllTheLines) {
  Shakespeare text{};
  for (size_t i = 0; i < text.line_count(); ++i) {
    char buffer[1024]{};
    const size_t length = text.get_next_line(buffer, sizeof(buffer));
    EXPECT_GT(length, 0);
    EXPECT_EQ('\n', buffer[length - 1]);
  }
  // Re-read the first line.
  char buffer[1024]{};
  const size_t length = text.get_next_line(buffer, sizeof(buffer));
  ASSERT_EQ(std::strlen(FIRST_LINE), length);
  EXPECT_STREQ(FIRST_LINE, buffer);
  EXPECT_EQ('\n', buffer[length - 1]);
}

TEST(ShakespeareTest, CanReadAllTheLinesTwice) {
  Shakespeare text{};
  for (size_t i = 0; i < 2 * text.line_count(); ++i) {
    char buffer[1024]{};
    const size_t length = text.get_next_line(buffer, sizeof(buffer));
    EXPECT_GT(length, 0);
    EXPECT_EQ('\n', buffer[length - 1]);
  }
}

TEST(ShakespeareTest, ResetsPositionAfterReadingAllTheLinesTwice) {
  Shakespeare text{};
  for (size_t i = 0; i < 2 * text.line_count(); ++i) {
    char buffer[1024]{};
    const size_t length = text.get_next_line(buffer, sizeof(buffer));
    EXPECT_GT(length, 0);
    EXPECT_EQ('\n', buffer[length - 1]);
  }
  // Re-read the first line.
  char buffer[1024]{};
  const size_t length = text.get_next_line(buffer, sizeof(buffer));
  ASSERT_EQ(std::strlen(FIRST_LINE), length);
  EXPECT_STREQ(FIRST_LINE, buffer);
  EXPECT_EQ('\n', buffer[length - 1]);
}

TEST(ShakespeareTest, LinesEndInNewlineWhenBufferBigEnough) {
  Shakespeare text{};
  char buffer[1024]{};
  const size_t length = text.get_next_line(buffer, sizeof(buffer));
  EXPECT_EQ('\n', buffer[length - 1]);
}

TEST(ShakespeareTest, CanReadIntoSmallerBuffers) {
  constexpr size_t SIZE_REDUCTION{10};
  Shakespeare text{};
  char buffer[sizeof(FIRST_LINE) - SIZE_REDUCTION]{};
  const size_t length = text.get_next_line(buffer, sizeof(buffer));

  // sizeof includes storage for the newline. Since buffer is initialized with the
  // sizeof(FIRST_LINE), it actually has space for one more character than if it was allocated with
  // the strlen(FIRST_LINE).
  ASSERT_EQ(std::strlen(FIRST_LINE) + 1, sizeof(FIRST_LINE));
  EXPECT_EQ(std::strlen(FIRST_LINE) - SIZE_REDUCTION + 1, length);
}

TEST(ShakespeareTest, CanReadAllTheLinesIntoTooSmallBuffer) {
  Shakespeare text{};
  for (size_t i = 0; i < text.line_count(); ++i) {
    char buffer[20]{};
    const size_t length = text.get_next_line(buffer, sizeof(buffer));
    EXPECT_GT(length, 0);
  }
}

TEST(ShakespeareTest, ResetsPositionAfterReadingAllTheLinesIntoTooSmallBuffer) {
  Shakespeare text{};
  for (size_t i = 0; i < text.line_count(); ++i) {
    char buffer[20]{};
    const size_t length = text.get_next_line(buffer, sizeof(buffer));
    EXPECT_GT(length, 0);
  }
  // Re-read the first line.
  char buffer[20]{};
  const size_t length = text.get_next_line(buffer, sizeof(buffer));
  EXPECT_EQ(0, std::strncmp(FIRST_LINE, buffer, length));
}

TEST(ShakespeareTest, CanReadAllTheLinesTwiceIntoTooSmallBuffer) {
  Shakespeare text{};
  for (size_t i = 0; i < 2 * text.line_count(); ++i) {
    char buffer[20]{};
    const size_t length = text.get_next_line(buffer, sizeof(buffer));
    EXPECT_GT(length, 0);
  }
}

TEST(ShakespeareTest, ResetsPositionAfterReadingAllTheLinesTwiceIntoTooSmallBuffer) {
  Shakespeare text{};
  for (size_t i = 0; i < 2 * text.line_count(); ++i) {
    char buffer[20]{};
    const size_t length = text.get_next_line(buffer, sizeof(buffer));
    EXPECT_GT(length, 0);
  }
  // Re-read the first line.
  char buffer[20]{};
  const size_t length = text.get_next_line(buffer, sizeof(buffer));
  EXPECT_GT(length, 0);
  EXPECT_EQ(0, std::strncmp(FIRST_LINE, buffer, length));
}

}  // namespace tvsc::service::communications
