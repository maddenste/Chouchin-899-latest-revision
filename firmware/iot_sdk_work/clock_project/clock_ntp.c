#ifdef CLOCK_NTP_TEST
#include "ntp_test_hal.h"
#else
#include "sys_config.h"
#include "typesdef.h"
#include "osal/task.h"
#include "osal/sleep.h"
#include "osal/atomic.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/tcpip.h"
#endif
#include "clock_ntp.h"
#include "../../clock_ntp_packet.h"
#include "../../clock_settings.h"
#include <string.h>

#if !LWIP_SO_RCVTIMEO || LWIP_SO_SNDRCVTIMEO_NONSTANDARD
#error "Clock NTP requires enabled timeval socket receive timeouts"
#endif

static struct os_task ntp_task;
static volatile uint8 task_running;
static volatile uint8 network_ready;
static volatile uint8 fresh_result;
static volatile uint32 sync_seconds;
static volatile uint32 sync_tick;
static volatile uint32 request_epoch;
static char configured_host[128];
static char configured_backup_host[128];

/* Persistent callback storage: a timed-out DNS lookup may complete later.
 * Never reuse its slot until lwIP has delivered the callback. */
struct clock_dns_lookup {
    char host[128];
    volatile uint32 address;
    volatile uint8 busy, done, valid;
    volatile int error;
};
static struct clock_dns_lookup dns_lookups[2];

static void dns_complete(const char *name, const ip_addr_t *address, void *arg)
{
    struct clock_dns_lookup *lookup = arg;
    uint32 irq_state = cpu_intrpt_save();
    (void)name;
    lookup->valid = address != NULL && address->addr != 0u;
    lookup->address = lookup->valid ? address->addr : 0u;
    lookup->done = 1u;
    lookup->busy = 0u;
    cpu_intrpt_restore(irq_state);
}

static void dns_start(void *arg)
{
    struct clock_dns_lookup *lookup = arg;
    ip_addr_t address;
    err_t status = dns_gethostbyname(lookup->host, &address, dns_complete, lookup);
    lookup->error = status == ERR_INPROGRESS ? ERR_OK : status;
    if (status == ERR_OK) dns_complete(lookup->host, &address, lookup);
    else if (status != ERR_INPROGRESS) dns_complete(lookup->host, NULL, lookup);
}

static int start_lookup(const char *hostname, unsigned slot)
{
    struct clock_dns_lookup *lookup = &dns_lookups[slot];
    uint32 irq_state = cpu_intrpt_save();
    if (lookup->busy) {
        cpu_intrpt_restore(irq_state);
        return 0;
    }
    memcpy(lookup->host, hostname, strlen(hostname) + 1u);
    lookup->busy = 1u;
    lookup->done = lookup->valid = 0u;
    lookup->address = 0u;
    lookup->error = ERR_OK;
    cpu_intrpt_restore(irq_state);
    /* Queue DNS operations on lwIP's thread without blocking on its mailbox. */
    if (tcpip_try_callback(dns_start, lookup) != ERR_OK) {
        lookup->error = ERR_MEM;
        dns_complete(hostname, NULL, lookup);
        return 0;
    }
    return 1;
}

void clock_ntp_dns_status(unsigned slot, uint32 *address, int *state, int *error)
{
    struct clock_dns_lookup *lookup;
    uint32 irq_state;
    if (slot >= 2u || !address || !state || !error) return;
    lookup = &dns_lookups[slot];
    irq_state = cpu_intrpt_save();
    *address = lookup->address;
    *state = lookup->busy ? 1 : lookup->done ? (lookup->valid ? 2 : 3) : 0;
    *error = lookup->error;
    cpu_intrpt_restore(irq_state);
}

