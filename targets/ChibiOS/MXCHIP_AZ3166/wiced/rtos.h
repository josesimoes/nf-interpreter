//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

// ChibiOS RTOS abstraction for WICED high-level API headers.
// Provides the types required by include/wiced_rtos.h.
// This is the ChibiOS equivalent of WICED/RTOS/FreeRTOS/WICED/rtos.h.

#pragma once

#include "ch.h"
#include "wiced_result.h"
#include "wwd_rtos.h"

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------
// Macros
// --------------------------------------------------------------------------

#define WICED_HARDWARE_IO_WORKER_THREAD     ((wiced_worker_thread_t *)&wiced_hardware_io_worker_thread)
#define WICED_NETWORKING_WORKER_THREAD      ((wiced_worker_thread_t *)&wiced_networking_worker_thread)

#define WICED_PRIORITY_TO_NATIVE_PRIORITY(priority) \
    ((tprio_t)(RTOS_HIGHEST_PRIORITY - (priority)))

#define WICED_END_OF_THREAD(thread)                  chThdExit(MSG_OK)
#define WICED_END_OF_CURRENT_THREAD()                chThdExit(MSG_OK)
#define WICED_END_OF_CURRENT_THREAD_NO_LEAK_CHECK()  chThdExit(MSG_OK)

#define WICED_GET_THREAD_HANDLE(thread) (thread)
#define WICED_GET_QUEUE_HANDLE(queue)   (queue)

// --------------------------------------------------------------------------
// Constants — built-in worker threads
// --------------------------------------------------------------------------

#ifndef HARDWARE_IO_WORKER_THREAD_STACK_SIZE
#define HARDWARE_IO_WORKER_THREAD_STACK_SIZE  (512)
#endif
#define HARDWARE_IO_WORKER_THREAD_QUEUE_SIZE  (10)
#define HARDWARE_IO_WORKER_THREAD_PRIORITY    \
    (WICED_PRIORITY_TO_NATIVE_PRIORITY(WICED_DEFAULT_LIBRARY_PRIORITY))

#ifndef NETWORKING_WORKER_THREAD_STACK_SIZE
#define NETWORKING_WORKER_THREAD_STACK_SIZE   (7 * 1024)
#endif
#define NETWORKING_WORKER_THREAD_QUEUE_SIZE   (15)
#define NETWORKING_WORKER_THREAD_PRIORITY     \
    (WICED_PRIORITY_TO_NATIVE_PRIORITY(WICED_NETWORK_WORKER_PRIORITY))

#define RTOS_NAME    "ChibiOS"
#define RTOS_VERSION CH_VERSION

// --------------------------------------------------------------------------
// Type definitions — ChibiOS equivalents
// --------------------------------------------------------------------------

typedef event_source_t          wiced_event_flags_t;
typedef host_semaphore_type_t   wiced_semaphore_t;
typedef mutex_t                 wiced_mutex_t;

typedef void (*timer_handler_t)(void *arg);

typedef struct
{
    virtual_timer_t handle;
    timer_handler_t function;
    void           *arg;
} wiced_timer_t;

typedef thread_t *wiced_thread_t;

typedef mailbox_t wiced_queue_t;

typedef struct
{
    wiced_thread_t thread;
    wiced_queue_t  event_queue;
} wiced_worker_thread_t;

typedef wiced_result_t (*event_handler_t)(void *arg);

typedef struct
{
    event_handler_t        function;
    void                  *arg;
    wiced_timer_t          timer;
    wiced_worker_thread_t *thread;
} wiced_timed_event_t;

// --------------------------------------------------------------------------
// Global worker thread instances (defined elsewhere)
// --------------------------------------------------------------------------

extern wiced_worker_thread_t wiced_hardware_io_worker_thread;
extern wiced_worker_thread_t wiced_networking_worker_thread;

#ifdef __cplusplus
} // extern "C"
#endif
