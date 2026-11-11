#pragma once

#include <stdexcept>

#include "base/except.h"

namespace tvsc::buffer {

class BufferRef final {
  std::byte* address;
  size_t size;
  size_t alignment;
};

class BufferAllocator final {
 private:
  std::byte* const base_address_;
  const size_t total_size_;
  size_t offset_{0};

  [[nodiscard]] static constexpr size_t align_forward(size_t addr, size_t alignment) noexcept {
    return (addr + alignment - 1) & ~(alignment - 1);
  }

 public:
  constexpr BufferAllocator(std::byte* memory, size_t size_bytes) noexcept
      : base_address_(memory), total_size_(size_bytes), offset_(0) {}

  template <size_t N>
  constexpr explicit BufferAllocator(std::array<char, N>& memory) noexcept
      : base_address_(memory.data()), total_size_(N), offset_(0) {}

  template <size_t N>
  constexpr explicit BufferAllocator(std::array<std::byte, N>& memory) noexcept
      : base_address_(memory.data()), total_size_(N), offset_(0) {}

  [[nodiscard]] constexpr BufferRef allocate(size_t num_bytes, size_t alignment = 4) noexcept {
    const size_t aligned_offset{align_forward(offset_, alignment)};

    if (aligned_offset + num_bytes > total_size_) {
      except<std::bad_alloc>("BufferAllocator out of memory");
    }

    offset_ = aligned_offset + num_bytes;
    std::byte* const allocated_ptr{base_address_ + aligned_offset};
    return BufferRef{allocated_ptr, num_bytes, alignment};
  }

  [[nodiscard]] constexpr std::byte* data() noexcept { return base_address_; }
  [[nodiscard]] constexpr const std::byte* data() const noexcept { return base_address_; }

  [[nodiscard]] constexpr size_t size() const noexcept { return total_size_; }
  [[nodiscard]] constexpr size_t capacity() const noexcept { return total_size_; }
  [[nodiscard]] constexpr size_t bytes_allocated() const noexcept { return offset_; }
  [[nodiscard]] constexpr size_t available_bytes() const noexcept { return total_size_ - offset_; }

  /**
   * Reset this allocator. All buffers are effectively deallocated. No destructors will be
   * called. This approach follows the pattern of an arena allocator
   * (https://en.wikipedia.org/wiki/Region-based_memory_management).
   */
  constexpr void reset() noexcept { offset_ = 0; }
};
}  // namespace tvsc::buffer
