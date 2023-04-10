#pragma once

#include <array>
#include <atomic>
#include <mutex>
#include <thread>

#include "buffer/buffer.h"

namespace tvsc::buffer {

/**
 * Lock-free paged RingBuffer with a single optional source and a single optional sink.
 */
template <typename ElementT, size_t PAGE_SIZE, size_t NUM_PAGES,
          bool PRIORITIZE_OLD_ELEMENTS = true>
class RingBuffer final {
 public:
  class DataSource {
   private:
    RingBuffer* ring_buffer_{nullptr};

   public:
    virtual ~DataSource() = default;

    RingBuffer* ring_buffer() { return ring_buffer_; }
    void set_ring_buffer(RingBuffer& ring_buffer) { ring_buffer_ = &ring_buffer; }

    virtual void signal_data_needed() = 0;
  };

  class DataSink {
   private:
    RingBuffer* ring_buffer_{nullptr};

   public:
    virtual ~DataSink() = default;

    RingBuffer* ring_buffer() { return ring_buffer_; }
    void set_ring_buffer(RingBuffer& ring_buffer) { ring_buffer_ = &ring_buffer; }

    virtual void signal_data_available() = 0;
  };

 private:
  // These pointers are monotonically increasing. They count the total number of elements written
  // into the ring and the total number of elements read from the ring. They do not reset to zero.
  // This is done to keep the logic simple. In particular, it can distinguish between the ring being
  // full and the ring being empty. In both states, the read_pointer_ and write_pointer_ would be
  // equal if we were to add to these values modulus the number of max_buffered_elements().
  std::atomic<size_t> read_pointer_{0};
  std::atomic<size_t> write_pointer_{0};

  std::array<Buffer<ElementT, PAGE_SIZE>, NUM_PAGES> buffers_{};
  DataSource* source_{nullptr};
  DataSink* sink_{nullptr};

  void check_data_available() {
    // If we have an mtu's worth of data, signal the sink that it can read more data.
    if (sink_ != nullptr) {
      if (elements_available() >= mtu()) {
        sink_->signal_data_available();
      }
    }
  }

  void check_data_needed() {
    // If we have space to store an mtu's worth of data, signal the source to provide more data.
    if (source_ != nullptr) {
      if (max_buffered_elements() - elements_available() >= mtu()) {
        source_->signal_data_needed();
      }
    }
  }

  size_t compute_buffer_index(size_t pointer) const { return pointer / PAGE_SIZE; }

  size_t compute_buffer_offset(size_t pointer) const { return pointer % PAGE_SIZE; }

  size_t compute_pointer(size_t index, size_t offset) const { return index * PAGE_SIZE + offset; }

  size_t compute_elements_available(size_t read_pointer_value, size_t write_pointer_value) const {
    return write_pointer_value - read_pointer_value;
  }

  size_t compute_elements_to_accept(size_t num_elements, size_t write_buffer_offset,
                                    size_t read_pointer_value, size_t write_pointer_value) const {
    if constexpr (PRIORITIZE_OLD_ELEMENTS) {
      return std::min(max_buffered_elements() -
                          compute_elements_available(read_pointer_value, write_pointer_value),
                      std::min(PAGE_SIZE - write_buffer_offset, num_elements));
    } else {
      return std::min(max_buffered_elements(),
                      std::min(PAGE_SIZE - write_buffer_offset, num_elements));
    }
  }

 public:
  RingBuffer() = default;

  RingBuffer(DataSource& source, DataSink& sink) : source_(&source), sink_(&sink) {
    source_->set_ring_buffer(*this);
    sink_->set_ring_buffer(*this);
    source_->signal_data_needed();
  }

  constexpr size_t mtu() const { return PAGE_SIZE; }
  constexpr size_t buffer_size() const { return PAGE_SIZE; }
  constexpr size_t num_buffers() const { return NUM_PAGES; }
  constexpr size_t max_buffered_elements() const { return NUM_PAGES * PAGE_SIZE; }

  /**
   * Consume up to num_elements from the RingBuffer. The number of elements made available may be
   * fewer than num_elements based on number of elements available and how they align with the
   * internal buffering.
   *
   * Returns the number of elements actually consumed.
   */
  size_t consume(size_t num_elements, ElementT* dest) {
    size_t read_pointer_value{read_pointer_.load()};
    size_t write_pointer_value{write_pointer_.load()};

    size_t read_buffer_index{compute_buffer_index(read_pointer_value)};
    size_t read_buffer_offset{compute_buffer_offset(read_pointer_value)};

    size_t elements_consumed{
        std::min(compute_elements_available(read_pointer_value, write_pointer_value),
                 std::min(PAGE_SIZE - read_buffer_offset, num_elements))};

    if (elements_consumed > 0) {
      buffers_[read_buffer_index % NUM_PAGES].read(read_buffer_offset, elements_consumed, dest);

      read_buffer_offset += elements_consumed;
      size_t new_read_pointer_value = compute_pointer(read_buffer_index, read_buffer_offset);
      if (!read_pointer_.compare_exchange_strong(read_pointer_value, new_read_pointer_value)) {
        elements_consumed = 0;
      }
    }

    check_data_available();
    check_data_needed();

    return elements_consumed;
  }

