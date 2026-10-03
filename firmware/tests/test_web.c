// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
/* Exercise actual HTTP handlers, not a duplicate protocol implementation. */
#define CLOCK_WEB_TEST 1
#include "../iot_sdk_work/clock_project/clock_web.c"
#include <assert.h>
static struct txw_clock_settings saved;
static char output[40000];
static const char *input;
static int portal = 1, saves, resets, seen, scan_start, fail_save;
static uint32 ticks;
int send(int fd, const void *bytes, size_t count, int flags) {
    size_t len = strlen(output); (void)fd; (void)flags;
    assert(len + count < sizeof(output)); memcpy(output + len, bytes, count); output[len+count] = 0; return count;
}
int recv(int fd, void *bytes, size_t count, int flags) {
    size_t available = strlen(input); (void)fd; (void)flags;
    if (count > 17) count = 17; /* Fragment headers AND body. */
    if (count > available) count = available;
    memcpy(bytes, input, count); input += count; return count;
}
int setsockopt(int fd, int level, int option, const void *v, size_t size) {
    const struct timeval *t = v; (void)fd; (void)level; (void)option;
    assert(size == sizeof(*t) && t->tv_sec == 3); return 0;
}
ip_addr_t lwip_netif_get_ip2(const char *n) { ip_addr_t p; (void)n; p.addr = 0x0104a8c0u; return p; }
uint32 os_jiffies(void) { return ticks; }
uint32 os_jiffies_to_msecs(uint32 t) { return t; }
int ieee80211_scan(uint8 m, uint8 start, struct ieee80211_scandata *p) {
    (void)m; if (start) { assert(p && p->scan_cnt == 1); ++scan_start; } return 0;
}
int ieee80211_get_bsslist(struct hgic_bss_info *bss, int count) {
    assert(count == 16); strcpy((char *)bss[0].ssid, "Test\"network"); bss[0].signal = -42; return 1;
}
void clock_app_portal_client_seen(void) { ++seen; }
const char *clock_app_hostname(void) { return "WiFi-Clock-ABCDEF"; }
int clock_app_is_portal(void) { return portal; }
void clock_app_settings_copy(struct txw_clock_settings *s) { *s = saved; }
int clock_app_save_settings(const struct txw_clock_settings *s) { if (fail_save) return 0; saved = *s; ++saves; return 1; }
int clock_app_reset_settings(void) { ++resets; txw_clock_settings_defaults(&saved); return 1; }
static int time_synced;
int clock_ntp_fresh(void) { return time_synced; }
int clock_ntp_utc_now(uint32 *out) { *out = 1790858096u; return time_synced; }
void clock_ntp_dns_status(unsigned slot, uint32 *address, int *state, int *error) {
    *address = slot ? 0 : 0x0200a8c0u; *state = slot ? 3 : 2; *error = slot ? -1 : 0;
}
static void request(const char *method, const char *path, const char *headers, const char *body) {
    char http[4096];
    snprintf(http, sizeof(http), "%s %s HTTP/1.1\r\nHost: 192.168.4.1\r\n%sContent-Length: %u\r\n\r\n%s",
             method,path,headers,(unsigned)strlen(body),body);
    input = http; output[0] = 0; service_client(1);
}
int main(void) {
    char form[900]; unsigned movement, parking;
    const char *auth = "X-Clock-Token: 0123456789abcdef0123456789abcdef\r\nContent-Type: application/x-www-form-urlencoded\r\n";
    strcpy(csrf_token, "0123456789abcdef0123456789abcdef");
    txw_clock_settings_defaults(&saved);
    strcpy(saved.ssid, "Test\"network"); strcpy(saved.password, "secretpassword");
    request("GET", "/", "", ""); assert(strstr(output,"WiFi-Clock Setup") && strstr(output,"Hold&amp;Start") && strstr(output,"Save settings"));
    request("GET", "/api/v1/config", "", "");
    assert(strstr(output,"Test\\\"network") && strstr(output,"\"nightParking\":\"off\"") && !strstr(output,"secretpassword"));
    request("GET", "/api/v1/status", "", ""); assert(strstr(output,"\"timeSynced\":false"));
    assert(strstr(output,"\"nextDst\":null"));
    assert(strstr(output,"\"primaryDns\":{\"state\":2,\"error\":0,\"address\":\"192.168.0.2\"}"));
    assert(strstr(output,"\"secondaryDns\":{\"state\":3,\"error\":-1,\"address\":\"0.0.0.0\"}"));
    time_synced = 1;
    request("GET", "/api/v1/status", "", "");
    assert(strstr(output,"\"nextDst\":{\"utc\":1792890000,\"before\":60,\"after\":0}"));
    {
        struct txw_clock_settings before = saved;
        int old_saves = saves, old_resets = resets, old_seen = seen;
        request("POST", "/api/v1/dst-preview", "", "timezone=GMT0");
        assert(strstr(output,"403"));
        request("POST", "/api/v1/dst-preview", auth,
                "timezone=GMT0BST%2CM10.1.4%2F17%3A00%2CM11.1.0%2F2");
        assert(strstr(output,"200 OK") && strstr(output,"\"before\":0,\"after\":60"));
        request("POST", "/api/v1/dst-preview", auth, "timezone=GMT0");
        assert(strstr(output,"\"timeSynced\":true,\"nextDst\":null"));
        request("POST", "/api/v1/dst-preview", auth, "timezone=bad");
        assert(strstr(output,"400 Bad Request"));
        request("POST", "/api/v1/dst-preview", auth, "");
        assert(strstr(output,"400 Bad Request"));
        time_synced = 0;
        request("POST", "/api/v1/dst-preview", auth, "timezone=GMT0");
        assert(strstr(output,"\"timeSynced\":false,\"nextDst\":null"));
        time_synced = 1;
        assert(!memcmp(&before, &saved, sizeof(saved)) && saves == old_saves &&
               resets == old_resets && seen == old_seen);
    }
    strcpy(saved.timezone,"GMT0");
    request("GET", "/api/v1/status", "", ""); assert(strstr(output,"\"nextDst\":null"));
    strcpy(saved.timezone,TXW_CLOCK_DEFAULT_TZ); time_synced = 0;
    request("POST", "/api/v1/factory-reset", "", ""); assert(strstr(output,"403") && !resets);
    for (movement = 0; movement < 4; ++movement) for (parking = 0; parking < 3; ++parking) {
        snprintf(form, sizeof(form), "ssid=Test%%22network&password=&ntpHost=&ntpBackupHost=&timezone=GMT0BST%%2CM3.5.0%%2F1%%2CM10.5.0%%2F2&syncHour=10&syncMinute=20&minute_hand=%s&second_hand=%s&night_parking=%s",
                 movement&1 ? "jump":"gradual", movement&2 ? "pause_at_12":"continuous", parking == 2 ? "on":parking == 1 ? "night":"off");
        request("POST", "/api/v1/config", auth, form);
        assert(strstr(output,"\"saved\":true") && saved.movement_mode == movement && saved.night_parking == parking);
        assert(saved.daily_minute == 20 && !strcmp(saved.password,"secretpassword") && !strcmp(saved.ntp_host,"pool.ntp.org"));
        assert(!strcmp(saved.ntp_backup_host,"time.cloudflare.com"));
        request("GET", "/api/v1/config", "", "");
        assert(strstr(output,parking == 2 ? "\"nightParking\":\"on\"":parking == 1 ? "\"nightParking\":\"night\"":"\"nightParking\":\"off\""));
    }
    assert(saves == 12);
    {
        unsigned hour, minute;
        for (hour = 0; hour < 24; ++hour) for (minute = 0; minute < 60; ++minute) {
            struct txw_clock_settings candidate = saved;
            snprintf(form,sizeof(form),"ssid=Test%%22network&password=&ntpHost=&ntpBackupHost=&timezone=GMT0&syncHour=%u&syncMinute=%u&minute_hand=gradual&second_hand=continuous&night_parking=off",hour,minute);
            assert(parse_form(form,&candidate) == (minute % 10u == 0u));
            if (minute % 10u == 0u) assert(candidate.daily_hour == hour && candidate.daily_minute == minute);
        }
    }
    {
        struct txw_clock_settings candidate = saved;
        unsigned index;
        for (index = 0; index < 2; ++index) {
            snprintf(form, sizeof(form), "ssid=Test%%22network&password=&ntpHost=%s&ntpBackupHost=%s&timezone=GMT0BST%%2CM3.5.0%%2F1%%2CM10.5.0%%2F2&syncHour=10&syncMinute=20&minute_hand=gradual&second_hand=continuous&night_parking=on",
                     index == 0 ? "https%%3A%%2F%%2Fbad" : "pool.ntp.org",
                     index == 1 ? "https%%3A%%2F%%2Fbad" : "time.cloudflare.com");
            assert(!parse_form(form, &candidate));
        }
        snprintf(form, sizeof(form), "ssid=Test%%22network&password=&ntpHost=192.168.0.2&ntpBackupHost=backup.example.net&timezone=GMT0BST%%2CM3.5.0%%2F1%%2CM10.5.0%%2F2&syncHour=10&syncMinute=20&minute_hand=gradual&second_hand=continuous&night_parking=on");
        assert(parse_form(form, &candidate));
        assert(!strcmp(candidate.ntp_host,"192.168.0.2") && !strcmp(candidate.ntp_backup_host,"backup.example.net"));
    }
    strcat(form,"&ssid=duplicate"); request("POST","/api/v1/config",auth,form); assert(strstr(output,"400") && saves == 12);
    request("POST","/api/v1/config",auth,"ssid=%00"); assert(strstr(output,"400") && saves == 12);
    request("POST","/api/v1/keepalive",auth,""); assert(strstr(output,"204") && seen == 2);
    request("GET","/api/v1/scan?start=1",auth,""); assert(scan_start == 1 && strstr(output,"\"scanning\":true"));
    clock_web_scan_complete(); request("GET","/api/v1/scan",auth,""); assert(strstr(output,"-42") && strstr(output,"Test\\\"network"));
    request("POST","/api/v1/factory-reset",auth,""); assert(resets == 1 && !saved.ssid[0]);
    request("POST","/api/v1/keepalive","Origin: http://evil.example\r\n", ""); assert(strstr(output,"403"));
    puts("HTTP execution: fragmented requests, embedded page, config privacy, twelve movement/saving combinations, password retention, validation, CSRF, NTP default, scan and reset passed");
    return 0;
}
