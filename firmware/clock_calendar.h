// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TXW_CLOCK_CALENDAR_H
#define TXW_CLOCK_CALENDAR_H

#include <stdint.h>
#include "txw_hc32_protocol.h"

// POSIX fixed offsets and Mmonth.week.weekday[/time] DST rules are supported.
int txw_clock_timezone_supported(const char *zone);
int txw_clock_local_from_unix(uint32_t utc_seconds, const char *zone,
                              struct txw_hc32_time *result);

struct txw_clock_dst_change {
    uint32_t utc_seconds;
    int16_t before_minutes;
    int16_t after_minutes;
};
int txw_clock_next_dst_change(uint32_t utc_seconds, const char *zone,
                              struct txw_clock_dst_change *change);
// Outgoing-only DST wake override for HC32 V15 minute scheduling; retain the
// saved schedule otherwise. Second-based transitions round up one minute.
int txw_clock_next_wake(uint32_t utc_seconds, const char *zone,
                        uint8_t saved_hour, uint8_t saved_minute,
                        uint8_t *wake_hour, uint8_t *wake_minute);

#endif
