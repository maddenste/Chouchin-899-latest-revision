// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
/* Execute the real NTP worker's query/DNS path with deterministic mock time. */
#define CLOCK_NTP_TEST 1
#include "../iot_sdk_work/clock_project/clock_ntp.c"
#include <assert.h>
#include <stdio.h>

static uint32 ticks, receive_wait, test_started, backup_sent_at;
static unsigned sends, closes, dns_queries, recv_count;
static int scenario, active_server;
static uint8 sent[48];
static void (*late_callback)(const char *, const ip_addr_t *, void *);
static void *late_arg;
static const char *late_name;
uint32 os_jiffies(void) { return ticks; }
void os_sleep_ms(uint32 ms) { ticks += ms; }
uint32 cpu_intrpt_save(void) { return 0; }
void cpu_intrpt_restore(uint32 state) { (void)state; }
int socket(int family, int type, int protocol) { assert(family == AF_INET && type == SOCK_DGRAM && protocol == 0); return 7; }
int close(int fd) { assert(fd == 7); ++closes; return 0; }
int setsockopt(int fd, int level, int option, const void *value, size_t length) {
    const struct timeval *timeout = value;
    assert(fd == 7 && level == SOL_SOCKET && option == SO_RCVTIMEO && length == sizeof(*timeout));
    receive_wait = (uint32)(timeout->tv_sec * 1000 + timeout->tv_usec / 1000);
    assert(receive_wait > 0 && receive_wait <= 5000); return 0;
}
int sendto(int fd, const void *bytes, size_t size, int flags, const struct sockaddr *address, socklen_t length) {
    const struct sockaddr_in *server = (const struct sockaddr_in *)address;
    assert(fd == 7 && size == 48 && flags == 0 && length == sizeof(*server));
    assert(server->sin_port == htons(123));
    active_server = server->sin_addr.s_addr == 1 ? 0 : 1;
    if (active_server == 1) { backup_sent_at = ticks - test_started; assert(backup_sent_at >= 5000); }
    memcpy(sent, bytes, size); ++sends; return (int)size;
}
int recvfrom(int fd, void *bytes, size_t size, int flags, struct sockaddr *address, socklen_t *length) {
    uint8 *reply = bytes;
    struct sockaddr_in *source = (struct sockaddr_in *)address;
    uint32 stamp = UINT32_C(2208988800) + UINT32_C(1790858096);
    assert(fd == 7 && size >= 48 && flags == 0);
    ++recv_count;
    if ((scenario == 1 || scenario == 3 || scenario == 7 || scenario == 8) && (uint32)(ticks - test_started) < 5000) {
        ticks += receive_wait; return -1;
    }
    if (scenario == 4) { ticks += receive_wait; return -1; }
    if (scenario == 5) { network_ready = 0; return -1; }
    if (scenario == 6) { ++request_epoch; return -1; }
    ticks += 100;
    memset(reply, 0, 48); reply[0] = 0x24; reply[1] = 2;
    memcpy(reply + 24, sent + 40, 8);
    reply[40] = (uint8)(stamp >> 24); reply[41] = (uint8)(stamp >> 16);
    reply[42] = (uint8)(stamp >> 8); reply[43] = (uint8)stamp;
    source->sin_family = AF_INET; source->sin_port = htons(123);
    source->sin_addr.s_addr = 99; /* Redirected source, not the requested IP. */
    if (scenario == 7 || scenario == 8) {
        source->sin_addr.s_addr = 1; /* Late primary beats the secondary. */
        reply[43] ^= 1;
    }
    *length = sizeof(*source);
    if (scenario == 2 && recv_count == 1) reply[24] ^= 1;
    if (scenario == 2 && recv_count == 2) source->sin_port = htons(999);
    if (scenario == 9) reply[24] ^= 1; /* Invalid flood cannot extend deadline. */
    return 48;
}
err_t tcpip_try_callback(void (*callback)(void *), void *arg) { callback(arg); return ERR_OK; }
err_t dns_gethostbyname(const char *host, ip_addr_t *address,
                       void (*callback)(const char *, const ip_addr_t *, void *), void *arg) {
    ++dns_queries;
    if (scenario == 11 || (scenario == 10 && !strcmp(host, "primary.example.net"))) return ERR_MEM;
    if (!strcmp(host, "primary.example.net")) {
        if (scenario == 3) {
            late_callback = callback; late_arg = arg; late_name = host;
            return ERR_INPROGRESS;
        }
        address->addr = 1;
    } else {
        assert(!strcmp(host, "backup.example.net"));
        if (scenario == 8) {
            late_callback = callback; late_arg = arg; late_name = host;
            return ERR_INPROGRESS;
        }
        if (late_callback) {
            ip_addr_t old = {1};
            assert(!strcmp(late_name, "primary.example.net"));
            late_callback(late_name, &old, late_arg); late_callback = NULL;
        }
        address->addr = 2;
    }
    return ERR_OK;
}
static void reset(int test) {
    scenario = test; ticks = sends = closes = dns_queries = recv_count = 0;
    test_started = backup_sent_at = 0;
    memset(dns_lookups, 0, sizeof(dns_lookups)); late_callback = NULL;
    network_ready = 1; request_epoch = 1;
}
int main(void) {
    uint32 result, epoch;
    uint32 address; int state, error;
    reset(10);
    assert(query_servers("primary.example.net", "backup.example.net", 1, &result));
    assert(sends == 1 && dns_queries == 2 && backup_sent_at == 5000);
    clock_ntp_dns_status(0, &address, &state, &error);
    assert(address == 0 && state == 3 && error == ERR_MEM);
    clock_ntp_dns_status(1, &address, &state, &error);
    assert(address == 2 && state == 2 && error == ERR_OK);
    reset(11);
    assert(!query_servers("primary.example.net", "backup.example.net", 1, &result));
    assert(ticks == 10000 && sends == 0 && closes == 1);
    reset(0);
    assert(query_servers("primary.example.net", "backup.example.net", 1, &result));
    assert(result == 1790858096u && sends == 1 && closes == 1 && dns_queries == 1);
    reset(1);
    assert(query_servers("primary.example.net", "backup.example.net", 1, &result));
    assert(ticks == 5100 && sends == 2 && closes == 1 && dns_queries == 2 && backup_sent_at == 5000);
    reset(2);
    assert(query_servers("primary.example.net", "backup.example.net", 1, &result));
    assert(ticks == 300 && recv_count == 3 && sends == 1);
    reset(3);
    assert(query_servers("primary.example.net", "backup.example.net", 1, &result));
    assert(ticks == 5100 && sends == 2 && !dns_lookups[0].busy);
    reset(4);
    assert(!query_servers("primary.example.net", "backup.example.net", 1, &result));
    assert(ticks == 10000 && closes == 1);
    reset(5);
    assert(!query_servers("primary.example.net", "backup.example.net", 1, &result));
    assert(dns_queries == 1 && closes == 1);
    reset(7);
    assert(query_servers("primary.example.net", "backup.example.net", 1, &result));
    assert(result == (1790858096u ^ 1u) && sends == 2 && backup_sent_at == 5000);
    reset(8);
    assert(query_servers("primary.example.net", "backup.example.net", 1, &result));
    assert(result == (1790858096u ^ 1u) && sends == 1 && dns_queries == 2 && dns_lookups[1].busy);
    { ip_addr_t address = {2}; late_callback(late_name, &address, late_arg); late_callback = NULL; }
    reset(9);
    assert(!query_servers("primary.example.net", "backup.example.net", 1, &result));
    assert(ticks == 10000 && sends == 2 && closes == 1);
    reset(1); test_started = ticks = UINT32_MAX - 1000u;
    assert(query_servers("primary.example.net", "backup.example.net", 1, &result));
    assert((uint32)(ticks - test_started) == 5100 && backup_sent_at == 5000);
    reset(6); epoch = request_epoch;
    assert(!query_servers("primary.example.net", "backup.example.net", epoch, &result));
    assert(dns_queries == 1 && closes == 1);
    clock_ntp_begin("primary.example.net", "backup.example.net");
    assert(!strcmp(configured_host, "primary.example.net") && !strcmp(configured_backup_host, "backup.example.net"));
    puts("NTP integration: redirect, primary/secondary success, 5s overlap, late primary during secondary DNS, late callbacks, invalid flood deadline, timer rollover and cancellation passed");
    return 0;
}
