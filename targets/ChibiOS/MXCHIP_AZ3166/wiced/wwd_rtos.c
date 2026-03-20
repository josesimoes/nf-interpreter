//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

// ChibiOS/RT RTOS abstraction implementation for WICED WWD.
// Maps the host_rtos_* API required by WICED onto ChibiOS/RT primitives.

#include "wwd_rtos.h"
#include "wwd_constants.h"
#include "wwd_assert.h"

#include "ch.h"

#include <string.h>

// --------------------------------------------------------------------------
// Helper: thread trampoline
// --------------------------------------------------------------------------
// ChibiOS thread entry signature is THD_FUNCTION(name, arg) → void name(void *arg).
// WICED expects void entry(uint32_t arg).

typedef struct
{
    void (*entry)(uint32_t);
    uint32_t arg;
} wwd_thread_args_t;

static THD_FUNCTION(wwd_thread_trampoline, arg)
{
    wwd_thread_args_t *ta = (wwd_thread_args_t *)arg;
    void (*entry)(uint32_t) = ta->entry;
    uint32_t thread_arg = ta->arg;

    // Free the argument block — it was allocated from the heap.
    chHeapFree(ta);

    entry(thread_arg);
}

// --------------------------------------------------------------------------
// Thread management
// --------------------------------------------------------------------------

wwd_result_t host_rtos_create_thread(
    host_thread_type_t *thread,
    void (*entry_function)(uint32_t arg),
    const char *name,
    void *stack,
    uint32_t stack_size,
    uint32_t priority)
{
    return host_rtos_create_thread_with_arg(thread, entry_function, name, stack, stack_size, priority, 0);
}

wwd_result_t host_rtos_create_thread_with_arg(
    host_thread_type_t *thread,
    void (*entry_function)(uint32_t arg),
    const char *name,
    void *stack,
    uint32_t stack_size,
    uint32_t priority,
    uint32_t arg)
{
    // ChibiOS allocates from heap — caller-supplied stack ignored
    (void)stack;
    // ChibiOS doesn't store thread names by default
    (void)name;

    if (thread == NULL || entry_function == NULL)
    {
        return WWD_BADARG;
    }

    // Allocate a small struct to pass the real entry + arg through the trampoline.
    wwd_thread_args_t *ta = (wwd_thread_args_t *)chHeapAlloc(NULL, sizeof(wwd_thread_args_t));
    if (ta == NULL)
    {
        return WWD_MALLOC_FAILURE;
    }

    ta->entry = entry_function;
    ta->arg = arg;

    *thread = chThdCreateFromHeap(
        NULL,                          // default heap
        THD_WORKING_AREA_SIZE(stack_size),
        name,
        (tprio_t)priority,
        wwd_thread_trampoline,
        ta);

    if (*thread == NULL)
    {
        chHeapFree(ta);
        return WWD_THREAD_CREATE_FAILED;
    }

    return WWD_SUCCESS;
}

wwd_result_t host_rtos_finish_thread(host_thread_type_t *thread)
{
    (void)thread;
    // ChibiOS threads terminate when they return from entry. Nothing to do.
    chThdExit(MSG_OK);

    // Never reached.
    return WWD_SUCCESS;
}

wwd_result_t host_rtos_join_thread(host_thread_type_t *thread)
{
    if (thread == NULL || *thread == NULL)
    {
        return WWD_BADARG;
    }

    chThdWait(*thread);
    *thread = NULL;

    return WWD_SUCCESS;
}

wwd_result_t host_rtos_delete_terminated_thread(host_thread_type_t *thread)
{
    if (thread == NULL || *thread == NULL)
    {
        return WWD_BADARG;
    }

    // Thread memory was allocated from heap by chThdCreateFromHeap.
    // After chThdWait() returns (via join), ChibiOS has already freed the
    // working area.  Nothing else to do.
    *thread = NULL;

    return WWD_SUCCESS;
}

// --------------------------------------------------------------------------
// Semaphore management
// --------------------------------------------------------------------------

wwd_result_t host_rtos_init_semaphore(host_semaphore_type_t *semaphore)
{
    if (semaphore == NULL)
    {
        return WWD_BADARG;
    }

    chSemObjectInit(semaphore, 0);

    return WWD_SUCCESS;
}

wwd_result_t host_rtos_get_semaphore(
    host_semaphore_type_t *semaphore,
    uint32_t timeout_ms,
    wiced_bool_t will_set_in_isr)
{
    (void)will_set_in_isr;

    if (semaphore == NULL)
    {
        return WWD_BADARG;
    }

    sysinterval_t ticks = RTOS_MS_TO_TICKS(timeout_ms);

    msg_t result = chSemWaitTimeout(semaphore, ticks);

    if (result == MSG_OK)
    {
        return WWD_SUCCESS;
    }

    if (result == MSG_TIMEOUT)
    {
        return WWD_TIMEOUT;
    }

    return WWD_SEMAPHORE_ERROR;
}

