#pragma once

#include <cstdint>

namespace tvsc::bus::spi {

class SpiBus;

/**
 * RIAA approach to managing transactions on an SpiBus. Hold an instance of this class on the stack
 * while doing operations on the bus.
 */
class SpiTransaction final {
 private:
  SpiBus* bus_{nullptr};

 public:
  SpiTransaction() = default;

  SpiTransaction(SpiBus& bus) : bus_(&bus) {}

  ~SpiTransaction();

  /**
   * Disallow copying of this transaction.
   */
  SpiTransaction(const SpiTransaction&) = delete;
  SpiTransaction& operator=(const SpiTransaction&) = delete;

  /**
   * Allow moving from another transaction to this transaction.
   */
  SpiTransaction(SpiTransaction&& rhs) {
    bus_ = rhs.bus_;
    rhs.bus_ = nullptr;
  }

  /**
   * Allow moving from another transaction to this transaction.
   */
  SpiTransaction& operator=(SpiTransaction&& rhs) {
    SpiBus* const prev{bus_};
    bus_ = rhs.bus_;
    rhs.bus_ = prev;
    return *this;
  }
};

/**
 * RIAA approach to managing peripheral selections on an SpiBus. Hold an instance of this class on
 * the stack while doing operations on a selected peripheral on the bus.
 */
class SpiPeripheralSelection final {
 private:
  SpiBus* bus_{nullptr};
  uint8_t peripheral_select_pin_{0xff};

 public:
  SpiPeripheralSelection() = default;

  SpiPeripheralSelection(SpiBus& bus, uint8_t peripheral_select_pin)
      : bus_(&bus), peripheral_select_pin_(peripheral_select_pin) {}

  ~SpiPeripheralSelection();

  /**
   * Disallow copying of a peripheral selection.
   */
  SpiPeripheralSelection(const SpiPeripheralSelection&) = delete;
  SpiPeripheralSelection& operator=(const SpiPeripheralSelection&) = delete;

  /**
   * Allow moving from another peripheral selection to this selection.
   */
  SpiPeripheralSelection(SpiPeripheralSelection&& rhs) {
    bus_ = rhs.bus_;
    peripheral_select_pin_ = rhs.peripheral_select_pin_;
    rhs.bus_ = nullptr;
    rhs.peripheral_select_pin_ = 0xff;
  }

  /**
   * Allow moving from another peripheral selection to this selection.
   */
  SpiPeripheralSelection& operator=(SpiPeripheralSelection&& rhs) {
    SpiBus* const prev_bus{bus_};
    const uint8_t prev_pin{peripheral_select_pin_};

    bus_ = rhs.bus_;
    peripheral_select_pin_ = rhs.peripheral_select_pin_;

    rhs.bus_ = prev_bus;
    rhs.peripheral_select_pin_ = prev_pin;

    return *this;
  }
};

/**
 * Encapsulation of operations on an SPI bus. Different platforms will provide different
 * implementations of the methods in this class.
 *
 * For folks familiar with the old names for SPI signals, but not the new names, please read:
 * https://www.oshwa.org/a-resolution-to-redefine-spi-signal-names/
 */
class SpiBus {
 private:
  void* spi_;

  void end_transaction();

  void initialize_peripheral(uint8_t peripheral_select_pin, uint8_t write_address_mask);
  void deselect_peripheral(uint8_t peripheral_select_pin);

  friend class SpiTransaction;
  friend class SpiPeripheralSelection;
  friend class SpiPeripheral;

 public:
  SpiBus(void* spi_implementation);
  ~SpiBus();

  void init();
  
  /**
   * Indicate to the bus that a particular interrupt number is in use elsewhere. This method is
   * based on the Arduino SPI function called "usingInterrupt()". It's documentation
   * (https://reference.arduino.cc/reference/en/language/functions/communication/spi/usinginterrupt/)
   * says:
   *
   * If your program will perform SPI transactions within an interrupt, call this function to
   * register the interrupt number or name with the SPI library. This allows
   * SPI.beginTransaction() to prevent usage conflicts. Note that the interrupt specified in the
   * call to usingInterrupt() will be disabled on a call to beginTransaction() and re-enabled in
   * endTransaction().
   *
   */
  void using_interrupt(uint8_t interrupt_number);

  /**
   * Start a transaction on this SPI bus. This is a low-level function to handle use cases that
   * aren't met by the rest of the API.
   */
  [[nodiscard]] SpiTransaction begin_transaction();

  /**
   * Indicate to a peripheral that the next SPI messages are targeted at it. This is a low-level
   * function to handle use cases that aren't met by the rest of the API.
   */
  [[nodiscard]] SpiPeripheralSelection select_peripheral(uint8_t peripheral_select_pin);

