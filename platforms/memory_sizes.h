#pragma once

/**
 * Header defining FLASH_MEM_SIZE, SRAM1_MEM_SIZE, and
 * SRAM2_MEM_SIZE for non-L4+ models in the STM32L4 series.
 *
 * Designed to rely purely on family symbols (e.g. STM32L452xx) and full part
 * numbers (e.g. STM32L452RCT6 or STM32L452RC), without requiring density-level
 * macros (e.g. STM32L452xC).
 */

/* SRAM size determination. */

/* --- STM32L412 / STM32L422 (40 KB Total RAM: 32 KB SRAM1 + 8 KB SRAM2) --- */
#if defined(STM32L412xx) || defined(STM32L422xx) || defined(STM32L412K8) || \
    defined(STM32L412KB) || defined(STM32L412C8) || defined(STM32L412CB) || \
    defined(STM32L412R8) || defined(STM32L412RB) || defined(STM32L412T8) || \
    defined(STM32L412TB) || defined(STM32L422K8) || defined(STM32L422KB) || \
    defined(STM32L422C8) || defined(STM32L422CB) || defined(STM32L422R8) || \
    defined(STM32L422RB) || defined(STM32L412KBU6)
#define SRAM1_MEM_SIZE (32 * 1024)
#define SRAM2_MEM_SIZE (8 * 1024)

/* --- STM32L431 / L432 / L433 / L442 / L443 (64 KB Total RAM: 48 KB SRAM1 + 16 KB SRAM2) --- */
#elif defined(STM32L431xx) || defined(STM32L432xx) || defined(STM32L433xx) || \
    defined(STM32L442xx) || defined(STM32L443xx) || defined(STM32L431KB) ||   \
    defined(STM32L431KC) || defined(STM32L431CB) || defined(STM32L431CC) ||   \
    defined(STM32L431RB) || defined(STM32L431RC) || defined(STM32L431RE) ||   \
    defined(STM32L431VC) || defined(STM32L432KB) || defined(STM32L432KC) ||   \
    defined(STM32L433CB) || defined(STM32L433CC) || defined(STM32L433RB) ||   \
    defined(STM32L433RC) || defined(STM32L433RE) || defined(STM32L433VC) ||   \
    defined(STM32L433VE) || defined(STM32L442KC) || defined(STM32L443CC) ||   \
    defined(STM32L443RC) || defined(STM32L432KCU6)
#define SRAM1_MEM_SIZE (48 * 1024)
#define SRAM2_MEM_SIZE (16 * 1024)

/* --- STM32L451 / L452 / L462 (160 KB Total RAM: 128 KB SRAM1 + 32 KB SRAM2) --- */
#elif defined(STM32L451xx) || defined(STM32L452xx) || defined(STM32L462xx) ||     \
    defined(STM32L451CC) || defined(STM32L451CE) || defined(STM32L451RC) ||       \
    defined(STM32L451RE) || defined(STM32L451VC) || defined(STM32L451VE) ||       \
    defined(STM32L452CC) || defined(STM32L452CE) || defined(STM32L452RC) ||       \
    defined(STM32L452RE) || defined(STM32L452VC) || defined(STM32L452VE) ||       \
    defined(STM32L452CCT6) || defined(STM32L452CET6) || defined(STM32L452RCT6) || \
    defined(STM32L452RET6) || defined(STM32L452RCI6) || defined(STM32L452REI6) || \
    defined(STM32L452VCT6) || defined(STM32L452VET6) || defined(STM32L462CC) ||   \
    defined(STM32L462CE) || defined(STM32L462RC) || defined(STM32L462RE)
#define SRAM1_MEM_SIZE (128 * 1024)
#define SRAM2_MEM_SIZE (32 * 1024)

