#include "except_no_output.h"
#include "platforms/linker_sections.h"

namespace tvsc {
FAULT_BUFFER FixedBuffer<256> fault_message{};
}
