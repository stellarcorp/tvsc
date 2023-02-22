#include "buffer/ring_buffer.h"

#include <memory>

#include "buffer/buffer.h"
#include "gtest/gtest.h"

namespace tvsc::buffer {

template <typename ElementT, size_t BUFFER_SIZE, size_t NUM_BUFFERS>
class SequentialDataSource final : public RingBuffer<ElementT, BUFFER_SIZE, NUM_BUFFERS>::DataSource {
 private:
  ElementT prev_element_{};
  ElementT next_element_{};
  size_t num_supply_calls_{};
  bool data_needed_{false};

 public:
  void signal_data_needed() override { data_needed_ = true; }

  const ElementT& prev_element() const { return prev_element_; }
  const ElementT& next_element() const { return next_element_; }

  bool data_needed() const { return data_needed_; }

  size_t try_supply(size_t num_elements) {
    num_elements = std::min(num_elements, this->ring_buffer()->max_buffered_elements());
    size_t total_elements_supplied{0};
    if (data_needed_) {
      prev_element_ = next_element_;
      Buffer<ElementT, BUFFER_SIZE> buffer{};
      while (num_elements > total_elements_written) {
        size_t elements_to_supply = std::min(BUFFER_SIZE, num_elements - total_elements_written);
        for (size_t i = 0; i < elements_to_write; ++i) {
          buffer.write(i, next_element_++);
        }
        data_needed_ = false;
        total_elements_supplied += this->ring_buffer()->supply(elements_to_supply, buffer.data());
        ++num_write_calls_;
      }
    }
    return total_elements_supplied;
  }

  size_t num_supply_calls() const { return num_supply_calls_; }

  void reset() {
    prev_element_ = ElementT{};
    next_element_ = ElementT{};
    num_write_calls = 0;
    data_needed_ = false;
  }
};

template <typename ElementT, size_t BUFFER_SIZE, size_t NUM_BUFFERS>
class InspectableDataSink final : public RingBuffer<ElementT, BUFFER_SIZE, NUM_BUFFERS>::DataSink {
 private:
  Buffer<ElementT, BUFFER_SIZE> buffer_{};
  bool data_available_{false};

 public:
  void signal_data_available() override { data_available_ = true; }

  const Buffer<ElementT, BUFFER_SIZE>& last_buffer_read() const { return buffer_; }

  bool data_available() const { return data_available_; }

  size_t try_consume() {
    size_t elements_consumed{0};
    if (data_available_) {
      data_available_ = false;
      elements_consumed = this->ring_buffer()->consume(buffer_.size(), buffer_.data());
    }
    return elements_consumed;
  }