/* --- STM32L471 / L475 / L476 / L485 / L486 (128 KB Total RAM: 96 KB SRAM1 + 32 KB SRAM2) --- */
#elif defined(STM32L471xx) || defined(STM32L475xx) || defined(STM32L476xx) ||   \
    defined(STM32L485xx) || defined(STM32L486xx) || defined(STM32L471RC) ||     \
    defined(STM32L471RE) || defined(STM32L471RG) || defined(STM32L471VC) ||     \
    defined(STM32L471VE) || defined(STM32L471VG) || defined(STM32L475RC) ||     \
    defined(STM32L475RE) || defined(STM32L475RG) || defined(STM32L475VC) ||     \
    defined(STM32L475VE) || defined(STM32L475VG) || defined(STM32L476RC) ||     \
    defined(STM32L476RE) || defined(STM32L476RG) || defined(STM32L476VC) ||     \
    defined(STM32L476VE) || defined(STM32L476VG) || defined(STM32L476ZC) ||     \
    defined(STM32L476ZE) || defined(STM32L476ZG) || defined(STM32L476RCT6) ||   \
    defined(STM32L476RET6) || defined(STM32L476RGT6) || defined(STM32L485JE) || \
    defined(STM32L485JG) || defined(STM32L485RE) || defined(STM32L485RG) ||     \
    defined(STM32L486RC) || defined(STM32L486RE) || defined(STM32L486RG) ||     \
    defined(STM32L486VC) || defined(STM32L486VE) || defined(STM32L486VG) ||     \
    defined(STM32L486ZC) || defined(STM32L486ZE) || defined(STM32L486ZG)
#define SRAM1_MEM_SIZE (96 * 1024)
#define SRAM2_MEM_SIZE (32 * 1024)

/* --- STM32L496 / STM32L4A6 (320 KB Total RAM: 256 KB SRAM1 + 64 KB SRAM2) --- */
#elif defined(STM32L496xx) || defined(STM32L4A6xx) || defined(STM32L496RE) ||   \
    defined(STM32L496RG) || defined(STM32L496VE) || defined(STM32L496VG) ||     \
    defined(STM32L496ZE) || defined(STM32L496ZG) || defined(STM32L496AE) ||     \
    defined(STM32L496AG) || defined(STM32L496RET6) || defined(STM32L496RGT6) || \
    defined(STM32L4A6RE) || defined(STM32L4A6RG) || defined(STM32L4A6VE) ||     \
    defined(STM32L4A6VG) || defined(STM32L4A6ZE) || defined(STM32L4A6ZG) ||     \
    defined(STM32L4A6AE) || defined(STM32L4A6AG)
#define SRAM1_MEM_SIZE (256 * 1024)
#define SRAM2_MEM_SIZE (64 * 1024)
#endif

/* Flash size determination. */

/* --- 64 KB Flash ('8' Density Code) --- */
#if defined(STM32L412K8) || defined(STM32L412K8T6) || defined(STM32L412K8U6) || \
    defined(STM32L412C8) || defined(STM32L412C8T6) || defined(STM32L412C8U6) || \
    defined(STM32L412R8) || defined(STM32L412R8T6) || defined(STM32L412T8) ||   \
    defined(STM32L412T8U6) || defined(STM32L422K8) || defined(STM32L422K8U6) || \
    defined(STM32L422C8) || defined(STM32L422C8T6) || defined(STM32L422R8) ||   \
    defined(STM32L422R8T6)
#define FLASH_MEM_SIZE (64 * 1024)

/* --- 128 KB Flash ('B' Density Code) --- */
#elif defined(STM32L412KB) || defined(STM32L412KBT6) || defined(STM32L412KBU6) || \
    defined(STM32L412CB) || defined(STM32L412CBT6) || defined(STM32L412CBU6) ||   \
    defined(STM32L412RB) || defined(STM32L412RBT6) || defined(STM32L412TB) ||     \
    defined(STM32L412TBU6) || defined(STM32L422KB) || defined(STM32L422KBU6) ||   \
    defined(STM32L422CB) || defined(STM32L422CBT6) || defined(STM32L422RB) ||     \
    defined(STM32L422RBT6) || defined(STM32L431KB) || defined(STM32L431KBU6) ||   \
    defined(STM32L431CB) || defined(STM32L431CBT6) || defined(STM32L431RB) ||     \
    defined(STM32L431RBT6) || defined(STM32L432KB) || defined(STM32L432KBU6) ||   \
    defined(STM32L433CB) || defined(STM32L433CBT6) || defined(STM32L433RB) ||     \
    defined(STM32L433RBT6) || defined(STM32L442KB) || defined(STM32L442KBU6) ||   \
    defined(STM32L443CB) || defined(STM32L443CBT6) || defined(STM32L443RB) ||     \
    defined(STM32L443RBT6)
