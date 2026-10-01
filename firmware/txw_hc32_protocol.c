// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#include "txw_hc32_protocol.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

const char txw_hc32_wifi_id_ok[] = "WIFIIDOK\r\n";
const char txw_hc32_wifi_id_no_credentials[] = "WIFIIDNC\r\n";
const char txw_hc32_wifi_ap_ok[] = "WIFIAP OK\r\n";
const char txw_hc32_wifi_sta_ok[] = "WIFISTA OK\r\n";
const char txw_hc32_wifi_reset_ok[] = "WIFIRESET OK\r\n";
const char txw_hc32_wifi_exit[] = "WIFIEXIT\r\n";
const char txw_hc32_wifi_pairing[] = "WIFIAPPING\r\n";
const char txw_hc32_wifi_app_ok[] = "WIFIAPPOK\r\n";

void txw_hc32_time_init(struct txw_hc32_time *time)
{
    if (time != NULL) {
        memset(time, 0, sizeof(*time));
        time->night_parking = 0u;
    }
}

void txw_hc32_uart_init(struct txw_hc32_uart *uart)
{
    if (uart != NULL) memset(uart, 0, sizeof(*uart));
}

static enum txw_hc32_command classify(const char *line)
{
    if (strcmp(line, "WIFIID") == 0) return TXW_HC32_COMMAND_WIFI_ID;
    if (strcmp(line, "WIFIAP") == 0) return TXW_HC32_COMMAND_WIFI_AP;
    if (strcmp(line, "WIFISTA") == 0) return TXW_HC32_COMMAND_WIFI_STA;
    if (strcmp(line, "WIFIRESET") == 0) return TXW_HC32_COMMAND_WIFI_RESET;
    return TXW_HC32_COMMAND_NONE;
}

enum txw_hc32_command txw_hc32_uart_feed(struct txw_hc32_uart *uart,
                                          uint8_t byte)
{
    enum txw_hc32_command command = TXW_HC32_COMMAND_NONE;
    if (uart == NULL) return command;
    if (byte == '\r') {
        if (uart->saw_cr) uart->discarding = 1;
        uart->saw_cr = 1;
        return command;
    }
    if (byte == '\n') {
        if (uart->saw_cr && !uart->discarding && uart->length != 0) {
            uart->line[uart->length] = '\0';
            command = classify(uart->line);
        }
        uart->length = 0;
        uart->discarding = 0;
        uart->saw_cr = 0;
        return command;
    }
    if (uart->saw_cr) {
        uart->saw_cr = 0;
        uart->discarding = 1;
    }
    if (uart->discarding) return command;
    if (byte < 0x20u || byte > 0x7eu ||
        (size_t)uart->length + 1u >= sizeof(uart->line)) {
        uart->length = 0;
        uart->discarding = 1;
        return command;
    }
    uart->line[uart->length++] = (char)byte;
    return command;
}

void txw_hc32_time_stream_init(struct txw_hc32_time_stream *stream)
{
    if (stream != NULL) memset(stream, 0, sizeof(*stream));
}

int txw_hc32_time_due(struct txw_hc32_time_stream *stream,
                       uint32_t now_ms, int fresh_time)
{
    if (stream == NULL) return 0;
    if (!fresh_time) {
        stream->started = 0;
        return 0;
    }
    if (!stream->started ||
        (uint32_t)(now_ms - stream->last_emit_ms) >= TXW_HC32_TIME_INTERVAL_MS) {
        stream->started = 1;
        stream->last_emit_ms = now_ms;
        return 1;
    }
    return 0;
}

static unsigned days_in_month(unsigned year, unsigned month)
{
    static const uint8_t days[] = {31, 28, 31, 30, 31, 30,
                                   31, 31, 30, 31, 30, 31};
    unsigned leap;
    if (month < 1 || month > 12) return 0;
    leap = (year % 4u == 0u && (year % 100u != 0u || year % 400u == 0u));
    return days[month - 1u] + (month == 2u && leap ? 1u : 0u);
}

size_t txw_hc32_format_time(char *out, size_t capacity,
                            const struct txw_hc32_time *t,
                            enum txw_hc32_time_wire_format format)
{
    static const char *const weekdays[] =
        {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char *const months[] =
        {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
         "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    unsigned offset, offset_hour, offset_minute;
    char sign;
    int written;

    if (out == NULL || capacity == 0u) return 0;
    out[0] = '\0';
    if (t == NULL || (format != TXW_HC32_TIME_FACTORY &&
                      format != TXW_HC32_TIME_V11_SELECTORS) ||
        t->year < 2000 || t->year > 2099 ||
        t->day < 1 || t->day > days_in_month(t->year, t->month) ||
        t->weekday > 6 || t->hour > 23 || t->minute > 59 ||
        t->second > 59 || t->daily_update_hour > 23 ||
        t->daily_update_minute > 59 || t->movement_mode > 3 ||
        t->night_parking > 2 || t->utc_offset_minutes < -1439 ||
        t->utc_offset_minutes > 1439) return 0;

    sign = t->utc_offset_minutes < 0 ? '-' : '+';
    offset = (unsigned)(t->utc_offset_minutes < 0 ?
                        -(int)t->utc_offset_minutes : (int)t->utc_offset_minutes);
    offset_hour = offset / 60u;
    offset_minute = offset % 60u;
    if (format == TXW_HC32_TIME_FACTORY) {
        written = snprintf(out, capacity,
            "+TIME:%s %s %2u %02u:%02u:%02u %04u %c%02u%02u %02u:%02u\r\n",
            weekdays[t->weekday], months[t->month - 1u], (unsigned)t->day,
            (unsigned)t->hour, (unsigned)t->minute, (unsigned)t->second,
            (unsigned)t->year, sign, offset_hour, offset_minute,
            (unsigned)t->daily_update_hour, (unsigned)t->daily_update_minute);
    } else {
        written = snprintf(out, capacity,
            "+TIME:%s %s %2u %02u:%02u:%02u %04u %c%02u%02u %02u:%02u %u%u\r\n",
            weekdays[t->weekday], months[t->month - 1u], (unsigned)t->day,
            (unsigned)t->hour, (unsigned)t->minute, (unsigned)t->second,
            (unsigned)t->year, sign, offset_hour, offset_minute,
            (unsigned)t->daily_update_hour, (unsigned)t->daily_update_minute,
            (unsigned)t->movement_mode, (unsigned)t->night_parking);
    }
    if (written < 0 || (size_t)written >= capacity) {
        out[0] = '\0';
        return 0;
    }
    return (size_t)written;
}

size_t txw_hc32_format_time_for_clock(char *out, size_t capacity,
                                      const struct txw_hc32_time *time)
{
    return txw_hc32_format_time(out, capacity, time,
                                TXW_HC32_TIME_V11_SELECTORS);
}
