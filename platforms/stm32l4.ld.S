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
    KEEP(*(.build_time))
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

  /* C++ constructor initialization arrays */
  .preinit_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .init_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .fini_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
  } >FLASH

  /* Uninitialized status section at top of RAM */
  .status (NOLOAD) :
  {
    . = ALIGN(4);
    _sstatus = .;
    *(.status.time)
    *(.status.speed)
    *(.status.power)
    *(.status*)
    *(.bss.errno)
    *(.status.fault)
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
  .dma_buf (NOLOAD) :
  {
    . = ALIGN(32);
    PROVIDE(_sdma_buf = .);
    *(.dma_buf*)
    *(.dma_buffers*)
    . = ALIGN(32);
    PROVIDE(_edma_buf = .);
  } >DMA_BUFFERS

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
