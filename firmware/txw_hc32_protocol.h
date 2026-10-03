// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
// Portable TXW813/HC32 UART command and streaming policy. No SDK calls.
#ifndef TXW_HC32_PROTOCOL_H
#define TXW_HC32_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TXW_HC32_UART_BAUD 9600u
#define TXW_HC32_LINE_CAPACITY 32u
#define TXW_HC32_TIME_INTERVAL_MS 1000u
#define TXW_HC32_TIME_LINE_CAPACITY 64u

enum txw_hc32_command {
    TXW_HC32_COMMAND_NONE = 0,
    TXW_HC32_COMMAND_WIFI_ID,
    TXW_HC32_COMMAND_WIFI_AP,
    TXW_HC32_COMMAND_WIFI_STA,
    TXW_HC32_COMMAND_WIFI_RESET
};

struct txw_hc32_uart {
    char line[TXW_HC32_LINE_CAPACITY];
    uint8_t length;
    uint8_t discarding;
    uint8_t saw_cr;
};

struct txw_hc32_time_stream {
    uint32_t last_emit_ms;
    uint8_t started;
};

// Civil time and daily-update setting supplied by the clock application.
// No assumed timezone or current time is baked into this module.
struct txw_hc32_time {
    uint16_t year;
    uint8_t month;                 // 1..12
    uint8_t day;                   // valid day of month
    uint8_t weekday;               // 0=Sunday
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    int16_t utc_offset_minutes;
    uint8_t daily_update_hour;
    uint8_t daily_update_minute;
    uint8_t movement_mode;         // 0=original, 1=Jump, 2=Pause at 12, 3=both
    uint8_t night_parking;         // 0=Off (default), 1=Night parking, 2=On
};

enum txw_hc32_time_wire_format {
    TXW_HC32_TIME_FACTORY = 0,     // matches all five passive captures
    TXW_HC32_TIME_V11_SELECTORS = 1 // two-digit suffix; selector 2 needs HC32 V14+
};

// Initialize selectors to Gradual / Continuous / battery saver Off (suffix 00).
// Date/time remains invalid until a fresh NTP result fills it in.
void txw_hc32_time_init(struct txw_hc32_time *time);

void txw_hc32_uart_init(struct txw_hc32_uart *uart);

// Feed bytes in order. Only a complete CRLF line yields a command. No inter-byte
// timeout: the daily-wake capture has a 114 ms gap within WIFISTA OK.
// Repeated commands yield repeated events; the adapter must be idempotent.
enum txw_hc32_command txw_hc32_uart_feed(struct txw_hc32_uart *uart,
                                          uint8_t byte);

// Call when deciding whether to emit +TIME. The first call with a verified,
// fresh NTP-derived clock returns 1; thereafter at least 1000 ms must elapse.
// If fresh_time becomes false, streaming stops until it is true again.
// There is no fixed message count: the HC32 controls the power-off window.
void txw_hc32_time_stream_init(struct txw_hc32_time_stream *stream);
int txw_hc32_time_due(struct txw_hc32_time_stream *stream,
                       uint32_t now_ms, int fresh_time);

// Return bytes excluding NUL, or 0 on invalid date/selection or small buffer.
// The V11 selector extension is absent from the older factory captures.
// The public HC32 V15 firmware supports this extension and minute wake times.
size_t txw_hc32_format_time(char *out, size_t capacity,
                            const struct txw_hc32_time *time,
                            enum txw_hc32_time_wire_format format);

// Board application entry point: always emit the HC32 V11 two-digit suffix.
// The factory-format variant above is retained for comparison with captures.
size_t txw_hc32_format_time_for_clock(char *out, size_t capacity,
                                      const struct txw_hc32_time *time);

// Literal wire records observed at 9600 8N1, each terminated by CRLF.
// This portable module defines records, not actions. clock_app.c handles
// acknowledgements and settings reset; parsing alone never erases flash.
// The application sends WIFIAPPING for active browser leases, WIFIEXIT for
// inactive setup mode, and WIFIAPPOK after a saved-settings station switch.
extern const char txw_hc32_wifi_id_ok[];
extern const char txw_hc32_wifi_id_no_credentials[];
extern const char txw_hc32_wifi_ap_ok[];
extern const char txw_hc32_wifi_sta_ok[];
extern const char txw_hc32_wifi_reset_ok[];
extern const char txw_hc32_wifi_exit[];
extern const char txw_hc32_wifi_pairing[];
extern const char txw_hc32_wifi_app_ok[];

#ifdef __cplusplus
}
#endif
#endif