#define FLASH_MEM_SIZE (128 * 1024)

/* --- 256 KB Flash ('C' Density Code) --- */
#elif defined(STM32L431KC) || defined(STM32L431KCU6) || defined(STM32L431CC) || \
    defined(STM32L431CCT6) || defined(STM32L431RC) || defined(STM32L431RCT6) || \
    defined(STM32L431VC) || defined(STM32L431VCT6) || defined(STM32L432KC) ||   \
    defined(STM32L432KCU6) || defined(STM32L433CC) || defined(STM32L433CCT6) || \
    defined(STM32L433RC) || defined(STM32L433RCT6) || defined(STM32L433VC) ||   \
    defined(STM32L433VCT6) || defined(STM32L443CC) || defined(STM32L443CCT6) || \
    defined(STM32L443RC) || defined(STM32L443RCT6) || defined(STM32L451CC) ||   \
    defined(STM32L451CCT6) || defined(STM32L451RC) || defined(STM32L451RCT6) || \
    defined(STM32L451VC) || defined(STM32L451VCT6) || defined(STM32L452CC) ||   \
    defined(STM32L452CCT6) || defined(STM32L452RC) || defined(STM32L452RCT6) || \
    defined(STM32L452RCI6) || defined(STM32L452VC) || defined(STM32L452VCT6) || \
    defined(STM32L462CC) || defined(STM32L462CCT6) || defined(STM32L462RC) ||   \
    defined(STM32L462RCT6) || defined(STM32L471RC) || defined(STM32L471RCT6) || \
    defined(STM32L471VC) || defined(STM32L471VCT6) || defined(STM32L475RC) ||   \
    defined(STM32L475RCT6) || defined(STM32L475VC) || defined(STM32L475VCT6) || \
    defined(STM32L476RC) || defined(STM32L476RCT6) || defined(STM32L476VC) ||   \
    defined(STM32L476VCT6) || defined(STM32L476ZC) || defined(STM32L476ZCT6) || \
    defined(STM32L486RC) || defined(STM32L486RCT6) || defined(STM32L486VC) ||   \
    defined(STM32L486VCT6) || defined(STM32L486ZC) || defined(STM32L486ZCT6)
#define FLASH_MEM_SIZE (256 * 1024)

