#include "buffer/buffer.h"

#include <array>

#include "gtest/gtest.h"

namespace tvsc::buffer {

// Uncomment to verify compilation error due to static_assert violation.
// TEST(BufferTest, CantCreateBuffersWithZeroElements) { Buffer<int, 0> buffer{}; }

TEST(BufferTest, CanCreateBuffer) {
  Buffer<int, 128> buffer{};
  buffer[0] = 1;
  buffer[127] = 128;
  EXPECT_EQ(1, buffer[0]);
  EXPECT_EQ(128, buffer[127]);
}

TEST(BufferTest, BufferSizeMatchesTemplateParameter) {
  Buffer<int, 128> buffer{};
  EXPECT_EQ(128, buffer.size());
}

TEST(BufferTest, CanCreateTinyBuffer) {
  Buffer<int, 1> buffer{};
  buffer[0] = 1;
  EXPECT_EQ(1, buffer[0]);
}

TEST(BufferTest, CanCreateLargeBuffer) {
  constexpr size_t SIZE{10'000};
  Buffer<int, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    buffer[i] = i;
  }
  for (size_t i = 0; i < SIZE; ++i) {
    EXPECT_EQ(i, buffer[i]);
  }
}

TEST(BufferTest, CanBulkReadViaCArray) {
  constexpr size_t SIZE{256};
  int other[SIZE];
  Buffer<int, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    buffer[i] = i;
  }
  buffer.read(0, SIZE, other);
  for (size_t i = 0; i < SIZE; ++i) {
    EXPECT_EQ(i, other[i]);
  }
}

TEST(BufferTest, CanBulkReadViaCArrayWithOffset) {
  constexpr size_t SIZE{256};
  constexpr size_t OFFSET{SIZE / 2};
  int other[SIZE - OFFSET];
  Buffer<int, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    buffer[i] = i;
  }
  buffer.read(OFFSET, SIZE - OFFSET, other);
  for (size_t i = 0; i < SIZE - OFFSET; ++i) {
    EXPECT_EQ(i + OFFSET, other[i]);
  }
}

TEST(BufferTest, CanBulkReadViaStdArray) {
  constexpr size_t SIZE{256};
  std::array<int, SIZE> other{};
  Buffer<int, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    buffer[i] = i;
  }
  buffer.read(0, SIZE, other);
  for (size_t i = 0; i < SIZE; ++i) {
    EXPECT_EQ(i, other[i]);
  }
}

TEST(BufferTest, CanBulkReadViaStdArrayWithOffset) {
  constexpr size_t SIZE{256};
  constexpr size_t OFFSET{SIZE / 2};
  std::array<int, SIZE - OFFSET> other{};
  Buffer<int, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    buffer[i] = i;
  }
  buffer.read(OFFSET, SIZE - OFFSET, other);
  for (size_t i = 0; i < SIZE - OFFSET; ++i) {
    EXPECT_EQ(i + OFFSET, other[i]);
  }
}

TEST(BufferTest, CanBulkWriteViaCArray) {
  constexpr size_t SIZE{256};
  int other[SIZE];
  Buffer<int, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    other[i] = i;
  }
  buffer.write(0, SIZE, other);
  for (size_t i = 0; i < SIZE; ++i) {
    EXPECT_EQ(i, buffer[i]);
  }
}

TEST(BufferTest, CanBulkWriteViaCArrayWithOffset) {
  constexpr size_t SIZE{256};
  constexpr size_t OFFSET{SIZE / 2};
  int other[SIZE - OFFSET];
  Buffer<int, SIZE> buffer{};
  for (size_t i = 0; i < SIZE - OFFSET; ++i) {
    other[i] = i;
  }
  buffer.write(OFFSET, SIZE - OFFSET, other);
  for (size_t i = 0; i < SIZE; ++i) {
    if (i < OFFSET) {
      EXPECT_EQ(0, buffer[i]);
    } else {
      EXPECT_EQ(i - OFFSET, buffer[i]);
    }
  }
}

TEST(BufferTest, CanBulkWriteViaStdArray) {
  constexpr size_t SIZE{256};
  std::array<int, SIZE> other{};
  Buffer<int, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    other[i] = i;
  }
  buffer.write(0, SIZE, other);
  for (size_t i = 0; i < SIZE; ++i) {
    EXPECT_EQ(i, buffer[i]);
  }
}

