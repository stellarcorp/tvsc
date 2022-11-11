#pragma once

#include <condition_variable>
#include <mutex>

#include "buffer/ring_buffer.h"

namespace tvsc::buffer {

template <typename ElementT, size_t BUFFER_SIZE, size_t NUM_BUFFERS>
class BlockingDataSink final : public RingBuffer<ElementT, BUFFER_SIZE, NUM_BUFFERS>::DataSink {
 private:
  std::mutex mutex_{};
  std::condition_variable condition_variable_{};
  Buffer<ElementT, BUFFER_SIZE> buffer_{};
  bool data_available_{false};

 public:
  void signal_data_available() override {
    {
      std::unique_lock lock{mutex_};
      data_available_ = true;
    }
    condition_variable_.notify_one();
  }

  bool data_available() const { return data_available_; }

  template <typename Rep, typename Period>
  size_t try_read(size_t num_elements, ElementT* buffer, const std::chrono::duration<Rep, Period>& timeout) {
    size_t elements_read{0};
    std::unique_lock lock{mutex_};
    condition_variable_.wait_for(lock, timeout, [this](){ return this->data_available_; });
    if (data_available_) {
      data_available_ = false;
      elements_read = this->ring_buffer()->consume(buffer_.size(), buffer_.data());
    }
    return elements_read;
  }
};

}  // namespace tvsc::buffer
