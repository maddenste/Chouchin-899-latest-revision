// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#include "clock_settings.h"
#include "clock_calendar.h"
#include <string.h>

static size_t bounded_length(const char *text, size_t capacity)
{
    size_t length;
    for (length = 0; length < capacity; ++length) {
        if (text[length] == '\0') return length;
    }
    return capacity;
}

static int printable(const char *text, size_t capacity)
{
    size_t i, length = bounded_length(text, capacity);
    if (length == capacity) return 0;
    for (i = 0; i < length; ++i) {
        if ((unsigned char)text[i] < 0x20u ||
            (unsigned char)text[i] > 0x7eu) return 0;
    }
    return 1;
}

static int hostname(const char *text, size_t capacity)
{
    size_t i, length = bounded_length(text, capacity);
    if (length == 0u || length == capacity || text[0] == '.' ||
        text[length - 1u] == '.') return 0;
    for (i = 0; i < length; ++i) {
        unsigned char c = (unsigned char)text[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '.' || c == '-')) return 0;
    }
    return 1;
}

uint32_t txw_clock_settings_crc32(const void *data, size_t length)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t crc = UINT32_C(0xffffffff);
    size_t i;
    unsigned bit;
    if (data == NULL) return 0;
    for (i = 0; i < length; ++i) {
        crc ^= bytes[i];
        for (bit = 0; bit < 8u; ++bit) {
            crc = (crc >> 1u) ^ ((crc & 1u) ? UINT32_C(0xedb88320) : 0u);
        }
    }
    return ~crc;
}

void txw_clock_settings_defaults(struct txw_clock_settings *settings)
{
    if (settings == NULL) return;
    memset(settings, 0, sizeof(*settings));
    memcpy(settings->ntp_host, TXW_CLOCK_DEFAULT_NTP,
           sizeof(TXW_CLOCK_DEFAULT_NTP));
    memcpy(settings->ntp_backup_host, TXW_CLOCK_DEFAULT_NTP_BACKUP,
           sizeof(TXW_CLOCK_DEFAULT_NTP_BACKUP));
    memcpy(settings->timezone, TXW_CLOCK_DEFAULT_TZ,
           sizeof(TXW_CLOCK_DEFAULT_TZ));
    settings->daily_hour = 10u;
    settings->night_parking = 0u;
}

void txw_clock_settings_seal(struct txw_clock_settings *settings,
                             uint32_t generation)
{
    if (settings == NULL) return;
    settings->magic = TXW_CLOCK_SETTINGS_MAGIC;
    settings->version = TXW_CLOCK_SETTINGS_VERSION;
    settings->length = (uint16_t)sizeof(*settings);
    settings->generation = generation;
    settings->crc32 = txw_clock_settings_crc32(settings,
                                  offsetof(struct txw_clock_settings, crc32));
}

int txw_clock_settings_credentials_valid(const struct txw_clock_settings *s)
{
    size_t ssid_len, password_len;
    if (s == NULL || !printable(s->ssid, sizeof(s->ssid)) ||
        !printable(s->password, sizeof(s->password))) return 0;
    ssid_len = bounded_length(s->ssid, sizeof(s->ssid));
    password_len = bounded_length(s->password, sizeof(s->password));
    return ssid_len >= 1u && ssid_len <= 32u &&
           (password_len == 0u ||
            (password_len >= 8u && password_len <= 63u));
}

int txw_clock_settings_values_valid(const struct txw_clock_settings *s)
{
    if (s == NULL || s->daily_hour > 23u ||
        s->daily_minute > 59u || s->movement_mode > 3u ||
        s->night_parking > 2u ||
        !printable(s->ssid, sizeof(s->ssid)) ||
        !printable(s->password, sizeof(s->password)) ||
        !hostname(s->ntp_host, sizeof(s->ntp_host)) ||
        !hostname(s->ntp_backup_host, sizeof(s->ntp_backup_host)) ||
        !printable(s->timezone, sizeof(s->timezone)) ||
        !txw_clock_timezone_supported(s->timezone)) return 0;
    return 1;
}

int txw_clock_settings_valid(const struct txw_clock_settings *s)
{
    if (!txw_clock_settings_values_valid(s) ||
        s->magic != TXW_CLOCK_SETTINGS_MAGIC ||
        s->version != TXW_CLOCK_SETTINGS_VERSION ||
        s->length != sizeof(*s) || s->psk_ready > 1u ||
        (s->password[0] != '\0' && !s->psk_ready)) return 0;
    return s->crc32 == txw_clock_settings_crc32(s,
                                  offsetof(struct txw_clock_settings, crc32));
}
