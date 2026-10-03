// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef CLOCK_NTP_TEST_HAL_H
#define CLOCK_NTP_TEST_HAL_H
#include "typesdef.h"
#include <stddef.h>
typedef uint64_t uint64;
typedef int err_t;
typedef unsigned socklen_t;
typedef struct { uint32 addr; } ip_addr_t;
struct os_task { int unused; };
struct in_addr { uint32 s_addr; };
struct sockaddr_in { uint16 sin_family, sin_port; struct in_addr sin_addr; };
struct sockaddr { uint16 family; };
struct timeval { long tv_sec, tv_usec; };
#define OS_HZ 1000u
#define OS_TASK_PRIORITY_NORMAL 1
#define OS_TASK_INIT(a,b,c,d,e,f) ((void)0)
#define LWIP_SO_RCVTIMEO 1
#define LWIP_SO_SNDRCVTIMEO_NONSTANDARD 0
#define AF_INET 2
#define SOCK_DGRAM 2
#define SOL_SOCKET 1
#define SO_RCVTIMEO 2
#define ERR_OK 0
#define ERR_INPROGRESS -5
#define ERR_ARG -16
#define ERR_MEM -1
static uint16 htons(uint16 n) { return (uint16)((n >> 8) | (n << 8)); }
uint32 os_jiffies(void);
void os_sleep_ms(uint32 ms);
uint32 cpu_intrpt_save(void);
void cpu_intrpt_restore(uint32 state);
int socket(int family, int type, int protocol);
int close(int fd);
int setsockopt(int fd, int level, int option, const void *value, size_t length);
int sendto(int fd, const void *bytes, size_t size, int flags, const struct sockaddr *address, socklen_t length);
int recvfrom(int fd, void *bytes, size_t size, int flags, struct sockaddr *address, socklen_t *length);
err_t tcpip_try_callback(void (*callback)(void *), void *arg);
err_t dns_gethostbyname(const char *host, ip_addr_t *address,
                       void (*callback)(const char *, const ip_addr_t *, void *), void *arg);
#endif
