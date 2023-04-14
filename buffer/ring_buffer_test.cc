#include "buffer/ring_buffer.h"

#include <memory>

#include "buffer/buffer.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

namespace tvsc::buffer {

template <typename ElementT, size_t PAGE_SIZE, size_t NUM_PAGES, bool PRIORITIZE_OLD_ELEMENTS>
class SequentialDataSource final
    : public RingBuffer<ElementT, PAGE_SIZE, NUM_PAGES, PRIORITIZE_OLD_ELEMENTS>::DataSource {
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
    size_t total_elements_supplied{0};

    prev_element_ = next_element_;
    Buffer<ElementT, PAGE_SIZE> buffer{};
    while (num_elements > total_elements_supplied) {
      size_t elements_to_supply = std::min(PAGE_SIZE, num_elements - total_elements_supplied);
      for (size_t i = 0; i < elements_to_supply; ++i) {
        buffer.write(i, next_element_++);
      }
      data_needed_ = false;
      total_elements_supplied += this->ring_buffer()->supply(elements_to_supply, buffer.data());
      ++num_supply_calls_;
    }

    return total_elements_supplied;
  }

  size_t num_supply_calls() const { return num_supply_calls_; }

  void reset() {
    prev_element_ = ElementT{};
    next_element_ = ElementT{};
    num_supply_calls = 0;
    data_needed_ = false;
  }
};

template <typename ElementT, size_t PAGE_SIZE, size_t NUM_PAGES, bool PRIORITIZE_OLD_ELEMENTS>
class InspectableDataSink final
    : public RingBuffer<ElementT, PAGE_SIZE, NUM_PAGES, PRIORITIZE_OLD_ELEMENTS>::DataSink {
 private:
  Buffer<ElementT, PAGE_SIZE> buffer_{};
  bool data_available_{false};

 public:
  void signal_data_available() override { data_available_ = true; }

  const Buffer<ElementT, PAGE_SIZE>& last_buffer_read() const { return buffer_; }

  bool data_available() const { return data_available_; }

  size_t try_consume() {
    data_available_ = false;
    return this->ring_buffer()->consume(buffer_.size(), buffer_.data());
  }

  void reset() {
    buffer_.clear();
    data_available_ = false;
  }
};

template <typename SizesT>
class RingBufferTest : public ::testing::Test {
 public:
  using RingBufferT =
      RingBuffer<int, SizesT::buffer_size, SizesT::num_buffers, SizesT::prioritize_old_elements>;
  using DataSourceT = SequentialDataSource<int, SizesT::buffer_size, SizesT::num_buffers,
                                           SizesT::prioritize_old_elements>;
  using DataSinkT = InspectableDataSink<int, SizesT::buffer_size, SizesT::num_buffers,
                                        SizesT::prioritize_old_elements>;
};

/**
 * Tests for the RingBuffer where we prioritize new elements over old.
 */
template <typename SizesT>
class PrioritizeNewRingBufferTest : public RingBufferTest<SizesT> {};

template <size_t PAGE_SIZE, size_t NUM_PAGES, bool PRIORITIZE_OLD_ELEMENTS>
struct Sizes final {
  static constexpr size_t buffer_size{PAGE_SIZE};
  static constexpr size_t num_buffers{NUM_PAGES};
  static constexpr bool prioritize_old_elements{PRIORITIZE_OLD_ELEMENTS};
};

template <bool PRIORITIZE_OLD_ELEMENTS>
using TestSizes =
    ::testing::Types<Sizes<2, 3, PRIORITIZE_OLD_ELEMENTS>, Sizes<8, 8, PRIORITIZE_OLD_ELEMENTS>,
                     Sizes<1024, 16, PRIORITIZE_OLD_ELEMENTS>,
                     Sizes<1 << 11, 512, PRIORITIZE_OLD_ELEMENTS>>;

template <bool PRIORITIZE_OLD_ELEMENTS>
using SmallTestSizes =
    ::testing::Types<Sizes<2, 3, PRIORITIZE_OLD_ELEMENTS>, Sizes<8, 8, PRIORITIZE_OLD_ELEMENTS>>;

TYPED_TEST_SUITE(RingBufferTest, TestSizes<true>);
TYPED_TEST_SUITE(PrioritizeNewRingBufferTest, TestSizes<false>);

TYPED_TEST(RingBufferTest, CanUseWithoutDataSource) {
  typename TestFixture::DataSinkT sink{};

  typename TestFixture::RingBufferT ring{sink};

  static constexpr int value{1};
  ring.supply(value);

  EXPECT_EQ(1, sink.try_consume());

  EXPECT_EQ(value, sink.last_buffer_read()[0]);
}

TYPED_TEST(RingBufferTest, CallsDataNeededOnConstruction) {
  typename TestFixture::DataSourceT source{};
  typename TestFixture::DataSinkT sink{};

  ASSERT_FALSE(source.data_needed());
  ASSERT_FALSE(sink.data_available());

  typename TestFixture::RingBufferT{source, sink};
  EXPECT_TRUE(source.data_needed());
  EXPECT_FALSE(sink.data_available());
}

