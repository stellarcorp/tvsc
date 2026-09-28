#include <errno.h>
#include <stdint.h>
#include <sys/types.h>

/**
 * Mark a particular branch in an if-block as unlikely to occur to help optimize branch prediction.
 *
 * TODO(james): Replace with [[unlikely]] when moved to c23 dialect.
 */
#define unlikely(x) __builtin_expect(!!(x), 0)

void* _sbrk(ptrdiff_t incr)
{
    static uint8_t *heap_end = NULL;
    extern uint8_t _end;             /* Symbol defined in linker script */
    extern uint8_t _estack;          /* Symbol defined in linker script */
    extern uint32_t _Min_Stack_Size; /* Symbol defined in linker script */

    const uint32_t stack_limit = (uint32_t)&_estack - (uint32_t)&_Min_Stack_Size;
    const uint8_t *max_heap = (uint8_t *)stack_limit;
    uint8_t *prev_heap_end;

    /* Initialize heap end at first call */
    if (unlikely(NULL == heap_end))
    {
        heap_end = &_end;
    }

    // TODO(james): If using an RTOS, wrap this entire block in a mutex / critical section such as
    // vTaskSuspendAll(), __disable_irq(), or similar.

    /* Protect heap from growing into the reserved stack space */
    if (unlikely(incr > 0 && (heap_end + incr > max_heap)))
    {
        errno = ENOMEM;
        return (void *)-1;
    }

    /* Protect against shrinking below the base of the heap */
    if (unlikely(incr < 0 && (heap_end + incr < &_end)))
    {
        errno = EINVAL;
        return (void *)-1;
    }

    prev_heap_end = heap_end;
    heap_end += incr;

    return (void *)prev_heap_end;
}
