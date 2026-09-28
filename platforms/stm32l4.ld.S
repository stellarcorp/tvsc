#include "linker_sections.h"
#include "memory_sizes.h"

ENTRY(Reset_Handler)

/* Highest address of user mode stack. */
_estack = ORIGIN(RAM) + LENGTH(RAM);

_Min_Heap_Size  = 0x200;
_Min_Stack_Size = 0x400;

MEMORY
{
  RAM         (rw)  : ORIGIN = 0x20000000, LENGTH = SRAM1_MEM_SIZE
  FLASH       (rx)  : ORIGIN = 0x08000000, LENGTH = FLASH_MEM_SIZE
  DMA_BUFFERS (rw)  : ORIGIN = 0x20020000, LENGTH = SRAM2_MEM_SIZE
}

__FLASH_SIZE = LENGTH(FLASH);
__RAM_SIZE   = LENGTH(RAM);
__DMA_BUFFERS_SIZE = LENGTH(DMA_BUFFERS);

SECTIONS
{
  /* ISR vector table, including Reset_Handler */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  /* Build timestamp for version detection */
  .build_time :
  {
    . = ALIGN(4);
    KEEP(*(BUILD_TIME_SECTION))
    . = ALIGN(4);
  } >FLASH

  /* Program code */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;
  } >FLASH

  /* Constant data into Flash */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  /* C++ Exception handling tables */
  .ARM.extab :
  {
    . = ALIGN(4);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } >FLASH

  .ARM :
  {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4);
  } >FLASH

  /* Initialization arrays for C++ global constructors */
  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH

  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    /* Sort constructor functions by priority if specified */
    KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .ctors))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH

  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
    KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o) .dtors))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* Status section (uninitialized) at top of RAM */
  .status (NOLOAD) :
  {
    . = ALIGN(4);
    _sstatus = .;
    *(SYSTICK_VALUE_SECTION)
    *(CLOCK_SPEED_VALUE_SECTION)
    *(POWER_STATUS_SECTION)
    *(PRINCIPAL_RESULT_SECTION)
    *(SYSTEM_STATUS_SECTION.*)
    *(.bss.errno)
    *(FAULT_SECTION)
    . = ALIGN(4);
    _estatus = .;
  } >RAM

  /* Used by startup code to initialize .data in RAM */
  _sidata = LOADADDR(.data);

  /* Initialized data section into RAM */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } >RAM AT> FLASH

  /* Total Flash footprint boundaries */
  FIRMWARE_BEGIN = ORIGIN(FLASH);
  FIRMWARE_END   = LOADADDR(.data) + SIZEOF(.data);

  /* Uninitialized data section into RAM */
  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM

  /* Reserve minimum heap and stack space */
  ._user_heap_stack (NOLOAD) :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  /* Dedicated non-cacheable / SRAM2 DMA buffer section */
  .dma_buffers (NOLOAD) :
  {
    . = ALIGN(32);
    PROVIDE(_sdma_buf = .);
    *(DMA_BUFFER_SECTION)
    . = ALIGN(32);
    PROVIDE(_edma_buf = .);
  } >DMA_BUFFERS

  ASSERT((_edma_buf - _sdma_buf) <= SRAM2_MEM_SIZE, "DMA buffers exceed available size.")

  /* Remove all code and data originating from the following libraries. */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  /* Discard unused ARM build attributes if needed */
  .ARM.attributes 0 : { *(.ARM.attributes) }
}