static int query_servers(const char *host, const char *backup_host,
                         uint32 epoch, uint32 *result)
{
    struct sockaddr_in server, source;
    socklen_t source_length = sizeof(source);
    struct timeval timeout;
    uint8 request[TXW_NTP_PACKET_BYTES];
    uint8 reply[TXW_NTP_PACKET_BYTES + 32u];
    uint64 nonce;
    uint32 unix_seconds;
    uint32 started, elapsed, remaining;
    uint8 queued[2] = {0u, 0u}, handled[2] = {0u, 0u};
    unsigned index, active = 1u, sent = 0u;
    int sock, received, success = 0;

    started = (uint32)os_jiffies();
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    if (!network_ready || request_epoch != epoch) return 0;
    server.sin_port = htons(123);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return 0;
    nonce = (uint64)os_jiffies() ^ ((uint64)(uint32)&ntp_task << 32u);
    if (nonce == 0u) nonce = 1u;
    txw_ntp_make_request(request, nonce);
    queued[0] = (uint8)start_lookup(host, 0u);
    while (network_ready && request_epoch == epoch) {
        elapsed = (uint32)os_jiffies() - started;
        if (elapsed >= 10u * OS_HZ) break;
        if (active == 1u && elapsed >= 5u * OS_HZ) {
            queued[1] = (uint8)start_lookup(backup_host, 1u);
            active = 2u;
        }
        for (index = 0; index < active; ++index) {
            struct clock_dns_lookup *lookup = &dns_lookups[index];
            uint32 irq_state;
            int ready, valid;
            if (!queued[index] || handled[index]) continue;
            irq_state = cpu_intrpt_save();
            ready = lookup->done;
            valid = lookup->valid;
            server.sin_addr.s_addr = lookup->address;
            cpu_intrpt_restore(irq_state);
            if (!ready) continue;
            handled[index] = 1u;
            if (valid && sendto(sock, request, sizeof(request), 0,
                    (struct sockaddr *)&server, sizeof(server)) == sizeof(request))
                ++sent;
        }
        if (!sent) { os_sleep_ms(10); continue; }
        /* One unconnected socket retains both outstanding requests. Sharing
         * their request token lets the first valid reply win, even when NAT
         * changes its source IP. Poll DNS without blocking reply reception. */
        elapsed = (uint32)os_jiffies() - started;
        if (elapsed >= 10u * OS_HZ) break;
        remaining = (active == 1u ? 5u : 10u) * OS_HZ - elapsed;
        if (remaining > OS_HZ / 10u) remaining = OS_HZ / 10u;
        if (!remaining) continue;
        timeout.tv_sec = remaining / OS_HZ;
        timeout.tv_usec = ((remaining % OS_HZ) * 1000000u) / OS_HZ;
        if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
                       &timeout, sizeof(timeout)) != 0) break;
        memset(&source, 0, sizeof(source));
        source_length = sizeof(source);
        received = recvfrom(sock, reply, sizeof(reply), 0,
                            (struct sockaddr *)&source, &source_length);
        if (!network_ready || request_epoch != epoch) break;
        if (received <= 0) {
            /* Avoid a busy loop if a socket error returns before its timeout. */
            if ((uint32)os_jiffies() - started == elapsed) os_sleep_ms(10);
            continue;
        }
        if ((uint32)((uint32)os_jiffies() - started) >= 10u * OS_HZ) break;
        if (source_length != sizeof(source) || source.sin_family != AF_INET ||
            source.sin_port != htons(123)) continue;
        /* A transparent NTP redirect may reply directly from a different IP.
         * Match the echoed 64-bit request token rather than the source IP.
         * This is request correlation, not cryptographic authentication.
         * Invalid/stale replies cannot extend the total ten-second window. */
        if (!txw_ntp_parse_reply(reply, (size_t)received,
                                nonce, &unix_seconds)) continue;
        *result = unix_seconds;
        success = 1;
        break;
    }
    close(sock);
    return success;
}

static void clock_ntp_worker(void *arg)
{
    uint32 result;
    uint32 active_epoch;
    char host[sizeof(configured_host)];
    char backup_host[sizeof(configured_backup_host)];
    unsigned refresh_wait;
    uint32 irq_state;
    (void)arg;
    for (;;) {
        if (!network_ready) {
            os_sleep_ms(250);
            continue;
        }
        irq_state = cpu_intrpt_save();
        active_epoch = request_epoch;
        memcpy(host, configured_host, sizeof(host));
        memcpy(backup_host, configured_backup_host, sizeof(backup_host));
        cpu_intrpt_restore(irq_state);
        if (query_servers(host, backup_host, active_epoch, &result)) {
            irq_state = cpu_intrpt_save();
            if (network_ready && request_epoch == active_epoch) {
                sync_seconds = result;
                sync_tick = (uint32)os_jiffies();
                fresh_result = 1u;
            }
            cpu_intrpt_restore(irq_state);
            // Refresh eventually for long setup sessions. Normal wakes are
            // terminated by the HC32 within seconds after +TIME.
            for (refresh_wait = 0; refresh_wait < 3600u && network_ready &&
                 request_epoch == active_epoch;
                 ++refresh_wait) os_sleep_ms(1000);
        } else {
            if (request_epoch == active_epoch) {
                os_sleep_ms(1000);
            }
        }
    }
}

void clock_ntp_begin(const char *host, const char *backup_host)
{
    size_t length;
    uint32 irq_state;
    if (host == NULL || backup_host == NULL) return;
    length = strlen(host);
    if (length == 0u || length >= sizeof(configured_host)) return;
    if (strlen(backup_host) == 0u ||
        strlen(backup_host) >= sizeof(configured_backup_host)) return;
    irq_state = cpu_intrpt_save();
    memcpy(configured_host, host, length + 1u);
    memcpy(configured_backup_host, backup_host, strlen(backup_host) + 1u);
    fresh_result = 0u;
    ++request_epoch;
    network_ready = 1u;
    cpu_intrpt_restore(irq_state);
    if (!task_running) {
        task_running = 1u;
        OS_TASK_INIT("clock-ntp", &ntp_task, clock_ntp_worker, NULL,
                     OS_TASK_PRIORITY_NORMAL, 4096);
    }
}

void clock_ntp_disconnected(void)
{
    uint32 irq_state = cpu_intrpt_save();
    network_ready = 0u;
    fresh_result = 0u;
    ++request_epoch;
    cpu_intrpt_restore(irq_state);
}

int clock_ntp_fresh(void)
{
    return fresh_result && network_ready;
}

int clock_ntp_utc_now(uint32 *unix_seconds)
{
    uint32 elapsed, seconds, tick, irq_state;
    int ready;
    if (unix_seconds == NULL) return 0;
    irq_state = cpu_intrpt_save();
    ready = fresh_result && network_ready;
    seconds = sync_seconds;
    tick = sync_tick;
    cpu_intrpt_restore(irq_state);
    if (!ready) return 0;
    elapsed = ((uint32)os_jiffies() - tick) / OS_HZ;
    if (elapsed > 86400u || UINT32_MAX - seconds < elapsed) return 0;
    *unix_seconds = seconds + elapsed;
    return 1;
}
