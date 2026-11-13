#pragma once

#include <stdexcept>

#include "base/except.h"
#include "hal2/device.h"

namespace tvsc::hal2::dma {

struct DmaSessionConfiguration final {
  enum class TransferWidth {
    BYTE,
    HALF_WORD,
    WORD,
  };

  enum class StreamMode {
    SINGLE_SHOT,
    CONTINUOUS,
  };

  enum class Priority {
    LOW,
    MEDIUM,
    HIGH,
    VITAL,
  };

  enum class Direction {
    FROM_PERIPHERAL,
    TO_PERIPHERAL,
  };

  DeviceId peripheral_device{};
  TransferWidth peripheral_data_width{TransferWidth::BYTE};
  TransferWidth memory_data_width{TransferWidth::BYTE};

  Direction direction{Direction::FROM_PERIPHERAL};
  StreamMode stream_mode{StreamMode::SINGLE_SHOT};

  Priority priority{Priority::LOW};
};

class DmaChannel;

// TODO(james): Redesign this to be more like the Pin and PinDevice classes in GPIO. Channels will
// still need to be acquire()'d to have only a single owner at a time.
class DmaController : public ChannelDevice<DmaController, DmaChannel> {
 private:
  virtual void enable() noexcept = 0;
  virtual void disable() noexcept = 0;

  virtual void release_channel(uint8_t channel_id) noexcept = 0;

  virtual void configure(uint8_t channel_id, const DmaSessionConfiguration&) noexcept = 0;

  [[nodiscard]] virtual void* implementation_handle(uint8_t channel_index) noexcept = 0;
  [[nodiscard]] virtual const void* implementation_handle(uint8_t channel_index) const noexcept = 0;

  friend DmaChannel;
  friend ChannelDriver<DmaController, DmaChannel>;

 public:
  [[nodiscard]] virtual DmaChannel acquire_channel(uint8_t channel_index) noexcept = 0;
};

class DmaChannel final : public ChannelDriver<DmaController, DmaChannel> {
 public:
  constexpr DmaChannel() noexcept = default;
  constexpr DmaChannel(DmaController& device, uint8_t channel_id) noexcept
      : ChannelDriver<DmaController, DmaChannel>(device, channel_id) {}

  constexpr DmaChannel(DmaChannel&& rhs) noexcept = default;
  constexpr DmaChannel& operator=(DmaChannel&& rhs) noexcept = default;

  void configure(const DmaSessionConfiguration& config) noexcept {
    if (device_ == nullptr) [[unlikely]] {
      except<std::logic_error>("DmaChannel configured when it was not valid.");
    }
    device_->configure(channel_id_, config);
  }

  /**
   * Provide access to an implementation dependent handle. This handle is typically used to link a
   * DMA channel to a peripheral.
   */
  template <typename Handle>
  [[nodiscard]] Handle* implementation_handle() noexcept {
    return reinterpret_cast<Handle*>(device_->implementation_handle(channel_id_));
  }

  template <typename Handle>
  [[nodiscard]] const Handle* implementation_handle() const noexcept {
    return reinterpret_cast<const Handle*>(device_->implementation_handle(channel_id_));
  }
};

}  // namespace tvsc::hal2::dma