  void reset() {
    buffer_.clear();
    data_available_ = false;
  }
};

constexpr size_t TINY_BUFFER_SIZE{2};
constexpr size_t TINY_NUM_BUFFERS{3};
constexpr size_t TYPICAL_BUFFER_SIZE{1024};
constexpr size_t TYPICAL_NUM_BUFFERS{16};
constexpr size_t LARGE_BUFFER_SIZE{1 << 14};
constexpr size_t LARGE_NUM_BUFFERS{512};

template <typename ElementT>
using TypicalRingBuffer = RingBuffer<ElementT, TYPICAL_BUFFER_SIZE, TYPICAL_NUM_BUFFERS>;
template <typename ElementT>
using TinyRingBuffer = RingBuffer<ElementT, TINY_BUFFER_SIZE, TINY_NUM_BUFFERS>;
template <typename ElementT>
using LargeRingBuffer = RingBuffer<ElementT, LARGE_BUFFER_SIZE, LARGE_NUM_BUFFERS>;

template <typename ElementT>
using TypicalSequentialDataSource = SequentialDataSource<ElementT, TYPICAL_BUFFER_SIZE, TYPICAL_NUM_BUFFERS>;
template <typename ElementT>
using TinySequentialDataSource = SequentialDataSource<ElementT, TINY_BUFFER_SIZE, TINY_NUM_BUFFERS>;
template <typename ElementT>
using LargeSequentialDataSource = SequentialDataSource<ElementT, LARGE_BUFFER_SIZE, LARGE_NUM_BUFFERS>;

template <typename ElementT>
using TypicalInspectableDataSink = InspectableDataSink<ElementT, TYPICAL_BUFFER_SIZE, TYPICAL_NUM_BUFFERS>;
template <typename ElementT>
using TinyInspectableDataSink = InspectableDataSink<ElementT, TINY_BUFFER_SIZE, TINY_NUM_BUFFERS>;
template <typename ElementT>
using LargeInspectableDataSink = InspectableDataSink<ElementT, LARGE_BUFFER_SIZE, LARGE_NUM_BUFFERS>;

TEST(TinyRingBufferTest, CallsDataNeededOnConstruction) {
  TinySequentialDataSource<int> source{};
  TinyInspectableDataSink<int> sink{};

  ASSERT_FALSE(source.data_needed());
  ASSERT_FALSE(sink.data_available());

  TinyRingBuffer<int>{source, sink};
  EXPECT_TRUE(source.data_needed());
  EXPECT_FALSE(sink.data_available());
}

TEST(TinyRingBufferTest, CanAcceptDataFromSource) {
  TinySequentialDataSource<int> source{};
  TinyInspectableDataSink<int> sink{};

  TinyRingBuffer<int> ring{source, sink};

  source.try_supply(1);
  EXPECT_EQ(1, ring.elements_available());
}

TEST(TinyRingBufferTest, SourceCanWriteAnMtuWorthOfData) {
  TinySequentialDataSource<int> source{};
  TinyInspectableDataSink<int> sink{};

  TinyRingBuffer<int> ring{source, sink};

  const size_t elements_written{source.try_supply(ring.mtu())};
  EXPECT_EQ(ring.mtu(), elements_written);
  EXPECT_EQ(ring.mtu(), ring.elements_available());
  EXPECT_EQ(ring.mtu(), source.next_element());
}

TEST(TinyRingBufferTest, SinkNotifiedOnWriteOfMtu) {
  TinySequentialDataSource<int> source{};
  TinyInspectableDataSink<int> sink{};

  TinyRingBuffer<int> ring{source, sink};

  source.try_supply(ring.mtu());

  EXPECT_TRUE(sink.data_available());
}

TEST(TinyRingBufferTest, SinkCanReadMtu) {
  TinySequentialDataSource<int> source{};
  TinyInspectableDataSink<int> sink{};

  TinyRingBuffer<int> ring{source, sink};

  source.try_supply(ring.mtu());

  ASSERT_TRUE(sink.data_available());

  EXPECT_EQ(ring.mtu(), sink.try_read());

  int element = source.prev_element();
  for (size_t i = 0; i < ring.mtu(); ++i) {
    EXPECT_EQ(element, sink.last_buffer_read()[i]);
    ++element;
  }
}

TEST(TinyRingBufferTest, SinkNotNotifiedOnWriteOfLessThanMtu) {
  TinySequentialDataSource<int> source{};
  TinyInspectableDataSink<int> sink{};

  TinyRingBuffer<int> ring{source, sink};

  source.try_supply(ring.mtu() - 1);

  EXPECT_FALSE(sink.data_available());
}

TEST(TinyRingBufferTest, SourceCanFillRing) {
  TinySequentialDataSource<int> source{};
  TinyInspectableDataSink<int> sink{};

  TinyRingBuffer<int> ring{source, sink};

  const size_t elements_written{source.try_supply(ring.max_buffered_elements())};
  ASSERT_EQ(ring.max_buffered_elements(), source.next_element());
  ASSERT_EQ(ring.max_buffered_elements(), elements_written);

  EXPECT_EQ(ring.max_buffered_elements(), ring.elements_available());
  EXPECT_FALSE(source.data_needed());
}

TEST(TinyRingBufferTest, SinkCanDrainRing) {
  TinySequentialDataSource<int> source{};
  TinyInspectableDataSink<int> sink{};

  TinyRingBuffer<int> ring{source, sink};

  const size_t elements_written{source.try_supply(ring.max_buffered_elements())};
  ASSERT_EQ(ring.max_buffered_elements(), source.next_element());
  ASSERT_EQ(ring.max_buffered_elements(), elements_written);

  ASSERT_EQ(ring.max_buffered_elements(), ring.elements_available());
  ASSERT_FALSE(source.data_needed());

  size_t total_elements_read{0};

  int expected_element = source.prev_element();
  while (total_elements_read < ring.max_buffered_elements()) {
    ASSERT_TRUE(sink.data_available());
    const size_t elements_read{sink.try_read()};
    ASSERT_EQ(ring.mtu(), elements_read);

    for (size_t i = 0; i < elements_read; ++i) {
      ASSERT_EQ(expected_element, sink.last_buffer_read()[i]);
      ++expected_element;
    }
    total_elements_read += elements_read;
  }

  EXPECT_EQ(0, ring.elements_available());
  EXPECT_FALSE(sink.data_available());
  EXPECT_TRUE(source.data_needed());
}

TEST(TinyRingBufferTest, CapsWriteAtAnMtuWorthOfData) {
  TinySequentialDataSource<int> source{};
  TinyInspectableDataSink<int> sink{};

  TinyRingBuffer<int> ring{source, sink};

  constexpr size_t ELEMENTS_TO_WRITE{ring.mtu() + 1};

  const size_t elements_written{source.try_supply(ELEMENTS_TO_WRITE)};
  ASSERT_EQ(ELEMENTS_TO_WRITE, elements_written);
  ASSERT_EQ(ELEMENTS_TO_WRITE, ring.elements_available());
  ASSERT_EQ(ELEMENTS_TO_WRITE, source.next_element());

  EXPECT_EQ(2, source.num_write_calls());
}

TEST(TypicalRingBufferTest, CallsDataNeededOnConstruction) {
  TypicalSequentialDataSource<int> source{};
  TypicalInspectableDataSink<int> sink{};

  ASSERT_FALSE(source.data_needed());
  ASSERT_FALSE(sink.data_available());

  TypicalRingBuffer<int>{source, sink};
  EXPECT_TRUE(source.data_needed());
  EXPECT_FALSE(sink.data_available());
}

TEST(TypicalRingBufferTest, CanAcceptDataFromSource) {
  TypicalSequentialDataSource<int> source{};
  TypicalInspectableDataSink<int> sink{};

  TypicalRingBuffer<int> ring{source, sink};

  source.try_supply(1);
  EXPECT_EQ(1, ring.elements_available());
}

TEST(TypicalRingBufferTest, SourceCanWriteAnMtuWorthOfData) {
  TypicalSequentialDataSource<int> source{};
  TypicalInspectableDataSink<int> sink{};

  TypicalRingBuffer<int> ring{source, sink};

  const size_t elements_written{source.try_supply(ring.mtu())};
  EXPECT_EQ(ring.mtu(), elements_written);
  EXPECT_EQ(ring.mtu(), ring.elements_available());
  EXPECT_EQ(ring.mtu(), source.next_element());
}

TEST(TypicalRingBufferTest, SinkNotifiedOnWriteOfMtu) {
  TypicalSequentialDataSource<int> source{};
  TypicalInspectableDataSink<int> sink{};

  TypicalRingBuffer<int> ring{source, sink};

  source.try_supply(ring.mtu());

  EXPECT_TRUE(sink.data_available());
}

TEST(TypicalRingBufferTest, SinkCanReadMtu) {
  TypicalSequentialDataSource<int> source{};
  TypicalInspectableDataSink<int> sink{};

  TypicalRingBuffer<int> ring{source, sink};

  source.try_supply(ring.mtu());

  ASSERT_TRUE(sink.data_available());

  EXPECT_EQ(ring.mtu(), sink.try_read());

  int element = source.prev_element();
  for (size_t i = 0; i < ring.mtu(); ++i) {
    EXPECT_EQ(element, sink.last_buffer_read()[i]);
    ++element;
  }
}

TEST(TypicalRingBufferTest, SinkNotNotifiedOnWriteOfLessThanMtu) {
  TypicalSequentialDataSource<int> source{};
  TypicalInspectableDataSink<int> sink{};

  TypicalRingBuffer<int> ring{source, sink};

  source.try_supply(ring.mtu() - 1);

  EXPECT_FALSE(sink.data_available());
}

TEST(TypicalRingBufferTest, SourceCanFillRing) {
  TypicalSequentialDataSource<int> source{};
  TypicalInspectableDataSink<int> sink{};

  TypicalRingBuffer<int> ring{source, sink};

  const size_t elements_written{source.try_supply(ring.max_buffered_elements())};
  ASSERT_EQ(ring.max_buffered_elements(), source.next_element());
  ASSERT_EQ(ring.max_buffered_elements(), elements_written);

  EXPECT_EQ(ring.max_buffered_elements(), ring.elements_available());
  EXPECT_FALSE(source.data_needed());
}

TEST(TypicalRingBufferTest, SinkCanDrainRing) {
  TypicalSequentialDataSource<int> source{};
  TypicalInspectableDataSink<int> sink{};

  TypicalRingBuffer<int> ring{source, sink};

  const size_t elements_written{source.try_supply(ring.max_buffered_elements())};
  ASSERT_EQ(ring.max_buffered_elements(), source.next_element());
  ASSERT_EQ(ring.max_buffered_elements(), elements_written);

  ASSERT_EQ(ring.max_buffered_elements(), ring.elements_available());
  ASSERT_FALSE(source.data_needed());

  size_t total_elements_read{0};

  int expected_element = source.prev_element();
  while (total_elements_read < ring.max_buffered_elements()) {
    ASSERT_TRUE(sink.data_available());
    const size_t elements_read{sink.try_read()};
    ASSERT_EQ(ring.mtu(), elements_read);

    for (size_t i = 0; i < elements_read; ++i) {
      ASSERT_EQ(expected_element, sink.last_buffer_read()[i]);
      ++expected_element;
    }
    total_elements_read += elements_read;
  }

  EXPECT_EQ(0, ring.elements_available());
  EXPECT_FALSE(sink.data_available());
  EXPECT_TRUE(source.data_needed());
}

TEST(TypicalRingBufferTest, CapsWriteAtAnMtuWorthOfData) {
  TypicalSequentialDataSource<int> source{};
  TypicalInspectableDataSink<int> sink{};

  TypicalRingBuffer<int> ring{source, sink};

  constexpr size_t ELEMENTS_TO_WRITE{ring.mtu() + 1};

  const size_t elements_written{source.try_supply(ELEMENTS_TO_WRITE)};
  ASSERT_EQ(ELEMENTS_TO_WRITE, elements_written);
  ASSERT_EQ(ELEMENTS_TO_WRITE, ring.elements_available());
  ASSERT_EQ(ELEMENTS_TO_WRITE, source.next_element());

  EXPECT_EQ(2, source.num_write_calls());
}

TEST(LargeRingBufferTest, CallsDataNeededOnConstruction) {
  LargeSequentialDataSource<int> source{};
  LargeInspectableDataSink<int> sink{};

  ASSERT_FALSE(source.data_needed());
  ASSERT_FALSE(sink.data_available());

  std::unique_ptr{std::make_unique<LargeRingBuffer<int>>(source, sink)};
  EXPECT_TRUE(source.data_needed());
  EXPECT_FALSE(sink.data_available());
}

TEST(LargeRingBufferTest, CanAcceptDataFromSource) {
  LargeSequentialDataSource<int> source{};
  LargeInspectableDataSink<int> sink{};

  std::unique_ptr ring{std::make_unique<LargeRingBuffer<int>>(source, sink)};

  source.try_supply(1);
  EXPECT_EQ(1, ring->elements_available());
}

TEST(LargeRingBufferTest, SourceCanWriteAnMtuWorthOfData) {
  LargeSequentialDataSource<int> source{};
  LargeInspectableDataSink<int> sink{};

  std::unique_ptr ring{std::make_unique<LargeRingBuffer<int>>(source, sink)};

  const size_t elements_written{source.try_supply(ring->mtu())};
  EXPECT_EQ(ring->mtu(), elements_written);
  EXPECT_EQ(ring->mtu(), ring->elements_available());
  EXPECT_EQ(ring->mtu(), source.next_element());
}

TEST(LargeRingBufferTest, SinkNotifiedOnWriteOfMtu) {
  LargeSequentialDataSource<int> source{};
  LargeInspectableDataSink<int> sink{};

  std::unique_ptr ring{std::make_unique<LargeRingBuffer<int>>(source, sink)};

  source.try_supply(ring->mtu());

  EXPECT_TRUE(sink.data_available());
}

TEST(LargeRingBufferTest, SinkCanReadMtu) {
  LargeSequentialDataSource<int> source{};
  LargeInspectableDataSink<int> sink{};

  std::unique_ptr ring{std::make_unique<LargeRingBuffer<int>>(source, sink)};

  source.try_supply(ring->mtu());

  ASSERT_TRUE(sink.data_available());

  EXPECT_EQ(ring->mtu(), sink.try_read());

  int element = source.prev_element();
  for (size_t i = 0; i < ring->mtu(); ++i) {
    EXPECT_EQ(element, sink.last_buffer_read()[i]);
    ++element;
  }
}

TEST(LargeRingBufferTest, SinkNotNotifiedOnWriteOfLessThanMtu) {
  LargeSequentialDataSource<int> source{};
  LargeInspectableDataSink<int> sink{};

  std::unique_ptr ring{std::make_unique<LargeRingBuffer<int>>(source, sink)};

  source.try_supply(ring->mtu() - 1);

  EXPECT_FALSE(sink.data_available());
}

TEST(LargeRingBufferTest, SourceCanFillRing) {
  LargeSequentialDataSource<int> source{};
  LargeInspectableDataSink<int> sink{};

  std::unique_ptr ring{std::make_unique<LargeRingBuffer<int>>(source, sink)};

  const size_t elements_written{source.try_supply(ring->max_buffered_elements())};
  ASSERT_EQ(ring->max_buffered_elements(), source.next_element());
  ASSERT_EQ(ring->max_buffered_elements(), elements_written);

  EXPECT_EQ(ring->max_buffered_elements(), ring->elements_available());
  EXPECT_FALSE(source.data_needed());
}

TEST(LargeRingBufferTest, SinkCanDrainRing) {
  LargeSequentialDataSource<int> source{};
  LargeInspectableDataSink<int> sink{};

  std::unique_ptr ring{std::make_unique<LargeRingBuffer<int>>(source, sink)};

  const size_t elements_written{source.try_supply(ring->max_buffered_elements())};
  ASSERT_EQ(ring->max_buffered_elements(), source.next_element());
  ASSERT_EQ(ring->max_buffered_elements(), elements_written);

  ASSERT_EQ(ring->max_buffered_elements(), ring->elements_available());
  ASSERT_FALSE(source.data_needed());

  size_t total_elements_read{0};

  int expected_element = source.prev_element();
  while (total_elements_read < ring->max_buffered_elements()) {
    ASSERT_TRUE(sink.data_available());
    const size_t elements_read{sink.try_read()};
    ASSERT_EQ(ring->mtu(), elements_read);

    for (size_t i = 0; i < elements_read; ++i) {
      ASSERT_EQ(expected_element, sink.last_buffer_read()[i]);
      ++expected_element;
    }
    total_elements_read += elements_read;
  }

  EXPECT_EQ(0, ring->elements_available());
  EXPECT_FALSE(sink.data_available());
  EXPECT_TRUE(source.data_needed());
}

TEST(LargeRingBufferTest, CapsWriteAtAnMtuWorthOfData) {
  LargeSequentialDataSource<int> source{};
  LargeInspectableDataSink<int> sink{};

  std::unique_ptr ring{std::make_unique<LargeRingBuffer<int>>(source, sink)};

  const size_t ELEMENTS_TO_WRITE{ring->mtu() + 1};

  const size_t elements_written{source.try_supply(ELEMENTS_TO_WRITE)};
  ASSERT_EQ(ELEMENTS_TO_WRITE, elements_written);
  ASSERT_EQ(ELEMENTS_TO_WRITE, ring->elements_available());
  ASSERT_EQ(ELEMENTS_TO_WRITE, source.next_element());

  EXPECT_EQ(2, source.num_write_calls());
}

}  // namespace tvsc::buffer