TYPED_TEST(RingBufferTest, CanAcceptDataFromSource) {
  typename TestFixture::DataSourceT source{};

  typename TestFixture::RingBufferT ring{source};

  source.try_supply(1);
  EXPECT_EQ(1, ring.elements_available());
}

TYPED_TEST(RingBufferTest, SourceCanWriteAnMtuWorthOfData) {
  typename TestFixture::DataSourceT source{};

  typename TestFixture::RingBufferT ring{source};

  const size_t elements_written{source.try_supply(ring.mtu())};
  EXPECT_EQ(ring.mtu(), elements_written);
  EXPECT_EQ(ring.mtu(), ring.elements_available());
  EXPECT_EQ(ring.mtu(), source.next_element());
}

TYPED_TEST(RingBufferTest, SourceCanWriteAnMtuFollowedBySingleElement) {
  typename TestFixture::DataSourceT source{};

  typename TestFixture::RingBufferT ring{source};

  size_t elements_written{source.try_supply(ring.mtu())};
  ASSERT_EQ(ring.mtu(), elements_written);
  ASSERT_EQ(ring.mtu(), ring.elements_available());
  ASSERT_EQ(ring.mtu(), source.next_element());

  elements_written = source.try_supply(1);
  EXPECT_EQ(1, elements_written);
  EXPECT_EQ(ring.mtu() + 1, ring.elements_available());
  EXPECT_EQ(ring.mtu() + 1, source.next_element());
}

TYPED_TEST(RingBufferTest, SourceCanWriteTwoMtusInARow) {
  typename TestFixture::DataSourceT source{};

  typename TestFixture::RingBufferT ring{source};

  size_t elements_written{source.try_supply(ring.mtu())};
  ASSERT_EQ(ring.mtu(), elements_written);
  ASSERT_EQ(ring.mtu(), ring.elements_available());
  ASSERT_EQ(ring.mtu(), source.next_element());

  elements_written = source.try_supply(ring.mtu());
  EXPECT_EQ(ring.mtu(), elements_written);
  EXPECT_EQ(2 * ring.mtu(), ring.elements_available());
  EXPECT_EQ(2 * ring.mtu(), source.next_element());
}

TYPED_TEST(RingBufferTest, SinkNotifiedOnWriteOfMtu) {
  typename TestFixture::DataSourceT source{};
  typename TestFixture::DataSinkT sink{};

  typename TestFixture::RingBufferT ring{source, sink};

  source.try_supply(ring.mtu());

  EXPECT_TRUE(sink.data_available());
}

TYPED_TEST(RingBufferTest, SinkCanReadSingleElement) {
  typename TestFixture::DataSourceT source{};
  typename TestFixture::DataSinkT sink{};

  typename TestFixture::RingBufferT ring{source, sink};

  source.try_supply(1);

  EXPECT_EQ(1, sink.try_consume());

  EXPECT_EQ(source.prev_element(), sink.last_buffer_read()[0]);
}

TYPED_TEST(RingBufferTest, SinkCanReadMtu) {
  typename TestFixture::DataSourceT source{};
  typename TestFixture::DataSinkT sink{};

  typename TestFixture::RingBufferT ring{source, sink};

  source.try_supply(ring.mtu());

  ASSERT_TRUE(sink.data_available());

  EXPECT_EQ(ring.mtu(), sink.try_consume());

  int element = source.prev_element();
  for (size_t i = 0; i < ring.mtu(); ++i) {
    EXPECT_EQ(element, sink.last_buffer_read()[i]);
    ++element;
  }
}

TYPED_TEST(RingBufferTest, CanPeekSingleElement) {
  typename TestFixture::RingBufferT ring{};

  ring.supply(1);

  const int* value{};
  EXPECT_TRUE(ring.peek(&value));
  EXPECT_EQ(1, *value);
}

TYPED_TEST(RingBufferTest, CanPeekPopSingleElement) {
  typename TestFixture::RingBufferT ring{};

  ring.supply(1);

  const int* value{};
  ASSERT_TRUE(ring.peek(&value));
  ASSERT_EQ(1, *value);

  EXPECT_TRUE(ring.pop());
  EXPECT_TRUE(ring.empty());
}

TYPED_TEST(RingBufferTest, SinkNotNotifiedOnWriteOfLessThanMtu) {
  typename TestFixture::DataSourceT source{};
  typename TestFixture::DataSinkT sink{};

  typename TestFixture::RingBufferT ring{source, sink};

  source.try_supply(ring.mtu() - 1);

  EXPECT_FALSE(sink.data_available());
}

