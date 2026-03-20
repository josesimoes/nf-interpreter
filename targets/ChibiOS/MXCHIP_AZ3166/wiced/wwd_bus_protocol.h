//
// Copyright (c) .NET Foundation and Contributors
// See LICENSE file in the project root for full license information.
//

// Redirect to the WICED SDK's actual SDIO bus protocol header.
// Our port directory is searched first, so this file intercepts
// #include "wwd_bus_protocol.h" and forwards to the SDK's real
// SDIO implementation via GCC's #include_next extension.

#ifndef INCLUDED_NF_WWD_BUS_PROTOCOL_REDIRECT_H_
#define INCLUDED_NF_WWD_BUS_PROTOCOL_REDIRECT_H_

#include_next "wwd_bus_protocol.h"

#endif // INCLUDED_NF_WWD_BUS_PROTOCOL_REDIRECT_H_
