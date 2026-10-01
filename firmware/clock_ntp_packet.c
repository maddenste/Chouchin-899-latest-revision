// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#include "clock_ntp_packet.h"
#include <string.h>

#define NTP_UNIX_DELTA UINT32_C(2208988800)
#define EARLIEST_VALID_UNIX UINT32_C(1704067200) /* 2024-01-01 UTC */

static uint32_t read_be32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24u) | ((uint32_t)bytes[1] << 16u) |
           ((uint32_t)bytes[2] << 8u) | bytes[3];
}

void txw_ntp_make_request(uint8_t packet[TXW_NTP_PACKET_BYTES],
                          uint64_t nonce)
{
    unsigned index;
    if (packet == NULL) return;
    memset(packet, 0, TXW_NTP_PACKET_BYTES);
    packet[0] = 0x23u; // LI=0, version=4, mode=3 (client)
    for (index = 0; index < 8u; ++index) {
        packet[40u + index] = (uint8_t)(nonce >> (56u - index * 8u));
    }
}

int txw_ntp_parse_reply(const uint8_t *packet, size_t length,
                        uint64_t nonce, uint32_t *unix_seconds)
{
    unsigned index;
    uint8_t version, leap, mode;
    uint64_t ntp_seconds, epoch;
    if (packet == NULL || unix_seconds == NULL || length < TXW_NTP_PACKET_BYTES)
        return 0;
    leap = packet[0] >> 6u;
    version = (packet[0] >> 3u) & 7u;
    mode = packet[0] & 7u;
    if (leap == 3u || (version != 3u && version != 4u) || mode != 4u ||
        packet[1] == 0u || packet[1] > 15u) return 0;
    for (index = 0; index < 8u; ++index) {
        if (packet[24u + index] !=
            (uint8_t)(nonce >> (56u - index * 8u))) return 0;
    }
    ntp_seconds = read_be32(packet + 40u);
    if (ntp_seconds < NTP_UNIX_DELTA) ntp_seconds += UINT64_C(0x100000000);
    epoch = ntp_seconds - NTP_UNIX_DELTA;
    if (epoch < EARLIEST_VALID_UNIX || epoch > UINT32_MAX) return 0;
    *unix_seconds = (uint32_t)epoch;
    return 1;
}
