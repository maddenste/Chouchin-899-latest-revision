// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef UART_TEST_HAL_H
#define UART_TEST_HAL_H
#include "typesdef.h"
#include <stddef.h>
#define RET_OK 0
#define RET_ERR -1
#define HG_UART0_DEVID 0
#define UART_IRQ_FLAG_RX_BYTE 1
struct uart_device { int mock; };
void *dev_get(int id);
int uart_request_irq(struct uart_device *uart,
    int32 (*callback)(uint32, uint32, uint32, uint32), int flags, uint32 arg);
int uart_puts(struct uart_device *uart, uint8 *bytes, uint32 length);
uint32 os_jiffies(void);
uint32 os_jiffies_to_msecs(uint32 ticks);
#endif
