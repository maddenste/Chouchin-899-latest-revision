// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef CLOCK_UART_H
#define CLOCK_UART_H

#include "typesdef.h"
#include "../../txw_hc32_protocol.h"

// Inter-chip UART: buffered command reception and complete-record transmission.
int32 clock_uart_start(void);
enum txw_hc32_command clock_uart_next_command(void);
/* The firmware task passes received_ms to clock_app_command_at, rather than
 * timing queued reset records when they are eventually processed. */
enum txw_hc32_command clock_uart_next_command_at(uint32 *received_ms);
int clock_uart_send(const char *bytes, uint32 length);

#endif
