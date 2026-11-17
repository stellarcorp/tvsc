#include "error/fault_message.h"

#include "platforms/linker_sections.h"

namespace tvsc::error {
FAULT_BUFFER FaultMessage<256> fault_message{};
}
