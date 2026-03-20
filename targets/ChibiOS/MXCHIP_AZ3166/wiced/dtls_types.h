/*
 * Stub dtls_types.h for ChibiOS port — TLS is disabled (WICED_DISABLE_TLS).
 *
 * Provides empty type definitions so wiced_network.h and wiced_tcpip.h
 * compile without pulling in the full BESL/crypto stack.
 */

#pragma once

#include <stdint.h>

typedef struct { uint8_t unused; } wiced_dtls_context_t;
typedef enum { WICED_DTLS_AS_CLIENT = 0, WICED_DTLS_AS_SERVER = 1 } wiced_dtls_endpoint_type_t;
typedef enum { DTLS_NO_VERIFICATION = 0 } wiced_dtls_certificate_verification_t;
typedef enum { DTLS_UDP_TRANSPORT = 0 } dtls_transport_protocol_t;
