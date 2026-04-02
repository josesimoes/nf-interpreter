/*
 * lwIP sys_arch types for ChibiOS RTOS.
 *
 * This header shadows WICED/network/LwIP/WWD/FreeRTOS/arch/sys_arch.h,
 * replacing FreeRTOS types (xSemaphoreHandle, xQueueHandle, xTaskHandle)
 * with their ChibiOS equivalents (semaphore_t, mutex_t, mailbox_t, thread_t).
 * No FreeRTOS header is included anywhere in this build.
 */
#ifndef INCLUDED_SYS_ARCH_H
#define INCLUDED_SYS_ARCH_H

#include "ch.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Null handles */
#define SYS_MBOX_NULL   ((mailbox_t *)0)
#define SYS_SEM_NULL    ((semaphore_t *)0)
#define SYS_MUTEX_NULL  ((mutex_t *)0)

/* RTOS abstraction types */
typedef semaphore_t * sys_sem_t;
typedef mutex_t     * sys_mutex_t;
typedef mailbox_t   * sys_mbox_t;
typedef thread_t    * sys_thread_t;

uint16_t sys_rand16(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* INCLUDED_SYS_ARCH_H */
