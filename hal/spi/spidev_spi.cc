#include "hal/spi/spi.h"

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <mutex>

#include "base/except.h"
#include "glog/logging.h"
#include "hal/gpio/pins.h"
#include "wiringPi.h"

namespace tvsc::hal::spi {

struct BusState final {
  int fd{-1};
  spi_ioc_transfer xfer{};
  std::mutex transaction_mutex{};
};

inline BusState* as_state(void* ptr) {
  if (ptr == nullptr) {
    except<std::runtime_error>(
        "No BusState. Perhaps the SpiBus BusState got moved, but we continued working with the one "
        "without state?");
  }
  return reinterpret_cast<BusState*>(ptr);
}

SpiBus::SpiBus(void* device_name) : spi_(reinterpret_cast<void*>(new BusState())) {
  as_state(spi_)->fd = open(reinterpret_cast<const char*>(device_name), O_RDWR);
  if (as_state(spi_)->fd < 0) {
    except<std::runtime_error>("Could not open device");
  }

  // TODO(james): Change the API to allow these settings to be different for each peripheral.
  int fd = as_state(spi_)->fd;

  uint8_t bits{8};  // Bits per word.
  int status = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
  if (status == -1) {
    except<std::runtime_error>("Failed to set bits per word.");
  }

  uint8_t justification{0};  // MSB first.
  status = ioctl(fd, SPI_IOC_WR_LSB_FIRST, &justification);
  if (status == -1) {
    except<std::runtime_error>("Failed to bit justification (MSB or LSB).");
  }

  uint32_t speed{2'000'000};  // Speed of transfers in Hz.
  status = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
  if (status == -1) {
    except<std::runtime_error>("Failed to set transfer speed.");
  }

  uint8_t mode{SPI_MODE_0};
  status = ioctl(fd, SPI_IOC_WR_MODE, &mode);
  if (status == -1) {
    except<std::runtime_error>("Failed to set SPI transfer mode.");
  }
}

SpiBus::~SpiBus() {
  LOG(INFO) << "SpiBus::~SpiBus()";
  if (spi_ != nullptr) {
    close(as_state(spi_)->fd);
    delete as_state(spi_);
  }
}

void SpiBus::initialize_peripheral(uint8_t peripheral_select_pin) {
  tvsc::hal::gpio::set_mode(peripheral_select_pin, tvsc::hal::gpio::PinMode::MODE_OUTPUT);

  // Deselect the peripheral by raising the peripheral's chipselect pin to HIGH.
  digitalWrite(peripheral_select_pin, HIGH);
}

void SpiBus::using_interrupt(uint8_t pin) {
  // TODO(james): It's not clear what this should do, if anything.
  // We lock a transaction_mutex on begin_transaction and unlock it in end_transaction. If this bus
  // is being used without transactions, it seems like interruptions should be expected by the
  // calling code.
}

SpiTransaction SpiBus::begin_transaction(uint8_t peripheral_select_pin) {
  as_state(spi_)->transaction_mutex.lock();

  // Select the peripheral by dropping the peripheral's chipselect pin to LOW.
  digitalWrite(peripheral_select_pin, LOW);

  return SpiTransaction{*this, peripheral_select_pin};
}

void SpiBus::end_transaction(uint8_t peripheral_select_pin) {
  // Construct an spi_transfer with dummy_data set to true and put it in a message. The
  // cs_change bit should be set to 0 to indicate that the normal chipselect behavior (deselecting
  // the peripheral, usually by raising the chipselect line to high) should be observed at the end
  // of the message.
  struct spi_ioc_transfer& xfer{as_state(spi_)->xfer};

  std::memset(&xfer, 0, sizeof(struct spi_ioc_transfer));
  // Note that all of these are redundant with the memset call, but are enumerated explicitly to
  // explain the intent here.
  // We don't really want to transfer any data; we just want to deselect the chip.
  xfer.tx_buf = 0 /* nullptr */;
  xfer.rx_buf = 0 /* nullptr */;
  xfer.len = 0;
  xfer.cs_change = 0;

  int status = ioctl(as_state(spi_)->fd, SPI_IOC_MESSAGE(1), &xfer);
  if (status < 0) {
    except<std::runtime_error>("Could not perform SPI transfer.");
  }

  // Deselect the peripheral by raising the peripheral's chipselect pin to HIGH.
  digitalWrite(peripheral_select_pin, HIGH);

  as_state(spi_)->transaction_mutex.unlock();
}

uint8_t SpiBus::transfer(uint8_t value) {
  // Construct an spi_transfer with the given value and put it in a message. The
  // cs_change bit should be set to 1 to indicate that the normal chipselect behavior (deselecting
  // the peripheral, usually by raising the chipselect line to high) should be ignored at the end
  // of the message. In this case, we expect the chipselect to keep the peripheral selected after
  // transferring the message.
  struct spi_ioc_transfer& xfer{as_state(spi_)->xfer};

  uint8_t result{};

  std::memset(&xfer, 0, sizeof(struct spi_ioc_transfer));
  xfer.tx_buf = reinterpret_cast<uint64_t>(&value);
  xfer.rx_buf = reinterpret_cast<uint64_t>(&result);
  xfer.len = 1;
  xfer.cs_change = 1;

  int status = ioctl(as_state(spi_)->fd, SPI_IOC_MESSAGE(1), &xfer);
  if (status < 0) {
    except<std::runtime_error>("Could not perform SPI transfer.");
  }

  return result;
}

SpiBus get_default_spi_bus() { return SpiBus(const_cast<char*>("/dev/spidev0.0")); }

}  // namespace tvsc::hal::spi
