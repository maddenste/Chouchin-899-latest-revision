// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#include "clock_calendar.h"
#include "clock_settings.h"
#include <string.h>

static int leap(unsigned year)
{
    return (year % 4u == 0u && (year % 100u != 0u || year % 400u == 0u));
}

static unsigned month_days(unsigned year, unsigned month)
{
    static const uint8_t days[12] =
        {31,28,31,30,31,30,31,31,30,31,30,31};
    return days[month - 1u] + (month == 2u && leap(year) ? 1u : 0u);
}

struct dst_rule { unsigned month, week, day; int seconds; };

static int number(const char **cursor, unsigned maximum, unsigned *out)
{
    const char *p = *cursor;
    unsigned n = 0, count = 0;
    while (*p >= '0' && *p <= '9') {
        if (++count > 3u) return 0;
        n = n * 10u + (unsigned)(*p++ - '0');
        if (n > maximum) return 0;
    }
    if (!count) return 0;
    *cursor = p; *out = n; return 1;
}

static int name(const char **cursor)
{
    const char *p = *cursor;
    unsigned n = 0;
    if (*p == '<') {
        ++p;
        while ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') ||
               (*p >= '0' && *p <= '9') || *p == '+' || *p == '-') { ++p; ++n; }
        if (*p++ != '>' || n < 3u) return 0;
    } else {
        while ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z')) { ++p; ++n; }
        if (n < 3u) return 0;
    }
    *cursor = p; return 1;
}

static int duration(const char **cursor, unsigned max_hour, int *seconds)
{
    unsigned h, m = 0, s = 0;
    int sign = 1;
    const char *p = *cursor;
    if (*p == '-' || *p == '+') { if (*p == '-') sign = -1; ++p; }
    if (!number(&p, max_hour, &h)) return 0;
    if (*p == ':') { ++p; if (!number(&p, 59, &m)) return 0; }
    if (*p == ':') { ++p; if (!number(&p, 59, &s)) return 0; }
    *seconds = sign * (int)(h * 3600u + m * 60u + s);
    *cursor = p; return 1;
}

static int rule(const char **cursor, struct dst_rule *r)
{
    const char *p = *cursor;
    if (*p++ != 'M' || !number(&p, 12, &r->month) || !r->month ||
        *p++ != '.' || !number(&p, 5, &r->week) || !r->week ||
        *p++ != '.' || !number(&p, 6, &r->day)) return 0;
    r->seconds = 7200;
    if (*p == '/') { ++p; if (!duration(&p, 167, &r->seconds)) return 0; }
    *cursor = p; return 1;
}

static int64_t transition(unsigned year, const struct dst_rule *r, int prior_offset)
{
    unsigned y, m, days = 0, first_weekday, day;
    for (y = 1970; y < year; ++y) days += leap(y) ? 366u : 365u;
    for (m = 1; m < r->month; ++m) days += month_days(year, m);
    first_weekday = (days + 4u) % 7u;
    day = 1u + (r->day + 7u - first_weekday) % 7u + (r->week - 1u) * 7u;
    if (day > month_days(year, r->month)) day -= 7u;
    return (int64_t)(days + day - 1u) * 86400 + r->seconds - prior_offset;
}

struct zone_rules {
    int standard, daylight, has_dst;
    struct dst_rule start, end;
};

static int parse_zone(const char *zone, struct zone_rules *rules)
{
    int standard, daylight;
    const char *p;
    struct dst_rule start, end;
    if (zone == NULL || rules == NULL) return 0;
    memset(rules, 0, sizeof(*rules));
    if (strcmp(zone, "UTC") == 0 || strcmp(zone, "GMT") == 0) {
        return 1;
    }
    /* Retain the R1 human-readable fixed-offset syntax. POSIX offsets below
       have the opposite sign, e.g. IST-5:30 means UTC+05:30. */
    if (strlen(zone) == 9u && strncmp(zone, "UTC", 3) == 0 &&
        (zone[3] == '+' || zone[3] == '-') && zone[6] == ':') {
        p = zone + 3;
        if (!duration(&p, 23, &standard) || *p || standard % 60) return 0;
        rules->standard = standard; return 1;
    }
    p = zone;
    if (!name(&p) || !duration(&p, 23, &standard) || standard % 60) return 0;
    standard = -standard;
    if (!*p) { rules->standard = standard; return 1; }
    if (!name(&p)) return 0;
    daylight = standard + 3600;
    if (*p != ',') {
        if (!duration(&p, 23, &daylight) || daylight % 60) return 0;
        daylight = -daylight;
    }
    if (*p++ != ',' || !rule(&p, &start) || *p++ != ',' ||
        !rule(&p, &end) || *p || daylight < -86400 || daylight > 86400) return 0;
    rules->standard = standard; rules->daylight = daylight;
    rules->has_dst = 1; rules->start = start; rules->end = end;
    return 1;
}

