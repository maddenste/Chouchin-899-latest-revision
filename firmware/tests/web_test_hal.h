// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#include <stdint.h>
#include <stddef.h>
#include "typesdef.h"
#define LWIP_SO_RCVTIMEO 1
#define LWIP_SO_SNDTIMEO 1
#define LWIP_SO_SNDRCVTIMEO_NONSTANDARD 0
#define SOL_SOCKET 1
#define SO_RCVTIMEO 2
#define SO_SNDTIMEO 3
#define IP2STR_N(x) (unsigned)((x)&255),(unsigned)(((x)>>8)&255),(unsigned)(((x)>>16)&255),(unsigned)(((x)>>24)&255)
typedef struct { uint32 addr; } ip_addr_t;
struct timeval { long tv_sec, tv_usec; };
struct hgic_bss_info { uint8 bssid[6], ssid[32], encrypt; signed char signal; uint16 freq; };
struct ieee80211_scandata { uint32 chan_bitmap; uint8 scan_time, scan_cnt; };
struct { uint8 wifi_connected, dhcpc_done; struct { uint32 dns1; } dhcpc_result; } sys_status;
struct { uint8 wifi_mode; } sys_cfgs;
int send(int fd, const void *bytes, size_t count, int flags);
int recv(int fd, void *bytes, size_t count, int flags);
int setsockopt(int fd, int level, int option, const void *v, size_t size);
ip_addr_t lwip_netif_get_ip2(const char *name);
uint32 os_jiffies(void);
uint32 os_jiffies_to_msecs(uint32 t);
int ieee80211_scan(uint8 mode, uint8 start, struct ieee80211_scandata *s);
int ieee80211_get_bsslist(struct hgic_bss_info *bss, int count);
