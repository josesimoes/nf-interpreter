/*
 * Stub tls_types.h for ChibiOS port — TLS is disabled (WICED_DISABLE_TLS).
 *
 * Provides empty type definitions so wiced_network.h and wiced_tcpip.h
 * compile without pulling in the full BESL/crypto stack.
 */

#pragma once

#include <stdint.h>

typedef struct { uint8_t unused; } wiced_tls_context_t;
typedef struct { uint8_t unused; } wiced_tls_session_t;
typedef struct { uint8_t unused; } wiced_tls_certificate_t;
typedef struct { uint8_t unused; } wiced_tls_key_t;
typedef struct { uint8_t unused; } wiced_tls_identity_t;

typedef enum { WICED_TLS_AS_CLIENT = 0, WICED_TLS_AS_SERVER = 1 } wiced_tls_endpoint_type_t;
typedef enum { TLS_NO_VERIFICATION = 0 } wiced_tls_certificate_verification_t;
typedef enum { TLS_TCP_TRANSPORT = 0 } tls_transport_protocol_t;
typedef uint16_t cipher_suite_t;
