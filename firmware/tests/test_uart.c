// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#define CLOCK_UART_TEST 1
#include "../iot_sdk_work/clock_project/clock_uart.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct uart_device uart;
static int available = 1;
static uint32 tick;
uint32 os_jiffies(void) { return tick; }
uint32 os_jiffies_to_msecs(uint32 ticks) { return ticks; }
void *dev_get(int id) { (void)id; return available ? &uart : NULL; }
int uart_request_irq(struct uart_device *u,
    int32 (*cb)(uint32,uint32,uint32,uint32), int flags, uint32 arg)
{
    (void)arg; assert(u == &uart && cb == clock_uart_irq && flags == 1); return RET_OK;
}
int uart_puts(struct uart_device *u, uint8 *s, uint32 length)
{
    assert(u == &uart && s && length); return RET_OK;
}
static void inject(const char *s) { while (*s) clock_uart_irq(0,0,(uint8)*s++,0); }
int main(void)
{
    unsigned i;
    uint32 received;
    assert(clock_uart_start() == RET_OK);
    inject("WIFIID\r\nWIFIAP\r\nWIFISTA\r\nWIFIRESET\r\n");
    assert(clock_uart_next_command() == TXW_HC32_COMMAND_WIFI_ID);
    assert(clock_uart_next_command() == TXW_HC32_COMMAND_WIFI_AP);
    assert(clock_uart_next_command() == TXW_HC32_COMMAND_WIFI_STA);
    assert(clock_uart_next_command() == TXW_HC32_COMMAND_WIFI_RESET);
    assert(clock_uart_next_command() == TXW_HC32_COMMAND_NONE);
    for (i = 0; i < 1000; ++i) {
        inject("WIFIID\r\n"); assert(clock_uart_next_command() == TXW_HC32_COMMAND_WIFI_ID);
    }
    tick = 10000; inject("WIFIRESET\r\n");
    tick = 11300; inject("WIFIRESET\r\n"); tick = 20000;
    assert(clock_uart_next_command_at(&received) == TXW_HC32_COMMAND_WIFI_RESET && received == 10000);
    assert(clock_uart_next_command_at(&received) == TXW_HC32_COMMAND_WIFI_RESET && received == 11300);
    tick = UINT32_MAX - 100u; inject("WIFIRESET\r\n");
    tick += 1300; inject("WIFIRESET\r\n");
    assert(clock_uart_next_command_at(&received) == TXW_HC32_COMMAND_WIFI_RESET && received == UINT32_MAX - 100u);
    assert(clock_uart_next_command_at(&received) == TXW_HC32_COMMAND_WIFI_RESET && received == 1199u);
    inject("WIFI"); assert(clock_uart_next_command() == TXW_HC32_COMMAND_NONE);
    inject("RESET\r\n"); assert(clock_uart_next_command() == TXW_HC32_COMMAND_WIFI_RESET);
    for (i = 0; i < 128; ++i) clock_uart_irq(0,0,'X',0);
    assert(rx_overflow && clock_uart_next_command() == TXW_HC32_COMMAND_NONE);
    inject("WIFIID\r\n"); assert(clock_uart_next_command() == TXW_HC32_COMMAND_WIFI_ID);
    inject("noise\r\nWIFIID\r\n"); assert(clock_uart_next_command() == TXW_HC32_COMMAND_WIFI_ID);
    assert(!clock_uart_send(NULL,1) && !clock_uart_send("a",0) && !clock_uart_send("a",65));
    assert(clock_uart_send("WIFIIDOK\r\n",10));
    available = 0; assert(!clock_uart_send("a",1) && clock_uart_start() == RET_ERR);
    puts("UART hardware-boundary tests: command order, ring wrap, actual arrival timestamps, tick rollover, fragmentation, overflow recovery and transmit guards passed");
    return 0;
}
