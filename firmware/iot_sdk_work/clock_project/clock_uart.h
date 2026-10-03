// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef CLOCK_UART_H
#define CLOCK_UART_H

#include "typesdef.h"
#include "../../txw_hc32_protocol.h"

// Receive-only until the Wi-Fi/time adapters can produce truthful replies.
int32 clock_uart_start(void);
enum txw_hc32_command clock_uart_next_command(void);
int clock_uart_send(const char *bytes, uint32 length);

#endif
