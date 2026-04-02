/*
 * lwIP performance measurement hooks — empty stubs.
 *
 * This header shadows WICED/network/LwIP/WWD/FreeRTOS/arch/perf.h.
 * Provides no-op PERF_START / PERF_STOP to satisfy lwip/opt.h.
 * No FreeRTOS dependency.
 */
#ifndef __PERF_H__
#define __PERF_H__

#define PERF_START    /* empty */
#define PERF_STOP(x)  /* empty */

#endif /* __PERF_H__ */