TEST(BufferTest, CanBulkWriteViaStdArrayWithOffset) {
  constexpr size_t SIZE{256};
  constexpr size_t OFFSET{SIZE / 2};
  std::array<int, SIZE - OFFSET> other{};
  Buffer<int, SIZE> buffer{};
  for (size_t i = 0; i < SIZE - OFFSET; ++i) {
    other[i] = i;
  }
  buffer.write(OFFSET, SIZE - OFFSET, other);
  for (size_t i = 0; i < SIZE; ++i) {
    if (i < OFFSET) {
      EXPECT_EQ(0, buffer[i]);
    } else {
      EXPECT_EQ(i - OFFSET, buffer[i]);
    }
  }
}

class TriviallyCopyableType final {
 private:
  int value_{};

 public:
  TriviallyCopyableType& operator=(const TriviallyCopyableType& rhs) = default;

  TriviallyCopyableType& operator=(int value) {
    value_ = value;
    return *this;
  }

  int value() const { return value_; }

  bool operator==(const TriviallyCopyableType& rhs) const { return value_ == rhs.value_; }
};

bool operator==(int lhs, const TriviallyCopyableType& rhs) { return lhs == rhs.value(); }
bool operator==(const TriviallyCopyableType& lhs, int rhs) { return lhs.value() == rhs; }

TEST(TriviallyCopyableBufferTest, TriviallyCopyableIsTriviallyCopyable) {
  ASSERT_TRUE(std::is_trivially_copyable<TriviallyCopyableType>::value);
}

TEST(TriviallyCopyableBufferTest, CanCreateBuffer) {
  Buffer<TriviallyCopyableType, 128> buffer{};
  buffer[0] = 1;
  buffer[127] = 128;
  EXPECT_EQ(1, buffer[0]);
  EXPECT_EQ(128, buffer[127]);
}

TEST(TriviallyCopyableBufferTest, BufferSizeMatchesTemplateParameter) {
  Buffer<TriviallyCopyableType, 128> buffer{};
  EXPECT_EQ(128, buffer.size());
}

TEST(TriviallyCopyableBufferTest, CanCreateTinyBuffer) {
  Buffer<TriviallyCopyableType, 1> buffer{};
  buffer[0] = 1;
  EXPECT_EQ(1, buffer[0]);
}

TEST(TriviallyCopyableBufferTest, CanCreateLargeBuffer) {
  constexpr size_t SIZE{10'000};
  Buffer<TriviallyCopyableType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    buffer[i] = i;
  }
  for (size_t i = 0; i < SIZE; ++i) {
    EXPECT_EQ(i, buffer[i]);
  }
}

TEST(TriviallyCopyableBufferTest, CanBulkReadViaCArray) {
  constexpr size_t SIZE{256};
  TriviallyCopyableType other[SIZE];
  Buffer<TriviallyCopyableType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    buffer[i] = i;
  }
  buffer.read(0, SIZE, other);
  for (size_t i = 0; i < SIZE; ++i) {
    EXPECT_EQ(i, other[i]);
  }
}

TEST(TriviallyCopyableBufferTest, CanBulkReadViaCArrayWithOffset) {
  constexpr size_t SIZE{256};
  constexpr size_t OFFSET{SIZE / 2};
  TriviallyCopyableType other[SIZE - OFFSET];
  Buffer<TriviallyCopyableType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    buffer[i] = i;
  }
  buffer.read(OFFSET, SIZE - OFFSET, other);
  for (size_t i = 0; i < SIZE - OFFSET; ++i) {
    EXPECT_EQ(i + OFFSET, other[i]);
  }
}

TEST(TriviallyCopyableBufferTest, CanBulkReadViaStdArray) {
  constexpr size_t SIZE{256};
  std::array<TriviallyCopyableType, SIZE> other{};
  Buffer<TriviallyCopyableType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    buffer[i] = i;
  }
  buffer.read(0, SIZE, other);
  for (size_t i = 0; i < SIZE; ++i) {
    EXPECT_EQ(i, other[i]);
  }
}

TEST(TriviallyCopyableBufferTest, CanBulkReadViaStdArrayWithOffset) {
  constexpr size_t SIZE{256};
  constexpr size_t OFFSET{SIZE / 2};
  std::array<TriviallyCopyableType, SIZE - OFFSET> other{};
  Buffer<TriviallyCopyableType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    buffer[i] = i;
  }
  buffer.read(OFFSET, SIZE - OFFSET, other);
  for (size_t i = 0; i < SIZE - OFFSET; ++i) {
    EXPECT_EQ(i + OFFSET, other[i]);
  }
}

