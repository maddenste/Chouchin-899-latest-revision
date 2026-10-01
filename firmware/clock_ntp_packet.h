// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TXW_CLOCK_NTP_PACKET_H
#define TXW_CLOCK_NTP_PACKET_H

#include <stddef.h>
#include <stdint.h>

#define TXW_NTP_PACKET_BYTES 48u

void txw_ntp_make_request(uint8_t packet[TXW_NTP_PACKET_BYTES],
                          uint64_t nonce);
int txw_ntp_parse_reply(const uint8_t *packet, size_t length,
                        uint64_t nonce, uint32_t *unix_seconds);

#endif
