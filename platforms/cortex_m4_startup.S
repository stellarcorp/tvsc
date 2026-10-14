.syntax unified
.thumb

.global Reset_Handler

.section .status.exit_code, "aw", %nobits
.align 2
.global exit_code
.type exit_code, %object
.size exit_code, 4
exit_code:
    .space 4

.section .text.Reset_Handler, "ax", %progbits
.weak Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
  /* Initialize the stack pointer to the end of the stack as configured in the linker script. */
  ldr sp, =_estack

  /**
   * Zero fill the status segment. Zero filling these locations first allows SystemInit and other
   * initialization routines to report errors there.
   */
  ldr r2, =_sstatus
  ldr r4, =_estatus
  movs r3, #0
  b LoopFillZeroStatus

FillZeroStatus:
  str  r3, [r2]
  adds r2, r2, #4

LoopFillZeroStatus:
  cmp r2, r4
  bcc FillZeroStatus

  /* Call the clock system initialization function.*/
  bl  SystemInit

  /* Copy the data segment initializers from flash to SRAM */
  ldr r0, =_sdata
  ldr r1, =_edata
  ldr r2, =_sidata
  movs r3, #0
  b LoopCopyDataInit

CopyDataInit:
  ldr r4, [r2, r3]
  str r4, [r0, r3]
  adds r3, r3, #4

LoopCopyDataInit:
  adds r4, r0, r3
  cmp r4, r1
  bcc CopyDataInit
  
  /* Zero fill the BSS segment. */
  ldr r2, =_sbss
  ldr r4, =_ebss
  movs r3, #0
  b LoopFillZeroBss

FillZeroBss:
  str  r3, [r2]
  adds r2, r2, #4

LoopFillZeroBss:
  cmp r2, r4
  bcc FillZeroBss

  ldr r0, =0xABCD
  ldr r1, =exit_code
  str r0, [r1]

  /* Call static constructors */
  bl __libc_init_array

  ldr r0, =0xBAAB
  ldr r1, =exit_code
  str r0, [r1]

  /* Call the application's entry point.*/
  bl main
  ldr r1, =exit_code
  str r0, [r1]

ProgramComplete:
  b ProgramComplete
    
.size Reset_Handler, .-Reset_Handler
