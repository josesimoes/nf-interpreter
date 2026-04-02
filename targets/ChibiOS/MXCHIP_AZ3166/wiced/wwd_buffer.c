/*
 * Port replacement for WICED SDK's wwd_buffer.c
 *
 * The SDK version uses WICED-custom lwIP pool types (PBUF_POOL_TX, PBUF_POOL_RX,
 * MEMP_PBUF_POOL_TX, MEMP_PBUF_POOL_RX) which don't exist in standard lwIP 2.1.x.
 * This implementation uses the standard PBUF_POOL for both TX and RX.
 */

#include "wwd_buffer.h"
#include "wwd_buffer_interface.h"
#include "wwd_constants.h"
#include "wwd_rtos_interface.h"
#include "platform/wwd_bus_interface.h"

#include "lwip/pbuf.h"
#include "lwip/memp.h"

#include <string.h>

/* The SDK version has a FIFO for buffered packets; we keep a simple implementation */

wwd_result_t wwd_buffer_init(void *native_arg)
{
    (void)native_arg;
    return WWD_SUCCESS;
}

wwd_result_t wwd_buffer_deinit(void)
{
    return WWD_SUCCESS;
}

wwd_result_t host_buffer_check_leaked(void)
{
    /* No leak tracking in this minimal port */
    return WWD_SUCCESS;
}

wwd_result_t host_buffer_add_application_defined_pool(void *pool_in, wwd_buffer_dir_t direction)
{
    (void)pool_in;
    (void)direction;
    return WWD_SUCCESS;
}

wwd_result_t internal_host_buffer_get(wiced_buffer_t *buffer, wwd_buffer_dir_t direction,
                                      unsigned short size, unsigned long timeout_ms)
{
    struct pbuf *p;
    unsigned long elapsed = 0;

    (void)direction;

    do
    {
        p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
        if (p != NULL)
        {
            *buffer = p;
            return WWD_SUCCESS;
        }

        if (timeout_ms == 0)
        {
            break;
        }

        host_rtos_delay_milliseconds(1);
        if (timeout_ms != WICED_NEVER_TIMEOUT)
        {
            elapsed++;
            if (elapsed >= timeout_ms)
            {
                break;
            }
        }
    } while (1);

    *buffer = NULL;
    return WWD_BUFFER_UNAVAILABLE_TEMPORARY;
}

wwd_result_t host_buffer_get(wiced_buffer_t *buffer, wwd_buffer_dir_t direction,
                             unsigned short size, wiced_bool_t wait)
{
    unsigned long timeout = (wait == WICED_TRUE) ? WICED_NEVER_TIMEOUT : 0;
    return internal_host_buffer_get(buffer, direction, size, timeout);
}

void host_buffer_release(wiced_buffer_t buffer, wwd_buffer_dir_t direction)
{
    (void)direction;
    if (buffer != NULL)
    {
        pbuf_free(buffer);
    }
}

uint8_t *host_buffer_get_current_piece_data_pointer(wiced_buffer_t buffer)
{
    return (uint8_t *)buffer->payload;
}

uint16_t host_buffer_get_current_piece_size(wiced_buffer_t buffer)
{
    return buffer->len;
}

wiced_buffer_t host_buffer_get_next_piece(wiced_buffer_t buffer)
{
    return buffer->next;
}

wwd_result_t host_buffer_set_next_piece(wiced_buffer_t buffer, wiced_buffer_t next_buffer)
{
    buffer->next = next_buffer;
    return WWD_SUCCESS;
}

wwd_result_t host_buffer_add_remove_at_front(wiced_buffer_t *buffer, int32_t add_remove_amount)
{
    if (pbuf_header(*buffer, (s16_t)(-add_remove_amount)) != 0)
    {
        return WWD_BUFFER_POINTER_MOVE_ERROR;
    }
    return WWD_SUCCESS;
}

wwd_result_t host_buffer_set_size(wiced_buffer_t buffer, unsigned short size)
{
    if (size > buffer->len + (uint16_t)((uint8_t *)buffer->payload - (uint8_t *)buffer))
    {
        return WWD_BUFFER_SIZE_SET_ERROR;
    }
    buffer->tot_len = size;
    buffer->len = size;
    return WWD_SUCCESS;
}

wiced_buffer_t host_buffer_get_queue_next(wiced_buffer_t buffer)
{
    /* In the SDK, the queue pointer is stored in the first bytes of payload.
     * We use the same approach: treat first bytes of payload as the queue link. */
    return *((wiced_buffer_t *)buffer->payload);
}

wwd_result_t host_buffer_set_queue_next(wiced_buffer_t buffer, wiced_buffer_t next)
{
    *((wiced_buffer_t *)buffer->payload) = next;
    return WWD_SUCCESS;
}

wiced_bool_t host_buffer_pool_is_full(wwd_buffer_dir_t direction)
{
    struct pbuf *test;
    (void)direction;

    test = pbuf_alloc(PBUF_RAW, 1, PBUF_POOL);
    if (test == NULL)
    {
        return WICED_TRUE;
    }
    pbuf_free(test);
    return WICED_FALSE;
}

void memp_free_notify(unsigned int type)
{
    (void)type;
    /* Notify the bus that a buffer was freed, so any blocked transmit can retry */
    host_platform_bus_buffer_freed(WWD_NETWORK_TX);
}

/* ----- Simple FIFO implementation ----- */

void host_buffer_init_fifo(wiced_buffer_fifo_t *fifo)
{
    memset(fifo, 0, sizeof(*fifo));
}

void host_buffer_push_to_fifo(wiced_buffer_fifo_t *fifo, wiced_buffer_t buffer,
                              wwd_interface_t interface)
{
    wiced_buffer_interface_fifo_t *ififo = &fifo->per_interface_fifos[interface & 0x03];

    host_buffer_set_queue_next(buffer, NULL);

    if (ififo->last != NULL)
    {
        host_buffer_set_queue_next(ififo->last, buffer);
    }
    else
    {
        ififo->first = buffer;
    }
    ififo->last = buffer;
}

wiced_buffer_t host_buffer_pop_from_fifo(wiced_buffer_fifo_t *fifo,
                                         wwd_interface_t *interface_return)
{
    int i;
    for (i = 0; i < (int)WWD_INTERFACE_MAX; i++)
    {
        wiced_buffer_interface_fifo_t *ififo = &fifo->per_interface_fifos[i];
        if (ififo->first != NULL)
        {
            wiced_buffer_t buf = ififo->first;
            ififo->first = host_buffer_get_queue_next(buf);
            if (ififo->first == NULL)
            {
                ififo->last = NULL;
            }
            host_buffer_set_queue_next(buf, NULL);
            if (interface_return != NULL)
            {
                *interface_return = (wwd_interface_t)i;
            }
            return buf;
        }
    }
    return NULL;
}