  /**
   * Consume a single element from the RingBuffer.
   *
   * Returns true if an element was consumed.
   */
  bool consume(ElementT* dest) { return consume(1, dest) == 1; }

  /**
   * Peek at a single element from the RingBuffer.
   *
   * Returns true if an element was available.
   */
  bool peek(const ElementT** const dest) const {
    size_t read_pointer_value{read_pointer_.load()};
    size_t write_pointer_value{write_pointer_.load()};

    size_t read_buffer_index{compute_buffer_index(read_pointer_value)};
    size_t read_buffer_offset{compute_buffer_offset(read_pointer_value)};

    bool elements_available{compute_elements_available(read_pointer_value, write_pointer_value) >
                            0};

    if (elements_available) {
      *dest = &buffers_[read_buffer_index % NUM_PAGES][read_buffer_offset];
    }

    return elements_available;
  }

  /**
   * Pop a single element from the RingBuffer, if one is available.
   *
   * Returns true if an element was available to be removed.
   */
  bool pop() {
    size_t read_pointer_value{read_pointer_.load()};
    size_t write_pointer_value{write_pointer_.load()};

    size_t read_buffer_index{compute_buffer_index(read_pointer_value)};
    size_t read_buffer_offset{compute_buffer_offset(read_pointer_value)};

    bool elements_available{compute_elements_available(read_pointer_value, write_pointer_value) >
                            0};

    if (elements_available) {
      read_buffer_offset += 1;
      size_t new_read_pointer_value = compute_pointer(read_buffer_index, read_buffer_offset);
      if (!read_pointer_.compare_exchange_strong(read_pointer_value, new_read_pointer_value)) {
        elements_available = false;
      }
    }

    check_data_available();
    check_data_needed();

    return elements_available;
  }

  /**
   * Supply num_elements to the RingBuffer. The number of elements actually copied into the
   * RingBuffer may be fewer than num_elements based on space available and alignment with internal
   * buffering.
   *
   * Returns the number of elements actually copied into the RingBuffer.
   */
  size_t supply(size_t num_elements, const ElementT* src) {
    size_t read_pointer_value{read_pointer_.load()};
    size_t write_pointer_value{write_pointer_.load()};

    size_t write_buffer_index{compute_buffer_index(write_pointer_value)};
    size_t write_buffer_offset{compute_buffer_offset(write_pointer_value)};

    const size_t elements_available{
        compute_elements_available(read_pointer_value, write_pointer_value)};
    size_t elements_supplied{compute_elements_to_accept(num_elements, write_buffer_offset,
                                                        read_pointer_value, write_pointer_value)};

    if (elements_supplied > 0) {
      size_t new_read_pointer_value{read_pointer_value};
      if (elements_available + elements_supplied > max_buffered_elements()) {
        if constexpr (PRIORITIZE_OLD_ELEMENTS) {
          throw std::logic_error(
              "elements_supplied should never have a value that makes this true.");
        } else {
          new_read_pointer_value +=
              elements_available + elements_supplied - max_buffered_elements();
          if (!read_pointer_.compare_exchange_strong(read_pointer_value, new_read_pointer_value)) {
            elements_supplied = 0;
          }
        }
      }

      if (elements_supplied > 0) {
        buffers_[write_buffer_index % NUM_PAGES].write(write_buffer_offset, elements_supplied, src);

        write_buffer_offset += elements_supplied;

        size_t new_write_pointer_value = compute_pointer(write_buffer_index, write_buffer_offset);

        if (!write_pointer_.compare_exchange_strong(write_pointer_value, new_write_pointer_value)) {
          elements_supplied = 0;
        }
      }
    }

    check_data_available();
    check_data_needed();

    return elements_supplied;
  }

  bool supply(const ElementT& src) { return supply(1, &src) == 1; }

  size_t elements_available() const {
    size_t read_pointer_value{read_pointer_.load()};
    size_t write_pointer_value{write_pointer_.load()};

    return compute_elements_available(read_pointer_value, write_pointer_value);
  }

  /**
   * A RingBuffer is full if it has the maximum number of elements available.
   */
  bool full() const { return elements_available() == max_buffered_elements(); }

  bool empty() const {
    size_t read_pointer_value{read_pointer_.load()};
    size_t write_pointer_value{write_pointer_.load()};

    return read_pointer_value == write_pointer_value;
  }
};

}  // namespace tvsc::buffer
