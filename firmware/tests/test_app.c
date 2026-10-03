// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
/* Exercise the firmware app itself; mock only the hardware/network boundary. */
#define CLOCK_BACKEND_TEST 1
#include "../iot_sdk_work/clock_project/clock_app.c"
#include <assert.h>

struct test_config sys_cfgs;
struct test_status sys_status;
static struct txw_clock_settings stored;
static uint32 ticks;
static uint32 ntp_utc = 1790858096u;
static int have_stored, fail_save, starts, ntp_ready;
static char sent[512];
static struct netdev wifi;
uint32 os_jiffies(void) { return ticks; }
uint32 os_jiffies_to_msecs(uint32 tick) { return tick; }
void os_sleep_ms(uint32 delay) { ticks += delay; }
void mcu_watchdog_feed(void) {}
int os_mutex_init(struct os_mutex *m) { m->locked = 0; return 0; }
int os_mutex_lock(struct os_mutex *m, int timeout) { (void)timeout; assert(!m->locked); m->locked = 1; return 0; }
int os_mutex_unlock(struct os_mutex *m) { assert(m->locked); m->locked = 0; return 0; }
void *dev_get(int id) { (void)id; return &wifi; }
void sysctrl_efuse_mac_addr_calc(uint8 *mac) { const uint8 fixture[6] = {2,0x11,0x22,0xab,0xcd,0xef}; memcpy(mac, fixture, 6); }
void dhcpd_start(const char *name, struct dhcpd_param *p) { (void)name; assert(p->start_ip == AP_START_ADDR); }
void dhcpd_stop(const char *name) { (void)name; }
void ieee80211_iface_stop(int mode) { (void)mode; }
int ieee80211_iface_start(int mode) { (void)mode; ++starts; return 0; }
void netdev_set_wifi_mode(struct netdev *dev, int mode) { (void)dev; (void)mode; }
void lwip_netif_set_ip2(const char *n, ip_addr_t *ip, ip_addr_t *m, ip_addr_t *g) { (void)n; (void)m; (void)g; sys_cfgs.ipaddr = ip->addr; }
void lwip_netif_set_dhcp2(const char *n, int e) { (void)n; (void)e; }
ip_addr_t lwip_netif_get_ip2(const char *n) { ip_addr_t p; (void)n; p.addr = sys_cfgs.ipaddr; return p; }
int32 wificfg_flush(uint8 mode) { (void)mode; return 0; }
int clock_uart_send(const char *bytes, uint32 len) { assert(strlen(sent) + len < sizeof(sent)); strncat(sent, bytes, len); return 1; }
int clock_storage_load(struct txw_clock_settings *s) { if (have_stored) { *s = stored; return 1; } txw_clock_settings_defaults(s); return 0; }
int clock_storage_save(struct txw_clock_settings *s) { if (fail_save) return 0; txw_clock_settings_seal(s, stored.generation + 1); assert(txw_clock_settings_valid(s)); stored = *s; have_stored = 1; return 1; }
int clock_storage_reset(struct txw_clock_settings *s) { struct txw_clock_settings blank; txw_clock_settings_defaults(&blank); if (!clock_storage_save(&blank)) return 0; *s = blank; return 1; }
void clock_ntp_begin(const char *host, const char *backup_host) { assert(strcmp(host, "pool.ntp.org") == 0); assert(strcmp(backup_host, "time.cloudflare.com") == 0); ntp_ready = 1; }
void clock_ntp_disconnected(void) { ntp_ready = 0; }
int clock_ntp_fresh(void) { return ntp_ready; }
int clock_ntp_utc_now(uint32 *out) { *out = ntp_utc; return ntp_ready; }
static void expect(const char *record) { if (strcmp(sent, record)) fprintf(stderr, "Expected [%s], got [%s] at tick %u\n", record, sent, ticks); assert(strcmp(sent, record) == 0); sent[0] = 0; }

