#pragma once

#define VECTOR_TABLE_SECTION ".isr_vector"
#define SYSTEM_STATUS_SECTION ".status"
#define SYSTEM_STATUS_WILDCARD_SECTION ".status.*"
#define SYSTICK_VALUE_SECTION ".status.time"
#define CLOCK_SPEED_VALUE_SECTION ".status.speed"
#define POWER_STATUS_SECTION ".status.power"
#define FAULT_SECTION ".fault"
#define PRINCIPAL_RESULT_SECTION ".status.value"
#define DEBUG_VAR_SECTION ".status.debug"

#define BUILD_TIME_SECTION ".build_time"
#define BUILD_TIME_ADDR_SECTION ".build_time.addr"
#define DMA_BUFFER_SECTION ".dma_buffers"

#define STRINGIFY_X(s) STRINGIFY(s)
#define STRINGIFY(s) #s

#define IN_SECTION(sec) __attribute__((section(STRINGIFY_X(sec))))
#define KEEP_IN_SECTION(sec) \
  [[maybe_unused]] __attribute__((section(STRINGIFY_X(sec)))) __attribute__((used))

#define INTERRUPT_VECTOR_TABLE KEEP_IN_SECTION(VECTOR_TABLE_SECTION)
#define FAULT_BUFFER KEEP_IN_SECTION(FAULT_SECTION)

#define PRINCIPAL_RESULT KEEP_IN_SECTION(PRINCIPAL_RESULT_SECTION)
#define DEBUG_VAR KEEP_IN_SECTION(DEBUG_VAR_SECTION)
