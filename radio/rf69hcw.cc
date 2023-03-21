#include "radio/rf69hcw.h"

#include <cstdint>

namespace tvsc::radio {

RF69HCW* RF69HCW::interrupt_devices_[3];

uint8_t RF69HCW::next_interrupt_index_{0};

void RF69HCW::isr0() { interrupt_devices_[0]->handle_interrupt(); }
void RF69HCW::isr1() { interrupt_devices_[1]->handle_interrupt(); }
void RF69HCW::isr2() { interrupt_devices_[2]->handle_interrupt(); }

}  // namespace tvsc::radio