/* --- 512 KB Flash ('E' Density Code) --- */
#elif defined(STM32L431RE) || defined(STM32L431RET6) || defined(STM32L433RE) || \
    defined(STM32L433RET6) || defined(STM32L433VE) || defined(STM32L433VET6) || \
    defined(STM32L451CE) || defined(STM32L451CET6) || defined(STM32L451RE) ||   \
    defined(STM32L451RET6) || defined(STM32L451VE) || defined(STM32L451VET6) || \
    defined(STM32L452CE) || defined(STM32L452CET6) || defined(STM32L452RE) ||   \
    defined(STM32L452RET6) || defined(STM32L452REI6) || defined(STM32L452VE) || \
    defined(STM32L452VET6) || defined(STM32L462CE) || defined(STM32L462CET6) || \
    defined(STM32L462RE) || defined(STM32L462RET6) || defined(STM32L471RE) ||   \
    defined(STM32L471RET6) || defined(STM32L471VE) || defined(STM32L471VET6) || \
    defined(STM32L475RE) || defined(STM32L475RET6) || defined(STM32L475VE) ||   \
    defined(STM32L475VET6) || defined(STM32L475ME) || defined(STM32L475MEY6) || \
    defined(STM32L476RE) || defined(STM32L476RET6) || defined(STM32L476VE) ||   \
    defined(STM32L476VET6) || defined(STM32L476ZE) || defined(STM32L476ZET6) || \
    defined(STM32L476ME) || defined(STM32L476MEY6) || defined(STM32L476JE) ||   \
    defined(STM32L476JEY6) || defined(STM32L485JE) || defined(STM32L485JEY6) || \
    defined(STM32L485RE) || defined(STM32L485RET6) || defined(STM32L485VE) ||   \
    defined(STM32L485VET6) || defined(STM32L486RE) || defined(STM32L486RET6) || \
    defined(STM32L486VE) || defined(STM32L486VET6) || defined(STM32L486ZE) ||   \
    defined(STM32L486ZET6) || defined(STM32L496RE) || defined(STM32L496RET6) || \
    defined(STM32L496VE) || defined(STM32L496VET6) || defined(STM32L496ZE) ||   \
    defined(STM32L496ZET6) || defined(STM32L496AE) || defined(STM32L496AGI6) || \
    defined(STM32L4A6RE) || defined(STM32L4A6RET6) || defined(STM32L4A6VE) ||   \
    defined(STM32L4A6VET6) || defined(STM32L4A6ZE) || defined(STM32L4A6ZET6) || \
    defined(STM32L4A6AE) || defined(STM32L4A6AGI6)
#define FLASH_MEM_SIZE (512 * 1024)

/* --- 1 MB Flash ('G' Density Code) --- */
#elif defined(STM32L471RG) || defined(STM32L471RGT6) || defined(STM32L471VG) || \
    defined(STM32L471VGT6) || defined(STM32L475RG) || defined(STM32L475RGT6) || \
    defined(STM32L475VG) || defined(STM32L475VGT6) || defined(STM32L475MG) ||   \
    defined(STM32L475MGY6) || defined(STM32L476RG) || defined(STM32L476RGT6) || \
    defined(STM32L476VG) || defined(STM32L476VGT6) || defined(STM32L476ZG) ||   \
    defined(STM32L476ZGT6) || defined(STM32L476MG) || defined(STM32L476MGY6) || \
    defined(STM32L476JG) || defined(STM32L476JGY6) || defined(STM32L485JG) ||   \
    defined(STM32L485JGY6) || defined(STM32L485RG) || defined(STM32L485RGT6) || \
    defined(STM32L485VG) || defined(STM32L485VGT6) || defined(STM32L486RG) ||   \
    defined(STM32L486RGT6) || defined(STM32L486VG) || defined(STM32L486VGT6) || \
    defined(STM32L486ZG) || defined(STM32L486ZGT6) || defined(STM32L496RG) ||   \
    defined(STM32L496RGT6) || defined(STM32L496VG) || defined(STM32L496VGT6) || \
    defined(STM32L496ZG) || defined(STM32L496ZGT6) || defined(STM32L496AG) ||   \
    defined(STM32L496AGI6) || defined(STM32L4A6RG) || defined(STM32L4A6RGT6) || \
    defined(STM32L4A6VG) || defined(STM32L4A6VGT6) || defined(STM32L4A6ZG) ||   \
    defined(STM32L4A6ZGT6) || defined(STM32L4A6AG) || defined(STM32L4A6AGI6)
#define FLASH_MEM_SIZE (1024 * 1024)
#endif

#ifndef SRAM1_MEM_SIZE
#error \
    "Unable to determine SRAM sizes. Ensure a valid family macro (e.g., -DSTM32L452xx) or full part number (e.g., -DSTM32L452RCT6) is defined."
#endif

#ifndef FLASH_MEM_SIZE
#error \
    "Unable to determine FLASH_MEM_SIZE. Family macros (e.g., STM32L452xx) do not specify Flash size; ensure a full part number symbol (e.g., -DSTM32L452RCT6 or -DSTM32L452RC) is defined."
#endif
