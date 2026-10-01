// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#include "txw_hc32_protocol.h"
#include <assert.h>
#include <string.h>

static enum txw_hc32_command feed(struct txw_hc32_uart *uart,
                                   const char *bytes)
{
    enum txw_hc32_command result = TXW_HC32_COMMAND_NONE;
    while (*bytes) result = txw_hc32_uart_feed(uart, (uint8_t)*bytes++);
    return result;
}

int main(void)
{
    struct txw_hc32_uart uart;
    struct txw_hc32_time_stream stream;
    struct txw_hc32_time t;
    char output[TXW_HC32_TIME_LINE_CAPACITY];
    char small[8];
    unsigned i;

    txw_hc32_uart_init(&uart);
    assert(feed(&uart, "WIFIID\r\n") == TXW_HC32_COMMAND_WIFI_ID);
    assert(feed(&uart, "WIFIAP\r\n") == TXW_HC32_COMMAND_WIFI_AP);
    assert(feed(&uart, "WIFISTA\r\n") == TXW_HC32_COMMAND_WIFI_STA);
    assert(feed(&uart, "WIFISTA\r\n") == TXW_HC32_COMMAND_WIFI_STA);
    assert(feed(&uart, "WIFIRESET\r\n") == TXW_HC32_COMMAND_WIFI_RESET);
    assert(feed(&uart, "USER\r\n") == TXW_HC32_COMMAND_NONE);
    assert(feed(&uart, "CLEAN\r\n") == TXW_HC32_COMMAND_NONE);
    assert(feed(&uart, "WIFISTA EXTRA\r\n") == TXW_HC32_COMMAND_NONE);
    assert(feed(&uart, "WIFIID\n") == TXW_HC32_COMMAND_NONE);
    assert(feed(&uart, "WIFI\rID\r\n") == TXW_HC32_COMMAND_NONE);
    assert(feed(&uart, "WIFIID\r\r\n") == TXW_HC32_COMMAND_NONE);
    assert(feed(&uart, "WIFIIDXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\r\n") ==
           TXW_HC32_COMMAND_NONE);
    assert(feed(&uart, "WIFIID\r\n") == TXW_HC32_COMMAND_WIFI_ID);
    assert(feed(&uart, "WIFI") == TXW_HC32_COMMAND_NONE);
    // No timeout is applied between these two calls.
    assert(feed(&uart, "STA\r\n") == TXW_HC32_COMMAND_WIFI_STA);
    assert(txw_hc32_uart_feed(&uart, 0) == TXW_HC32_COMMAND_NONE);
    assert(feed(&uart, "WIFISTA\r\n") == TXW_HC32_COMMAND_NONE);
    assert(feed(&uart, "WIFIID\r\n") == TXW_HC32_COMMAND_WIFI_ID);

    assert(strcmp(txw_hc32_wifi_id_ok, "WIFIIDOK\r\n") == 0);
    assert(strcmp(txw_hc32_wifi_id_no_credentials, "WIFIIDNC\r\n") == 0);
    assert(strcmp(txw_hc32_wifi_ap_ok, "WIFIAP OK\r\n") == 0);
    assert(strcmp(txw_hc32_wifi_sta_ok, "WIFISTA OK\r\n") == 0);
    assert(strcmp(txw_hc32_wifi_reset_ok, "WIFIRESET OK\r\n") == 0);
    assert(strcmp(txw_hc32_wifi_exit, "WIFIEXIT\r\n") == 0);
    assert(strcmp(txw_hc32_wifi_pairing, "WIFIAPPING\r\n") == 0);
    assert(strcmp(txw_hc32_wifi_app_ok, "WIFIAPPOK\r\n") == 0);

    txw_hc32_time_stream_init(&stream);
    assert(!txw_hc32_time_due(&stream, 5000, 0));
    assert(txw_hc32_time_due(&stream, 5200, 1));
    assert(!txw_hc32_time_due(&stream, 6199, 1));
    assert(txw_hc32_time_due(&stream, 6200, 1));
    for (i = 1; i <= 1000; ++i)
        assert(txw_hc32_time_due(&stream, 6200u + i * 1000u, 1));
    assert(!txw_hc32_time_due(&stream, 1007200u, 0));
    assert(txw_hc32_time_due(&stream, 1007201u, 1));

    txw_hc32_time_init(&t);
    assert(t.movement_mode == 0u);
    assert(t.night_parking == 0u);
    assert(t.year == 0u); // no stale time before NTP
    t.year = 2026;
    t.month = 9;
    t.day = 30;
    t.weekday = 3;
    t.second = 10;
    t.utc_offset_minutes = 210;

    assert(txw_hc32_format_time(output, sizeof(output), &t,
                                 TXW_HC32_TIME_FACTORY) == 44u);
    assert(strcmp(output,
        "+TIME:Wed Sep 30 00:00:10 2026 +0330 00:00\r\n") == 0);
    assert(txw_hc32_format_time_for_clock(output, sizeof(output), &t) == 47u);
    assert(strcmp(output,
        "+TIME:Wed Sep 30 00:00:10 2026 +0330 00:00 00\r\n") == 0);
    t.movement_mode = 3;
    t.night_parking = 0;
    assert(txw_hc32_format_time_for_clock(output, sizeof(output), &t) == 47u);
    assert(strcmp(output,
        "+TIME:Wed Sep 30 00:00:10 2026 +0330 00:00 30\r\n") == 0);
    t.night_parking = 2;
    assert(txw_hc32_format_time_for_clock(output, sizeof(output), &t) == 47u);
    assert(strstr(output, " 32\r\n"));
    t.night_parking = 3;
    assert(txw_hc32_format_time_for_clock(output, sizeof(output), &t) == 0u);
    t.night_parking = 0;
    assert(txw_hc32_format_time(small, sizeof(small), &t,
                                 TXW_HC32_TIME_FACTORY) == 0u);
    assert(small[0] == '\0');
    t.month = 2;
    t.day = 29;
    assert(txw_hc32_format_time(output, sizeof(output), &t,
                                 TXW_HC32_TIME_FACTORY) == 0u);
    t.year = 2024;
    assert(txw_hc32_format_time(output, sizeof(output), &t,
                                 TXW_HC32_TIME_FACTORY) == 44u);
    return 0;
}
