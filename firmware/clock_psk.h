// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef CLOCK_PSK_H
#define CLOCK_PSK_H
#include <stdint.h>
/* WPA2 PBKDF2-HMAC-SHA1, 4096 rounds. Yield keeps UART/network responsive. */
int clock_psk_derive(const char *ssid, const char *password, uint8_t psk[32],
                     void (*yield_cpu)(void));
#endif
