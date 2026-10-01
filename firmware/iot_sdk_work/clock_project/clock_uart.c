#include "sys_config.h"
#include "typesdef.h"
#include "dev.h"
#include "devid.h"
#include "hal/uart.h"
#include "clock_uart.h"

#define CLOCK_RX_CAPACITY 128u
#define CLOCK_RX_MASK (CLOCK_RX_CAPACITY - 1u)

static struct txw_hc32_uart parser;
static volatile uint8 rx_data[CLOCK_RX_CAPACITY];
static volatile uint8 rx_head;
static volatile uint8 rx_tail;
static volatile uint8 rx_overflow;

static int32 clock_uart_irq(uint32 irq, uint32 irq_data,
                            uint32 param1, uint32 param2)
{
    uint8 next;
    (void)irq;
    (void)irq_data;
    (void)param2;
    if (param1 > 0xffu) return RET_OK;
    next = (uint8)((rx_head + 1u) & CLOCK_RX_MASK);
    if (next == rx_tail) {
        rx_overflow = 1u;
        return RET_OK;
    }
    rx_data[rx_head] = (uint8)param1;
    rx_head = next;
    return RET_OK;
}

int32 clock_uart_start(void)
{
    struct uart_device *uart = (struct uart_device *)dev_get(HG_UART0_DEVID);
    if (uart == NULL) return RET_ERR;
    rx_head = rx_tail = rx_overflow = 0u;
    txw_hc32_uart_init(&parser);
    return uart_request_irq(uart, clock_uart_irq, UART_IRQ_FLAG_RX_BYTE,
                            (uint32)uart);
}

enum txw_hc32_command clock_uart_next_command(void)
{
    enum txw_hc32_command command;
    if (rx_overflow) {
        rx_tail = rx_head;
        rx_overflow = 0u;
        txw_hc32_uart_init(&parser);
        return TXW_HC32_COMMAND_NONE;
    }
    while (rx_tail != rx_head) {
        uint8 byte = rx_data[rx_tail];
        rx_tail = (uint8)((rx_tail + 1u) & CLOCK_RX_MASK);
        command = txw_hc32_uart_feed(&parser, byte);
        if (command != TXW_HC32_COMMAND_NONE) return command;
    }
    return TXW_HC32_COMMAND_NONE;
}

int clock_uart_send(const char *bytes, uint32 length)
{
    struct uart_device *uart = (struct uart_device *)dev_get(HG_UART0_DEVID);
    if (uart == NULL || bytes == NULL || length == 0u || length > 64u)
        return 0;
    return uart_puts(uart, (uint8 *)bytes, length) == RET_OK;
}