  /**
   * Transfer a single byte via the SPI bus. This is a low-level function to handle use cases that
   * aren't met by the rest of the API.
   */
  uint8_t transfer(uint8_t value);
};

/**
 * Encapsulation of operations on a single SPI peripheral. This is a generic implementation using a
 * standard approach for reading and writing to SPI peripherals. This implementation should handle
 * most use cases for most bus implementations and peripherals. If something isn't covered, the
 * SpiBus can be used directly.
 */
class SpiPeripheral {
 private:
  SpiBus* const bus_;
  uint8_t peripheral_select_pin_;
  uint8_t write_address_mask_;

  friend class SpiBus;

 public:
  SpiPeripheral(SpiBus& bus, uint8_t peripheral_select_pin, uint8_t write_address_mask)
      : bus_(&bus),
        peripheral_select_pin_(peripheral_select_pin),
        write_address_mask_(write_address_mask) {
    bus_->initialize_peripheral(peripheral_select_pin_, write_address_mask_);
  }

  /**
   * Accessor to use the bus directly when necessary.
   */
  SpiBus& bus() { return *bus_; }

  /**
   * Read a value from an address. Returns the value at address.
   */
  uint8_t read(uint8_t address) {
    SpiTransaction transaction{bus_->begin_transaction()};
    SpiPeripheralSelection selection{select()};
    bus_->transfer(address & ~write_address_mask_);
    return bus_->transfer(0);
  }

  /**
   * Write a single value to an address. Returns the value read when transferring the address onto
   * the bus.
   */
  uint8_t write(uint8_t address, uint8_t value) {
    uint8_t status{};
    SpiTransaction transaction{bus_->begin_transaction()};
    SpiPeripheralSelection selection{select()};

    status = bus_->transfer(address | write_address_mask_);
    bus_->transfer(value);

    return status;
  }

  /**
   * Read count values into buffer from sequential addresses starting with address. Buffer must have
   * storage for count values. Returns status read when the address was transferred onto the bus.
   */
  uint8_t burst_read(uint8_t address, uint8_t* buffer, uint8_t count) {
    uint8_t status{};
    SpiTransaction transaction{bus_->begin_transaction()};
    SpiPeripheralSelection selection{select()};

    status = bus_->transfer(address & ~write_address_mask_);

    while (count--) {
      *buffer++ = bus_->transfer(0);
    }

    return status;
  }

  /**
   * Write count values from buffer from sequential addresses starting with address. Returns status
   * read when the address was transferred onto the bus.
   */
  uint8_t burst_write(uint8_t address, const uint8_t* buffer, uint8_t count) {
    uint8_t status{};
    SpiTransaction transaction{bus_->begin_transaction()};
    SpiPeripheralSelection selection{select()};

    status = bus_->transfer(address | write_address_mask_);

    while (count--) {
      bus_->transfer(*buffer++);
    }

    return status;
  }

  /**
   * Read up to count values into buffer from a single address. Buffer must have
   * storage for count values.
   *
   * The first byte transfered on the SDO line will be the address to read from. The method expects
   * that the first byte read will be the number of bytes available in the FIFO, and subsequent
   * reads will return the data from the FIFO.
   *
   * If count is smaller than the bytes available in the FIFO, no bytes will be read.
   *
   * Returns the number of bytes actually read.
   */
  uint8_t fifo_read(uint8_t address, uint8_t* buffer, uint8_t count) {
    SpiTransaction transaction{bus_->begin_transaction()};
    SpiPeripheralSelection selection{select()};

    bus_->transfer(address & ~write_address_mask_);
    uint8_t bytes_available = bus_->transfer(0);

    if (count >= bytes_available) {
      count = bytes_available;
      while (bytes_available--) {
        *buffer++ = bus_->transfer(0);
      }
    } else {
      count = 0;
    }

    return count;
  }

  /**
   * Write count values from buffer to a single address.
   *
   * The first byte transfered on the SDO line will be the address to write to or'd with the write
   * address mask. The next byte written will be count, and subsequent writes will transfer data
   * from buffer to the FIFO.
   *
   * Returns the number of bytes actually written.
   */
  uint8_t fifo_write(uint8_t address, const uint8_t* buffer, uint8_t count) {
    uint8_t status{};
    SpiTransaction transaction{bus_->begin_transaction()};
    SpiPeripheralSelection selection{select()};

    status = bus_->transfer(address | write_address_mask_);

    bus_->transfer(count);
    while (count--) {
      bus_->transfer(*buffer++);
    }

    return status;
  }

  /**
   * Indicate to this peripheral that the next SPI messages are targeted at it. This is a low-level
   * function to handle use cases that aren't met by the rest of the API.
   */
  [[nodiscard]] SpiPeripheralSelection select() {
    return bus_->select_peripheral(peripheral_select_pin_);
  }
};

/**
 * All of these factory functions may not exist on all platforms. If there is a link error, either
 * it wasn't implemented, or that bus is not available on that platform due to hardware
 * limitations.
 */
SpiBus get_default_spi_bus();
SpiBus get_second_spi_bus();
SpiBus get_third_spi_bus();
SpiBus get_fourth_spi_bus();

}  // namespace tvsc::bus::spi
