#include <sys/types.h>

void* _sbrk(ptrdiff_t incr) {
    // Force an immediate hard fault / trap if dynamic allocation is attempted
    __builtin_trap();

    // Return standard error pointer as a fallback
    return (void*)-1;
}
