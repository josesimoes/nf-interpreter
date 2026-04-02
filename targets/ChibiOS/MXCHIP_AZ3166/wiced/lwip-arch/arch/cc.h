/*
 * lwIP compiler/architecture abstraction for ARM Cortex-M4 / GCC.
 *
 * This header shadows WICED/network/LwIP/WWD/FreeRTOS/arch/cc.h.
 * It contains NO FreeRTOS types or includes — pure compiler/ABI definitions.
 * RTOS types (semaphores, mutexes, mailboxes) live in arch/sys_arch.h.
 *
 * Adapted from the WICED SDK FreeRTOS arch/cc.h (Cypress Semiconductor).
 */
#ifndef __CC_H__
#define __CC_H__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int sys_prot_t;

/* ARM Cortex-M is always little-endian */
#ifndef __MACHINE_ENDIAN_H__
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#define U16_F PRIu16
#define S16_F PRId16
#define X16_F PRIx16
#define U32_F PRIu32
#define S32_F PRId32
#define X32_F PRIx32

#define SZT_F "zu"

#ifdef __GNUC__
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x
#endif

#define LWIP_PLATFORM_DIAG(x) { printf x; }

#ifdef WICED_LWIP_DEBUG
#define LWIP_PLATFORM_ASSERT(x) WICED_ASSERTION_FAIL_ACTION()
#else
#define LWIP_PLATFORM_ASSERT(x)
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __CC_H__ */