TEST(TriviallyCopyableBufferTest, CanBulkWriteViaCArray) {
  constexpr size_t SIZE{256};
  TriviallyCopyableType other[SIZE];
  Buffer<TriviallyCopyableType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    other[i] = i;
  }
  buffer.write(0, SIZE, other);
  for (size_t i = 0; i < SIZE; ++i) {
    EXPECT_EQ(i, buffer[i]);
  }
}

TEST(TriviallyCopyableBufferTest, CanBulkWriteViaCArrayWithOffset) {
  constexpr size_t SIZE{256};
  constexpr size_t OFFSET{SIZE / 2};
  TriviallyCopyableType other[SIZE - OFFSET];
  Buffer<TriviallyCopyableType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE - OFFSET; ++i) {
    other[i] = i;
  }
  buffer.write(OFFSET, SIZE - OFFSET, other);
  for (size_t i = 0; i < SIZE; ++i) {
    if (i < OFFSET) {
      EXPECT_EQ(0, buffer[i]);
    } else {
      EXPECT_EQ(i - OFFSET, buffer[i]);
    }
  }
}

TEST(TriviallyCopyableBufferTest, CanBulkWriteViaStdArray) {
  constexpr size_t SIZE{256};
  std::array<TriviallyCopyableType, SIZE> other{};
  Buffer<TriviallyCopyableType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    other[i] = i;
  }
  buffer.write(0, SIZE, other);
  for (size_t i = 0; i < SIZE; ++i) {
    EXPECT_EQ(i, buffer[i]);
  }
}

TEST(TriviallyCopyableBufferTest, CanBulkWriteViaStdArrayWithOffset) {
  constexpr size_t SIZE{256};
  constexpr size_t OFFSET{SIZE / 2};
  std::array<TriviallyCopyableType, SIZE - OFFSET> other{};
  Buffer<TriviallyCopyableType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE - OFFSET; ++i) {
    other[i] = i;
  }
  buffer.write(OFFSET, SIZE - OFFSET, other);
  for (size_t i = 0; i < SIZE; ++i) {
    if (i < OFFSET) {
      EXPECT_EQ(0, buffer[i]);
    } else {
      EXPECT_EQ(i - OFFSET, buffer[i]);
    }
  }
}

class NontrivialType final {
 private:
  int value_{};

 public:
  NontrivialType() = default;
  NontrivialType(int value) : value_(value) {}

  // Adding this copy constructor makes this type non-trivially copyable.
  NontrivialType(const NontrivialType& rhs) : value_(rhs.value_) {}

  NontrivialType& operator=(const NontrivialType& rhs) = default;

  NontrivialType& operator=(int value) {
    value_ = value;
    return *this;
  }

  int value() const { return value_; }

  bool operator==(const NontrivialType& rhs) const { return value_ == rhs.value_; }
};

bool operator==(int lhs, const NontrivialType& rhs) { return lhs == rhs.value(); }
bool operator==(const NontrivialType& lhs, int rhs) { return lhs.value() == rhs; }

TEST(NontrivialTypeBufferTest, TriviallyCopyableIsTriviallyCopyable) {
  ASSERT_FALSE(std::is_trivially_copyable<NontrivialType>::value);
}

TEST(NontrivialTypeBufferTest, CanCreateBuffer) {
  Buffer<NontrivialType, 128> buffer{};
  buffer[0] = 1;
  buffer[127] = 128;
  EXPECT_EQ(1, buffer[0]);
  EXPECT_EQ(128, buffer[127]);
}

TEST(NontrivialTypeBufferTest, BufferSizeMatchesTemplateParameter) {
  Buffer<NontrivialType, 128> buffer{};
  EXPECT_EQ(128, buffer.size());
}

TEST(NontrivialTypeBufferTest, CanCreateTinyBuffer) {
  Buffer<NontrivialType, 1> buffer{};
  buffer[0] = 1;
  EXPECT_EQ(1, buffer[0]);
}