int main(void)
{
    struct txw_clock_settings candidate;
    uint32 before;
    clock_app_prepare();
    assert(clock_app_is_portal());
    assert(strcmp(sys_cfgs.ssid, "WiFi-Clock-Setup") == 0);
    assert(strcmp(clock_app_hostname(), "WiFi-Clock-ABCDEF") == 0);
    clock_app_command(TXW_HC32_COMMAND_WIFI_ID); expect("WIFIIDNC\r\n");
    clock_app_command(TXW_HC32_COMMAND_WIFI_AP); expect("");
    clock_app_network_ready(); clock_app_tick(); expect("WIFIAP OK\r\n");
    clock_app_command(TXW_HC32_COMMAND_WIFI_AP); expect("WIFIAP OK\r\n");
    assert(starts == 0);
    /* AP inactivity emits EXIT; a page enables one-second power keepalive. */
    ticks = 4000; clock_app_tick(); expect("WIFIEXIT\r\n");
    clock_app_portal_client_seen();
    ticks += 1000; clock_app_tick(); expect("WIFIAPPING\r\n");
    ticks += 999; clock_app_tick(); expect("");
    ++ticks; clock_app_tick(); expect("WIFIAPPING\r\n");
    ticks += 13000; clock_app_tick(); expect("WIFIEXIT\r\n");
    clock_app_settings_copy(&candidate);
    strcpy(candidate.ssid, "IEEE"); strcpy(candidate.password, "password");
    assert(clock_app_save_settings(&candidate));
    assert(stored.psk_ready && txw_clock_settings_valid(&stored));
    ticks += 500; clock_app_tick(); expect("WIFIAPPOK\r\n");
    assert(starts == 1 && sys_cfgs.key_mgmt == WPA_KEY_MGMT_PSK);
    assert(memcmp(sys_cfgs.psk, stored.psk, 32) == 0);
    clock_app_command(TXW_HC32_COMMAND_WIFI_STA); expect("WIFISTA OK\r\n");
    clock_app_command(TXW_HC32_COMMAND_WIFI_STA); expect("WIFISTA OK\r\n");
    assert(starts == 1);
    clock_app_command(TXW_HC32_COMMAND_WIFI_ID); expect("WIFIIDOK\r\n");
    clock_app_tick(); expect("");
    /* STA webpage keeps power on too, even without successful NTP. */
    ticks += 1000;
    clock_app_portal_client_seen(); clock_app_tick(); expect("WIFIAPPING\r\n");
    ticks += 999; clock_app_tick(); expect("");
    ++ticks; clock_app_tick(); expect("WIFIAPPING\r\n");
    ticks += 14000; clock_app_tick(); expect("");
    clock_app_portal_client_seen(); clock_app_tick(); expect("WIFIAPPING\r\n");
    ticks += 15000; clock_app_tick(); expect("");
    clock_app_dhcp_ready(); clock_app_tick();
    assert(strstr(sent, "+TIME:") == sent && strstr(sent, " 10:00 00\r\n")); sent[0] = 0;
    ticks += 999; clock_app_tick(); expect("");
    ++ticks; clock_app_tick(); assert(strstr(sent, "+TIME:") == sent); sent[0] = 0;
    /* Only outgoing wake fields change; saved schedule/CRC are untouched. */
    {
        struct txw_clock_settings unchanged = stored;
        ntp_utc = 1792832400u; /* 2026-10-24 09:00 UTC = 10:00 BST. */
        ticks += 1000; clock_app_tick();
        assert(strstr(sent," 02:00 00\r\n")); sent[0] = 0;
        assert(!memcmp(&stored, &unchanged, sizeof(stored)));
        assert(settings.daily_hour == 10 && settings.daily_minute == 0);
        strcpy(settings.timezone,"AAA0BBB,M3.5.0/1:30,M10.5.0/2:30");
        ticks += 1000; clock_app_tick();
        assert(strstr(sent," 02:30 00\r\n")); sent[0] = 0;
        assert(!memcmp(&stored, &unchanged, sizeof(stored)));
        ntp_utc = 1792891800u; /* 2026-10-25 01:30 UTC, custom DST ended. */
        ticks += 1000; clock_app_tick();
        assert(strstr(sent," 10:00 00\r\n")); sent[0] = 0;
        assert(!memcmp(&stored, &unchanged, sizeof(stored)));
        strcpy(settings.timezone,TXW_CLOCK_DEFAULT_TZ);
        ntp_utc = 1792890000u; /* 2026-10-25 01:00 UTC, DST has ended. */
        ticks += 1000; clock_app_tick();
        assert(strstr(sent," 10:00 00\r\n")); sent[0] = 0;
        strcpy(settings.timezone,"GMT0"); ntp_utc = 1792832400u;
        ticks += 1000; clock_app_tick();
        assert(strstr(sent," 10:00 00\r\n")); sent[0] = 0;
        strcpy(settings.timezone,TXW_CLOCK_DEFAULT_TZ); ntp_utc = 1790858096u;
    }
    /* Web lease and NTP output coexist as complete, non-interleaved lines. */
    {
        unsigned minute;
        for (minute = 0; minute < 60; minute += 10) {
            char expected[32];
            settings.daily_hour = 23; settings.daily_minute = minute;
            ticks += 1000; clock_app_tick();
            snprintf(expected,sizeof(expected)," 23:%02u 00\r\n",minute);
            assert(strstr(sent,expected)); sent[0] = 0;
        }
        settings.daily_hour = 10; settings.daily_minute = 0;
    }
    clock_app_portal_client_seen();
    ticks += 1000; clock_app_tick();
    assert(strncmp(sent, "WIFIAPPING\r\n+TIME:", 18) == 0); sent[0] = 0;
    ticks += 15000; clock_app_tick();
    assert(strstr(sent, "WIFIAPPING") == NULL && strstr(sent, "WIFIEXIT") == NULL);
    assert(strncmp(sent, "+TIME:", 6) == 0); sent[0] = 0;
    clock_app_settings_copy(&candidate);
    before = ticks; assert(clock_app_save_settings(&candidate)); assert(ticks == before);
    clock_app_disconnected(); clock_app_tick(); expect("");
    fail_save = 1; assert(!clock_app_save_settings(&candidate)); fail_save = 0;
    clock_app_command(TXW_HC32_COMMAND_WIFI_RESET); expect("");
    ticks += 100; clock_app_command(TXW_HC32_COMMAND_WIFI_RESET); expect("");
    ticks += 1200; clock_app_command(TXW_HC32_COMMAND_WIFI_RESET); expect("WIFIRESET OK\r\n");
    ticks += 500; clock_app_tick(); expect("");
    assert(clock_app_is_portal() && !stored.ssid[0]);
    clock_app_command(TXW_HC32_COMMAND_WIFI_ID); expect("WIFIIDNC\r\n");
    /* Cached settings boot never performs PSK derivation/yields. */
    stored = candidate; have_stored = 1; before = ticks; clock_app_prepare();
    assert(ticks == before && !clock_app_is_portal());
    puts("App integration: AP/STA web keepalive, lease expiry/renewal, simultaneous TIME, names, early/repeated UART commands, save/reuse, mode switches, reset and cached-key boot passed");
    return 0;
}
