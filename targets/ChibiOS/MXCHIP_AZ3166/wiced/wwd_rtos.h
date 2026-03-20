//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

// ChibiOS/RT RTOS abstraction layer for WICED WWD
// Modeled on the FreeRTOS port in WICED SDK 6.6.1

#ifndef INCLUDED_WWD_RTOS_H_
#define INCLUDED_WWD_RTOS_H_

#include "ch.h"
#include "wwd_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------
// RTOS type mappings — ChibiOS equivalents of FreeRTOS primitives
// --------------------------------------------------------------------------

// Thread handle
typedef thread_t *host_thread_type_t;

// Binary/counting semaphore
typedef semaphore_t host_semaphore_type_t;

// Mutex
typedef mutex_t host_mutex_type_t;

// Message queue (ChibiOS mailbox)
typedef struct
{
    mailbox_t handle;
    msg_t *buffer;
} host_queue_type_t;

// --------------------------------------------------------------------------
// Thread priority mapping
// --------------------------------------------------------------------------

// ChibiOS thread priorities — lower number = lower priority (same as FreeRTOS).
// WICED defines priority offsets relative to the "worker" thread.
// Map them so the WWD thread runs above normal but below real-time.
#define RTOS_HIGHEST_PRIORITY       (NORMALPRIO + 7)
#define RTOS_DEFAULT_THREAD_PRIORITY (NORMALPRIO)
#define RTOS_LOWEST_PRIORITY        (NORMALPRIO - 7)

// Priority manipulation macros required by wwd_rtos_interface.h
// Note: typo in macro names is intentional — matches WICED SDK spelling
#define RTOS_HIGHER_PRIORTIY_THAN(x) ((x) + 1)
#define RTOS_LOWER_PRIORTIY_THAN(x)  ((x) - 1)

// ChibiOS allocates thread stacks dynamically from heap
#define RTOS_USE_DYNAMIC_THREAD_STACK

// Stack size for the WWD thread (bytes — ChibiOS uses bytes, not words)
#define RTOS_DEFAULT_STACK_SIZE     (4096)

// --------------------------------------------------------------------------
// Thread configuration (used by host_rtos_create_configed_thread)
// --------------------------------------------------------------------------
typedef struct
{
    uint32_t dummy; // placeholder — ChibiOS does not need extra config
} host_rtos_thread_config_type_t;

// --------------------------------------------------------------------------
// Timeout values
// --------------------------------------------------------------------------

// NEVER_TIMEOUT is defined by wwd_rtos_interface.h — do not define here

// Convert ms to ChibiOS ticks. 0xFFFFFFFF maps to TIME_INFINITE.
#define RTOS_MS_TO_TICKS(ms) (((ms) == 0xFFFFFFFFUL) ? TIME_INFINITE : TIME_MS2I(ms))

// --------------------------------------------------------------------------
// RTOS API — called by WICED WWD core
// --------------------------------------------------------------------------

// Thread management
extern wwd_result_t host_rtos_create_thread(
    host_thread_type_t *thread,
    void (*entry_function)(uint32_t arg),
    const char *name,
    void *stack,
    uint32_t stack_size,
    uint32_t priority);

extern wwd_result_t host_rtos_create_thread_with_arg(
    host_thread_type_t *thread,
    void (*entry_function)(uint32_t arg),
    const char *name,
    void *stack,
    uint32_t stack_size,
    uint32_t priority,
    uint32_t arg);

extern wwd_result_t host_rtos_finish_thread(host_thread_type_t *thread);
extern wwd_result_t host_rtos_join_thread(host_thread_type_t *thread);
extern wwd_result_t host_rtos_delete_terminated_thread(host_thread_type_t *thread);

// Semaphore management
extern wwd_result_t host_rtos_init_semaphore(host_semaphore_type_t *semaphore);
extern wwd_result_t host_rtos_get_semaphore(
    host_semaphore_type_t *semaphore,
    uint32_t timeout_ms,
    wiced_bool_t will_set_in_isr);
extern wwd_result_t host_rtos_set_semaphore(
    host_semaphore_type_t *semaphore,
    wiced_bool_t called_from_isr);
extern wwd_result_t host_rtos_deinit_semaphore(host_semaphore_type_t *semaphore);

// Mutex management
extern wwd_result_t host_rtos_init_mutex(host_mutex_type_t *mutex);
extern wwd_result_t host_rtos_lock_mutex(host_mutex_type_t *mutex);
extern wwd_result_t host_rtos_unlock_mutex(host_mutex_type_t *mutex);
extern wwd_result_t host_rtos_deinit_mutex(host_mutex_type_t *mutex);

// Queue (mailbox) management
extern wwd_result_t host_rtos_init_queue(
    host_queue_type_t *queue,
    void *buffer,
    uint32_t buffer_size,
    uint32_t message_size);
extern wwd_result_t host_rtos_push_to_queue(
    host_queue_type_t *queue,
    void *message,
    uint32_t timeout_ms);
extern wwd_result_t host_rtos_pop_from_queue(
    host_queue_type_t *queue,
    void *message,
    uint32_t timeout_ms);
extern wwd_result_t host_rtos_deinit_queue(host_queue_type_t *queue);

// Time / delay
extern wwd_result_t host_rtos_delay_milliseconds(uint32_t num_ms);
extern uint32_t host_rtos_get_time(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifndef INCLUDED_WWD_RTOS_H_