wwd_result_t host_rtos_set_semaphore(
    host_semaphore_type_t *semaphore,
    wiced_bool_t called_from_isr)
{
    if (semaphore == NULL)
    {
        return WWD_BADARG;
    }

    if (called_from_isr == WICED_TRUE)
    {
        chSysLockFromISR();
        chSemSignalI(semaphore);
        chSysUnlockFromISR();
    }
    else
    {
        chSemSignal(semaphore);
    }

    return WWD_SUCCESS;
}

wwd_result_t host_rtos_deinit_semaphore(host_semaphore_type_t *semaphore)
{
    if (semaphore == NULL)
    {
        return WWD_BADARG;
    }

    chSemReset(semaphore, 0);

    return WWD_SUCCESS;
}

// --------------------------------------------------------------------------
// Mutex management
// --------------------------------------------------------------------------

wwd_result_t host_rtos_init_mutex(host_mutex_type_t *mutex)
{
    if (mutex == NULL)
    {
        return WWD_BADARG;
    }

    chMtxObjectInit(mutex);

    return WWD_SUCCESS;
}

wwd_result_t host_rtos_lock_mutex(host_mutex_type_t *mutex)
{
    if (mutex == NULL)
    {
        return WWD_BADARG;
    }

    chMtxLock(mutex);

    return WWD_SUCCESS;
}

wwd_result_t host_rtos_unlock_mutex(host_mutex_type_t *mutex)
{
    if (mutex == NULL)
    {
        return WWD_BADARG;
    }

    chMtxUnlock(mutex);

    return WWD_SUCCESS;
}

wwd_result_t host_rtos_deinit_mutex(host_mutex_type_t *mutex)
{
    (void)mutex;
    // ChibiOS mutexes do not need deinitialization.
    return WWD_SUCCESS;
}

// --------------------------------------------------------------------------
// Queue (mailbox) management
// --------------------------------------------------------------------------

wwd_result_t host_rtos_init_queue(
    host_queue_type_t *queue,
    void *buffer,
    uint32_t buffer_size,
    uint32_t message_size)
{
    (void)message_size; // ChibiOS mailbox always uses msg_t-sized slots

    if (queue == NULL)
    {
        return WWD_BADARG;
    }

    // Number of msg_t slots in the provided buffer
    uint32_t num_messages = buffer_size / sizeof(msg_t);

    if (buffer == NULL || num_messages == 0)
    {
        return WWD_BADARG;
    }

    queue->buffer = (msg_t *)buffer;
    chMBObjectInit(&queue->handle, queue->buffer, num_messages);

    return WWD_SUCCESS;
}

wwd_result_t host_rtos_push_to_queue(
    host_queue_type_t *queue,
    void *message,
    uint32_t timeout_ms)
{
    if (queue == NULL || message == NULL)
    {
        return WWD_BADARG;
    }

    sysinterval_t ticks = RTOS_MS_TO_TICKS(timeout_ms);
    msg_t value;
    memcpy(&value, message, sizeof(msg_t));

    msg_t result = chMBPostTimeout(&queue->handle, value, ticks);

    if (result == MSG_OK)
    {
        return WWD_SUCCESS;
    }

    if (result == MSG_TIMEOUT)
    {
        return WWD_TIMEOUT;
    }

    return WWD_QUEUE_ERROR;
}

wwd_result_t host_rtos_pop_from_queue(
    host_queue_type_t *queue,
    void *message,
    uint32_t timeout_ms)
{
    if (queue == NULL || message == NULL)
    {
        return WWD_BADARG;
    }

    sysinterval_t ticks = RTOS_MS_TO_TICKS(timeout_ms);
    msg_t value;

    msg_t result = chMBFetchTimeout(&queue->handle, &value, ticks);

    if (result == MSG_OK)
    {
        memcpy(message, &value, sizeof(msg_t));
        return WWD_SUCCESS;
    }

    if (result == MSG_TIMEOUT)
    {
        return WWD_TIMEOUT;
    }

    return WWD_QUEUE_ERROR;
}

wwd_result_t host_rtos_deinit_queue(host_queue_type_t *queue)
{
    if (queue == NULL)
    {
        return WWD_BADARG;
    }

    chMBReset(&queue->handle);
    queue->buffer = NULL;

    return WWD_SUCCESS;
}

// --------------------------------------------------------------------------
// Time / delay
// --------------------------------------------------------------------------

wwd_result_t host_rtos_delay_milliseconds(uint32_t num_ms)
{
    if (num_ms == 0)
    {
        chThdYield();
    }
    else
    {
        chThdSleepMilliseconds(num_ms);
    }

    return WWD_SUCCESS;
}

uint32_t host_rtos_get_time(void)
{
    return (uint32_t)TIME_I2MS(chVTGetSystemTime());
}