static int offset_for_zone(uint32_t utc, const char *zone, int *minutes)
{
    unsigned year = 1970u, days;
    int standard, daylight;
    int64_t begin, finish, local;
    struct zone_rules rules;
    if (minutes == NULL || !parse_zone(zone, &rules)) return 0;
    standard = rules.standard; daylight = rules.daylight;
    if (!rules.has_dst) { *minutes = standard / 60; return 1; }
    local = (int64_t)utc + standard;
    if (local < 0) return 0;
    days = (unsigned)(local / 86400);
    while (days >= (leap(year) ? 366u : 365u)) {
        days -= leap(year) ? 366u : 365u; ++year;
    }
    begin = transition(year, &rules.start, standard);
    finish = transition(year, &rules.end, daylight);
    if (begin == finish) return 0;
    *minutes = ((begin < finish) ? ((int64_t)utc >= begin && (int64_t)utc < finish) :
                     ((int64_t)utc >= begin || (int64_t)utc < finish)) ? daylight / 60 : standard / 60;
    return 1;
}

int txw_clock_next_dst_change(uint32_t utc, const char *zone,
                              struct txw_clock_dst_change *change)
{
    struct zone_rules rules;
    unsigned year = 1970u, days, candidate_year, kind;
    int64_t local, at, best = (int64_t)UINT32_MAX + 1;
    if (change == NULL || !parse_zone(zone, &rules) || !rules.has_dst ||
        rules.standard == rules.daylight) return 0;
    local = (int64_t)utc + rules.standard;
    if (local < 0) return 0;
    days = (unsigned)(local / 86400);
    while (days >= (leap(year) ? 366u : 365u)) {
        days -= leap(year) ? 366u : 365u; ++year;
    }
    // Signed/extended transition times can cross a year boundary.
    for (candidate_year = year > 1970u ? year - 1u : year;
         candidate_year <= year + 1u; ++candidate_year) {
        if (transition(candidate_year, &rules.start, rules.standard) ==
            transition(candidate_year, &rules.end, rules.daylight)) continue;
        for (kind = 0; kind < 2; ++kind) {
            int before = kind == 0 ? rules.standard : rules.daylight;
            int after = kind == 0 ? rules.daylight : rules.standard;
            at = transition(candidate_year, kind == 0 ? &rules.start : &rules.end, before);
            if (at > utc && at < best) {
                best = at;
                change->utc_seconds = (uint32_t)at;
                change->before_minutes = (int16_t)(before / 60);
                change->after_minutes = (int16_t)(after / 60);
            }
        }
    }
    return best <= UINT32_MAX;
}

int txw_clock_next_wake(uint32_t utc, const char *zone,
                        uint8_t saved_hour, uint8_t saved_minute,
                        uint8_t *wake_hour, uint8_t *wake_minute)
{
    struct txw_clock_dst_change change;
    int offset;
    int64_t local_now, normal_wake, change_on_old_clock, minute_wake;
    if (wake_hour == NULL || wake_minute == NULL) return 0;
    *wake_hour = saved_hour; *wake_minute = saved_minute;
    if (saved_hour > 23u || saved_minute > 59u ||
        !offset_for_zone(utc, zone, &offset) ||
        !txw_clock_next_dst_change(utc, zone, &change)) return 0;
    local_now = (int64_t)utc + offset * 60;
    if (local_now < 0) return 0;
    normal_wake = (local_now / 86400) * 86400 + saved_hour * 3600u + saved_minute * 60u;
    if (normal_wake <= local_now) normal_wake += 86400;
    // HC32 still runs on the old offset until it receives corrected +TIME.
    change_on_old_clock = (int64_t)change.utc_seconds + offset * 60;
    // HC32 V15 can wake at any minute. Never wake before a transition that
    // specifies seconds: round only those transitions up to the next minute.
    minute_wake = ((change_on_old_clock + 59) / 60) * 60;
    if (minute_wake <= local_now || minute_wake >= normal_wake) return 0;
    *wake_hour = (uint8_t)((minute_wake / 3600) % 24);
    *wake_minute = (uint8_t)((minute_wake / 60) % 60);
    return 1;
}

int txw_clock_timezone_supported(const char *zone)
{
    int minutes;
    // This also rejects malformed strings without any clock dependency.
    return offset_for_zone(1704067200u, zone, &minutes);
}

int txw_clock_local_from_unix(uint32_t utc_seconds, const char *zone,
                              struct txw_hc32_time *result)
{
    int offset;
    int64_t local;
    uint32_t days, remainder;
    unsigned year = 1970u, month = 1u, year_days;
    if (result == NULL || !offset_for_zone(utc_seconds, zone, &offset)) return 0;
    local = (int64_t)utc_seconds + (int64_t)offset * 60;
    if (local < 0 || local > UINT32_MAX) return 0;
    days = (uint32_t)local / 86400u;
    remainder = (uint32_t)local % 86400u;
    result->weekday = (uint8_t)((days + 4u) % 7u);
    while (days >= (year_days = leap(year) ? 366u : 365u)) {
        days -= year_days;
        ++year;
    }
    if (year < 2000u || year > 2099u) return 0;
    while (days >= month_days(year, month)) {
        days -= month_days(year, month);
        ++month;
    }
    result->year = (uint16_t)year;
    result->month = (uint8_t)month;
    result->day = (uint8_t)(days + 1u);
    result->hour = (uint8_t)(remainder / 3600u);
    result->minute = (uint8_t)((remainder % 3600u) / 60u);
    result->second = (uint8_t)(remainder % 60u);
    result->utc_offset_minutes = (int16_t)offset;
    return 1;
}
