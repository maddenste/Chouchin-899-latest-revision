// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TXW_CLOCK_SETTINGS_H
#define TXW_CLOCK_SETTINGS_H

#include <stddef.h>
#include <stdint.h>

#define TXW_CLOCK_SETTINGS_MAGIC UINT32_C(0x314B4C43) /* CLK1 */
#define TXW_CLOCK_SETTINGS_VERSION 3u
#define TXW_CLOCK_DEFAULT_NTP "pool.ntp.org"
#define TXW_CLOCK_DEFAULT_NTP_BACKUP "time.cloudflare.com"
#define TXW_CLOCK_DEFAULT_TZ "GMT0BST,M3.5.0/1,M10.5.0/2"

// Fixed-size, versioned flash record. No pointers or implicit padding in the
// logical fields; length and CRC protect against incompatible future builds.
struct txw_clock_settings {
    uint32_t magic;
    uint16_t version;
    uint16_t length;
    uint32_t generation;
    char ssid[33];
    char password[65];
    char ntp_host[128];
    char ntp_backup_host[128];
    char timezone[65];
    uint8_t daily_hour;
    uint8_t daily_minute;
    uint8_t movement_mode;
    uint8_t night_parking; // 0=Off, 1=Night parking, 2=continuous park at 12
    uint8_t psk[32];
    uint8_t psk_ready;
    uint8_t reserved[3];
    uint32_t crc32;
};

void txw_clock_settings_defaults(struct txw_clock_settings *settings);
void txw_clock_settings_seal(struct txw_clock_settings *settings,
                             uint32_t generation);
int txw_clock_settings_valid(const struct txw_clock_settings *settings);
int txw_clock_settings_values_valid(const struct txw_clock_settings *settings);
int txw_clock_settings_credentials_valid(const struct txw_clock_settings *settings);
uint32_t txw_clock_settings_crc32(const void *data, size_t length);

#endif