TYPED_TEST(RingBufferTest, SourceCanFillRing) {
  typename TestFixture::DataSourceT source{};
  typename TestFixture::DataSinkT sink{};

  typename TestFixture::RingBufferT ring{source, sink};

  const size_t elements_written{source.try_supply(ring.max_buffered_elements())};
  ASSERT_EQ(ring.max_buffered_elements(), source.next_element());
  ASSERT_EQ(ring.max_buffered_elements(), elements_written);

  EXPECT_EQ(ring.max_buffered_elements(), ring.elements_available());
  EXPECT_FALSE(source.data_needed());
}

TYPED_TEST(RingBufferTest, SinkCanDrainRing) {
  typename TestFixture::DataSourceT source{};
  typename TestFixture::DataSinkT sink{};

  typename TestFixture::RingBufferT ring{source, sink};

  const size_t elements_written{source.try_supply(ring.max_buffered_elements())};
  ASSERT_EQ(ring.max_buffered_elements(), source.next_element());
  ASSERT_EQ(ring.max_buffered_elements(), elements_written);

  ASSERT_EQ(ring.max_buffered_elements(), ring.elements_available());
  ASSERT_FALSE(source.data_needed());

  size_t total_elements_read{0};

  int expected_element = source.prev_element();
  while (total_elements_read < ring.max_buffered_elements()) {
    ASSERT_TRUE(sink.data_available());
    const size_t elements_read{sink.try_consume()};
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

TYPED_TEST(RingBufferTest, CapsWriteAtAnMtuWorthOfData) {
  typename TestFixture::DataSourceT source{};
  typename TestFixture::DataSinkT sink{};

  typename TestFixture::RingBufferT ring{source, sink};

  constexpr size_t ELEMENTS_TO_WRITE{ring.mtu() + 1};

  const size_t elements_written{source.try_supply(ELEMENTS_TO_WRITE)};
  ASSERT_EQ(ELEMENTS_TO_WRITE, elements_written);
  ASSERT_EQ(ELEMENTS_TO_WRITE, ring.elements_available());
  ASSERT_EQ(ELEMENTS_TO_WRITE, source.next_element());

  EXPECT_EQ(2, source.num_supply_calls());
}

TYPED_TEST(PrioritizeNewRingBufferTest, SourceCanOverfillRingIfPrioritizeOldElementsIsFalse) {
  typename TestFixture::DataSourceT source{};
  typename TestFixture::DataSinkT sink{};

  typename TestFixture::RingBufferT ring{source, sink};

  const size_t first_elements_written{source.try_supply(ring.max_buffered_elements())};
  ASSERT_EQ(ring.max_buffered_elements(), source.next_element());
  ASSERT_EQ(ring.max_buffered_elements(), first_elements_written);

  ASSERT_EQ(ring.max_buffered_elements(), ring.elements_available());
  ASSERT_FALSE(source.data_needed());

  const size_t second_elements_written{source.try_supply(1)};
  EXPECT_EQ(1, second_elements_written);
}

TYPED_TEST(PrioritizeNewRingBufferTest,
           SourceCanOverfillRingByAnMtuIfPrioritizeOldElementsIsFalse) {
  typename TestFixture::DataSinkT sink{};
  typename TestFixture::DataSourceT source{};

  typename TestFixture::RingBufferT ring{source, sink};

  const size_t first_elements_written{source.try_supply(ring.max_buffered_elements())};
  ASSERT_EQ(ring.max_buffered_elements(), source.next_element());
  ASSERT_EQ(ring.max_buffered_elements(), first_elements_written);

  ASSERT_EQ(ring.max_buffered_elements(), ring.elements_available());
  ASSERT_FALSE(source.data_needed());

  LOG(INFO) << "Starting overfill.";

  static constexpr size_t NUM_ELEMENTS_TO_SUPPLY{3 /*ring.mtu()*/};
  const size_t second_elements_written{source.try_supply(NUM_ELEMENTS_TO_SUPPLY)};
  EXPECT_EQ(NUM_ELEMENTS_TO_SUPPLY, second_elements_written);
}

TYPED_TEST(PrioritizeNewRingBufferTest,
           SourceCanOverfillRingManyTimesIfPrioritizeOldElementsIsFalse) {
  typename TestFixture::DataSourceT source{};
  typename TestFixture::DataSinkT sink{};

  typename TestFixture::RingBufferT ring{source, sink};

  const size_t first_elements_written{source.try_supply(ring.max_buffered_elements())};
  ASSERT_EQ(ring.max_buffered_elements(), source.next_element());
  ASSERT_EQ(ring.max_buffered_elements(), first_elements_written);

  ASSERT_EQ(ring.max_buffered_elements(), ring.elements_available());
  ASSERT_FALSE(source.data_needed());

  static constexpr int TIMES_TO_OVERFILL{1};
  for (int i = 0; i < TIMES_TO_OVERFILL; ++i) {
    static constexpr size_t NUM_ELEMENTS_TO_SUPPLY{ring.mtu()};
    const size_t elements_written{source.try_supply(NUM_ELEMENTS_TO_SUPPLY)};
    EXPECT_EQ(NUM_ELEMENTS_TO_SUPPLY, elements_written);
  }
}

}  // namespace tvsc::buffer