TEST(NontrivialTypeBufferTest, CanCreateLargeBuffer) {
  constexpr size_t SIZE{10'000};
  Buffer<NontrivialType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    buffer[i] = i;
  }
  for (size_t i = 0; i < SIZE; ++i) {
    EXPECT_EQ(i, buffer[i]);
  }
}

TEST(NontrivialTypeBufferTest, CanBulkReadViaCArray) {
  constexpr size_t SIZE{256};
  NontrivialType other[SIZE];
  Buffer<NontrivialType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    buffer[i] = i;
  }
  buffer.read(0, SIZE, other);
  for (size_t i = 0; i < SIZE; ++i) {
    EXPECT_EQ(i, other[i]);
  }
}

TEST(NontrivialTypeBufferTest, CanBulkReadViaCArrayWithOffset) {
  constexpr size_t SIZE{256};
  constexpr size_t OFFSET{SIZE / 2};
  NontrivialType other[SIZE - OFFSET];
  Buffer<NontrivialType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    buffer[i] = i;
  }
  buffer.read(OFFSET, SIZE - OFFSET, other);
  for (size_t i = 0; i < SIZE - OFFSET; ++i) {
    EXPECT_EQ(i + OFFSET, other[i]);
  }
}

TEST(NontrivialTypeBufferTest, CanBulkReadViaStdArray) {
  constexpr size_t SIZE{256};
  std::array<NontrivialType, SIZE> other{};
  Buffer<NontrivialType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    buffer[i] = i;
  }
  buffer.read(0, SIZE, other);
  for (size_t i = 0; i < SIZE; ++i) {
    EXPECT_EQ(i, other[i]);
  }
}

TEST(NontrivialTypeBufferTest, CanBulkReadViaStdArrayWithOffset) {
  constexpr size_t SIZE{256};
  constexpr size_t OFFSET{SIZE / 2};
  std::array<NontrivialType, SIZE - OFFSET> other{};
  Buffer<NontrivialType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    buffer[i] = i;
  }
  buffer.read(OFFSET, SIZE - OFFSET, other);
  for (size_t i = 0; i < SIZE - OFFSET; ++i) {
    EXPECT_EQ(i + OFFSET, other[i]);
  }
}

TEST(NontrivialTypeBufferTest, CanBulkWriteViaCArray) {
  constexpr size_t SIZE{256};
  NontrivialType other[SIZE];
  Buffer<NontrivialType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    other[i] = i;
  }
  buffer.write(0, SIZE, other);
  for (size_t i = 0; i < SIZE; ++i) {
    EXPECT_EQ(i, buffer[i]);
  }
}

TEST(NontrivialTypeBufferTest, CanBulkWriteViaCArrayWithOffset) {
  constexpr size_t SIZE{256};
  constexpr size_t OFFSET{SIZE / 2};
  NontrivialType other[SIZE - OFFSET];
  Buffer<NontrivialType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE - OFFSET; ++i) {
    other[i] = i;
  }
  buffer.write(OFFSET, SIZE - OFFSET, other);
  for (size_t i = 0; i < SIZE; ++i) {
    if (i < OFFSET) {
      EXPECT_EQ(0, buffer[i]);
    } else {
      EXPECT_EQ(i - OFFSET, buffer[i]);
    }
  }
}

TEST(NontrivialTypeBufferTest, CanBulkWriteViaStdArray) {
  constexpr size_t SIZE{256};
  std::array<NontrivialType, SIZE> other{};
  Buffer<NontrivialType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE; ++i) {
    other[i] = i;
  }
  buffer.write(0, SIZE, other);
  for (size_t i = 0; i < SIZE; ++i) {
    EXPECT_EQ(i, buffer[i]);
  }
}

TEST(NontrivialTypeBufferTest, CanBulkWriteViaStdArrayWithOffset) {
  constexpr size_t SIZE{256};
  constexpr size_t OFFSET{SIZE / 2};
  std::array<NontrivialType, SIZE - OFFSET> other{};
  Buffer<NontrivialType, SIZE> buffer{};
  for (size_t i = 0; i < SIZE - OFFSET; ++i) {
    other[i] = i;
  }
  buffer.write(OFFSET, SIZE - OFFSET, other);
  for (size_t i = 0; i < SIZE; ++i) {
    if (i < OFFSET) {
      EXPECT_EQ(0, buffer[i]);
    } else {
      EXPECT_EQ(i - OFFSET, buffer[i]);
    }
  }
}

}  // namespace tvsc::buffer
