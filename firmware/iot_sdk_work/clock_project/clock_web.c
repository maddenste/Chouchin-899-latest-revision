// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#ifdef CLOCK_WEB_TEST
#include "web_test_hal.h"
#else
#include "sys_config.h"
#include "typesdef.h"
#include "osal/task.h"
#include "osal/sleep.h"
#include "osal/string.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "netif/ethernetif.h"
#include "syscfg.h"
#endif
#include "clock_web.h"
#include "clock_app.h"
#include "clock_ntp.h"
#include "clock_web_ui.h"
#include "../../clock_calendar.h"
#include <stdio.h>
#include <string.h>

#if !LWIP_SO_RCVTIMEO || !LWIP_SO_SNDTIMEO || LWIP_SO_SNDRCVTIMEO_NONSTANDARD
#error "Clock HTTP requires enabled timeval socket timeouts"
#endif

#define HTTP_REQUEST_MAX 3072u
#define HTTP_BODY_MAX 1024u

#ifndef CLOCK_WEB_TEST
static struct os_task web_task;
static struct os_task dns_task;
static uint8 web_started;
#endif
static char csrf_token[33];

static int send_all(int socket_fd, const char *text, size_t length)
{
    while (length != 0u) {
        int written = send(socket_fd, text, length, 0);
        if (written <= 0) return 0;
        text += written;
        length -= (size_t)written;
    }
    return 1;
}

static void send_text(int socket_fd, const char *text)
{
    send_all(socket_fd, text, strlen(text));
}

static void response(int socket_fd, const char *status,
                     const char *type, const char *body)
{
    char header[256];
    size_t length = strlen(body);
    int count = snprintf(header, sizeof(header),
         "HTTP/1.0 %s\r\nContent-Type: %s\r\nContent-Length: %u\r\n"
         "Cache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\n"
         "X-Frame-Options: DENY\r\nConnection: close\r\n\r\n",
         status, type, (unsigned)length);
    if (count > 0 && count < sizeof(header)) {
        send_all(socket_fd, header, (size_t)count);
        send_all(socket_fd, body, length);
    }
}

static int ascii_equal_n(const char *left, const char *right, size_t count)
{
    size_t i;
    for (i = 0; i < count; ++i) {
        char a = left[i], b = right[i];
        if (a >= 'A' && a <= 'Z') a += 'a' - 'A';
        if (b >= 'A' && b <= 'Z') b += 'a' - 'A';
        if (a != b) return 0;
    }
    return 1;
}

static int header_value(const char *request, const char *name,
                        char *value, size_t capacity)
{
    const char *line = strstr(request, "\r\n");
    size_t name_length = strlen(name);
    if (line == NULL || capacity == 0u) return 0;
    line += 2;
    while (line[0] != '\r' || line[1] != '\n') {
        const char *end = strstr(line, "\r\n");
        const char *begin;
        size_t length;
        if (end == NULL) return 0;
        if ((size_t)(end - line) <= name_length + 1u ||
            line[name_length] != ':' ||
            !ascii_equal_n(line, name, name_length)) {
            line = end + 2;
            continue;
        }
        begin = line + name_length + 1u;
        while (begin < end && *begin == ' ') ++begin;
        length = (size_t)(end - begin);
        if (length == 0u || length >= capacity) return 0;
        memcpy(value, begin, length);
        value[length] = '\0';
        return 1;
    }
    return 0;
}

static int hex_digit(char value)
{
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    return -1;
}

static int form_field(const char *body, const char *name,
                      char *value, size_t capacity)
{
    size_t name_length = strlen(name), used = 0u;
    const char *part = body;
    int found = 0;
    if (capacity == 0u) return 0;
    while (*part != '\0') {
        const char *end = strchr(part, '&');
        const char *cursor;
        if (end == NULL) end = part + strlen(part);
        if ((size_t)(end - part) >= name_length + 1u &&
            part[name_length] == '=' &&
            memcmp(part, name, name_length) == 0) {
            if (found) return 0; // Duplicate fields are ambiguous.
            found = 1;
            cursor = part + name_length + 1u;
            while (cursor < end) {
                unsigned char decoded = (unsigned char)*cursor++;
                if (decoded == '+') decoded = ' ';
                else if (decoded == '%') {
                    int high, low;
                    if (end - cursor < 2) return 0;
                    high = hex_digit(cursor[0]);
                    low = hex_digit(cursor[1]);
                    if (high < 0 || low < 0) return 0;
                    decoded = (unsigned char)(high * 16 + low);
                    cursor += 2;
                }
                if (decoded < 0x20u || decoded > 0x7eu ||
                    used + 1u >= capacity) return 0;
                value[used++] = (char)decoded;
            }
            value[used] = '\0';
        }
        if (*end == '\0') break;
        part = end + 1;
    }
    return found;
}

static int parse_number(const char *text, unsigned maximum, uint8 *output)
{
    unsigned value = 0u, count = 0u;
    if (text == NULL || !*text) return 0;
    while (*text) {
        if (*text < '0' || *text > '9' || count++ > 2u) return 0;
        value = value * 10u + (unsigned)(*text++ - '0');
    }
    if (value > maximum) return 0;
    *output = (uint8)value;
    return 1;
}

static int parse_form(const char *body, struct txw_clock_settings *candidate)
{
    char ssid[33] = {0}, password[65] = {0}, ntp[128] = {0}, zone[65] = {0};
    char ntp_backup[128] = {0};
    char hour[4] = {0}, minute[4] = {0}, hand[16] = {0};
    char second[20] = {0}, parking[8] = {0};
    if (!form_field(body, "ssid", ssid, sizeof(ssid)) ||
        !form_field(body, "password", password, sizeof(password)) ||
        !form_field(body, "ntpHost", ntp, sizeof(ntp)) ||
        !form_field(body, "ntpBackupHost", ntp_backup, sizeof(ntp_backup)) ||
        !form_field(body, "timezone", zone, sizeof(zone)) ||
        !form_field(body, "syncHour", hour, sizeof(hour)) ||
        !form_field(body, "syncMinute", minute, sizeof(minute)) ||
        !form_field(body, "minute_hand", hand, sizeof(hand)) ||
        !form_field(body, "second_hand", second, sizeof(second)) ||
        !form_field(body, "night_parking", parking, sizeof(parking))) return 0;
    if (!parse_number(hour, 23u, &candidate->daily_hour) ||
        !parse_number(minute, 59u, &candidate->daily_minute) ||
        candidate->daily_minute % 10u != 0u ||
        !txw_clock_timezone_supported(zone)) return 0;
    if (strcmp(hand, "gradual") == 0) candidate->movement_mode = 0u;
    else if (strcmp(hand, "jump") == 0) candidate->movement_mode = 1u;
    else return 0;
    if (strcmp(second, "continuous") == 0) {}
    else if (strcmp(second, "pause_at_12") == 0)
        candidate->movement_mode += 2u;
    else return 0;
    if (strcmp(parking, "on") == 0) candidate->night_parking = 2u;
    else if (strcmp(parking, "night") == 0) candidate->night_parking = 1u;
    else if (strcmp(parking, "off") == 0) candidate->night_parking = 0u;
    else return 0;
    if (!ntp[0]) strcpy(ntp, TXW_CLOCK_DEFAULT_NTP);
    if (!ntp_backup[0]) strcpy(ntp_backup, TXW_CLOCK_DEFAULT_NTP_BACKUP);
    if (password[0] == '\0' && strcmp(ssid, candidate->ssid) == 0) {
        // Blank password leaves the saved one unchanged for the same SSID.
    } else {
        memcpy(candidate->password, password, sizeof(password));
    }
    memcpy(candidate->ssid, ssid, sizeof(ssid));
    memcpy(candidate->ntp_host, ntp, sizeof(ntp));
    memcpy(candidate->ntp_backup_host, ntp_backup, sizeof(ntp_backup));
    memcpy(candidate->timezone, zone, sizeof(zone));
    return txw_clock_settings_values_valid(candidate) &&
           txw_clock_settings_credentials_valid(candidate);
}

/* Escape strings into bounded JSON; never expose passwords or cached keys. */
static void json_string(char *out, size_t capacity, const char *input)
{
    size_t used = 0;
    if (capacity == 0u) return;
    while (*input) {
        unsigned char next = (unsigned char)*input;
        size_t needed = next < 32u || next > 126u ? 6u :
                        (next == '"' || next == '\\' ? 2u : 1u);
        if (used + needed >= capacity) break;
        unsigned char c = (unsigned char)*input++;
        if (c == '"' || c == '\\') { out[used++] = '\\'; out[used++] = c; }
        else if (c < 32u || c > 126u) {
            snprintf(out + used, capacity - used, "\\u%04x", (unsigned)c); used += 6;
        } else out[used++] = c;
    }
    out[used] = 0;
}

static void send_page(int fd)
{
    response(fd, "200 OK", "text/html; charset=utf-8", clock_web_ui);
}

static void send_config(int fd)
{
    struct txw_clock_settings s;
    /* Validated hostnames contain no JSON escape characters. Keep the web
     * task's stack comfortably below its 10 KiB allocation. */
    char ssid[200], ntp[128], ntp_backup[128], tz[390], json[1600];
    clock_app_settings_copy(&s);
    json_string(ssid, sizeof(ssid), s.ssid);
    json_string(ntp, sizeof(ntp), s.ntp_host);
    json_string(ntp_backup, sizeof(ntp_backup), s.ntp_backup_host);
    json_string(tz, sizeof(tz), s.timezone);
    snprintf(json, sizeof(json),
        "{\"requestToken\":\"%s\",\"ssid\":\"%s\",\"hasPassword\":%s,"
        "\"ntpHost\":\"%s\",\"ntpBackupHost\":\"%s\",\"timezone\":\"%s\",\"syncHour\":%u,\"syncMinute\":%u,"
        "\"minuteHand\":\"%s\",\"secondHand\":\"%s\",\"nightParking\":\"%s\"}",
        csrf_token, ssid, s.password[0] ? "true" : "false", ntp, ntp_backup, tz,
        s.daily_hour, s.daily_minute,
        (s.movement_mode & 1u) ? "jump" : "gradual",
        (s.movement_mode & 2u) ? "pause_at_12" : "continuous",
        s.night_parking == 2u ? "on" : s.night_parking == 1u ? "night" : "off");
    response(fd, "200 OK", "application/json", json);
}

/* Read-only preview: never save settings, schedule a wake, or send UART. */
static void send_dst_preview(int fd, const char *body)
{
    char zone[65], json[180], dst[128] = "null";
    uint32 utc;
    struct txw_clock_dst_change change;
    int synced;
    if (!form_field(body, "timezone", zone, sizeof(zone)) ||
        !txw_clock_timezone_supported(zone)) {
        response(fd, "400 Bad Request", "application/json",
                 "{\"error\":\"Enter a valid POSIX rule to preview DST.\"}");
        return;
    }
    synced = clock_ntp_fresh() && clock_ntp_utc_now(&utc);
    if (synced && txw_clock_next_dst_change(utc, zone, &change))
        snprintf(dst, sizeof(dst), "{\"utc\":%lu,\"before\":%d,\"after\":%d}",
                 (unsigned long)change.utc_seconds, (int)change.before_minutes,
                 (int)change.after_minutes);
    snprintf(json, sizeof(json), "{\"timeSynced\":%s,\"nextDst\":%s}",
             synced ? "true" : "false", dst);
    response(fd, "200 OK", "application/json", json);
}

static void send_status(int fd)
{
    char ip[20], json[1200], zone[390], dst[128] = "null";
    uint32 dns_address[2];
    int dns_state[2], dns_error[2];
    struct txw_clock_settings saved;
    struct txw_clock_dst_change change;
    uint32 utc;
    ip_addr_t address = lwip_netif_get_ip2("w0");
    clock_app_settings_copy(&saved);
    clock_ntp_dns_status(0, &dns_address[0], &dns_state[0], &dns_error[0]);
    clock_ntp_dns_status(1, &dns_address[1], &dns_state[1], &dns_error[1]);
    json_string(zone, sizeof(zone), saved.timezone);
    if (clock_ntp_fresh() && clock_ntp_utc_now(&utc) &&
        txw_clock_next_dst_change(utc, saved.timezone, &change))
        snprintf(dst, sizeof(dst), "{\"utc\":%lu,\"before\":%d,\"after\":%d}",
                 (unsigned long)change.utc_seconds, (int)change.before_minutes,
                 (int)change.after_minutes);
    snprintf(ip, sizeof(ip), "%u.%u.%u.%u", IP2STR_N(address.addr));
    snprintf(json, sizeof(json),
        "{\"firmwareVersion\":\"WiFi Clock \\u00b7 v2.0\","
        "\"buildId\":\"TXW813 HC32-V15 R16\",\"mode\":\"%s\","
        "\"ip\":\"%s\",\"hostname\":\"%s\",\"connected\":%s,\"timeSynced\":%s,"
        "\"timezone\":\"%s\",\"nextDst\":%s,"
        "\"dnsServer\":\"%u.%u.%u.%u\","
        "\"primaryDns\":{\"state\":%d,\"error\":%d,\"address\":\"%u.%u.%u.%u\"},"
        "\"secondaryDns\":{\"state\":%d,\"error\":%d,\"address\":\"%u.%u.%u.%u\"}}",
        clock_app_is_portal() ? "portal" : "station", ip, clock_app_hostname(),
        sys_status.wifi_connected && sys_status.dhcpc_done ? "true" : "false",
        clock_ntp_fresh() ? "true" : "false", zone, dst,
        IP2STR_N(sys_status.dhcpc_result.dns1),
        dns_state[0], dns_error[0], IP2STR_N(dns_address[0]),
        dns_state[1], dns_error[1], IP2STR_N(dns_address[1]));
    response(fd, "200 OK", "application/json", json);
}

static volatile uint8 scan_running;
static uint32 scan_started_ms;
void clock_web_scan_complete(void) { scan_running = 0; }

static void send_scan(int fd, int start)
{
    /* Keep the vendor BSS cache bounded and do not scan while joining an AP. */
    /* One serial HTTP worker owns these buffers; avoid deep task-stack use. */
    static struct hgic_bss_info bss[16];
    static char json[4300];
    char ssid[33], escaped[200];
    int count, i, used = 0;
    uint32 now = os_jiffies_to_msecs(os_jiffies());
    if (scan_running && (uint32)(now - scan_started_ms) > 15000u) {
        ieee80211_scan(sys_cfgs.wifi_mode, 0, NULL);
        scan_running = 0;
    }
    if (start && !scan_running) {
        struct ieee80211_scandata params;
        if (!clock_app_is_portal() && !sys_status.wifi_connected) {
            response(fd, "409 Conflict", "application/json",
                     "{\"error\":\"Wi-Fi is connecting. Enter the network name manually.\"}");
            return;
        }
        memset(&params, 0, sizeof(params));
        params.chan_bitmap = 0xffffu; /* Vendor demo scan mask; country policy applies. */
        params.scan_time = 30; params.scan_cnt = 1;
        scan_started_ms = now;
        scan_running = 1;
        if (ieee80211_scan(sys_cfgs.wifi_mode, 1, &params) < 0) {
            scan_running = 0;
            response(fd, "503 Service Unavailable", "application/json",
                     "{\"error\":\"Network scan unavailable. Enter the network name manually.\"}");
            return;
        }
    }
    if (scan_running) {
        response(fd, "200 OK", "application/json", "{\"scanning\":true,\"networks\":[]}");
        return;
    }
    memset(bss, 0, sizeof(bss));
    /* SDK disassembly confirms list_size is an ENTRY COUNT, not byte size. */
    count = ieee80211_get_bsslist(bss, sizeof(bss) / sizeof(bss[0]));
    if (count < 0) count = 0;
    if (count > 16) count = 16;
    used = snprintf(json, sizeof(json), "{\"scanning\":false,\"networks\":[");
    for (i = 0; i < count; ++i) {
        memcpy(ssid, bss[i].ssid, 32); ssid[32] = 0;
        json_string(escaped, sizeof(escaped), ssid);
        used += snprintf(json + used, sizeof(json) - used,
                         "%s{\"ssid\":\"%s\",\"rssi\":%d}",
                         i ? "," : "", escaped, (int)bss[i].signal);
    }
    snprintf(json + used, sizeof(json) - used, "]}");
    response(fd, "200 OK", "application/json", json);
}

static void send_redirect(int socket_fd)
{
    send_text(socket_fd, "HTTP/1.0 302 Found\r\nLocation: http://192.168.4.1/\r\n"
             "Content-Length: 0\r\nConnection: close\r\n\r\n");
}

static void local_host(char *output, size_t capacity)
{
    ip_addr_t ip = lwip_netif_get_ip2("w0");
    snprintf(output, capacity, "%u.%u.%u.%u", IP2STR_N(ip.addr));
}

static void service_client(int socket_fd)
{
    char request[HTTP_REQUEST_MAX + 1u], host[80], origin[100];
    char allowed_host[20], content_length[16], token[40];
    char *header_end = NULL, *body;
    size_t used = 0u, body_length = 0u;
    struct timeval timeout = {3, 0};
    int received, is_post, is_get;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0 ||
        setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) != 0) return;
    while (used < HTTP_REQUEST_MAX) {
        received = recv(socket_fd, request + used, HTTP_REQUEST_MAX - used, 0);
        if (received <= 0) return;
        used += (size_t)received;
        request[used] = '\0';
        header_end = strstr(request, "\r\n\r\n");
        if (header_end != NULL) break;
    }
    if (header_end == NULL) return;
    is_post = strncmp(request, "POST ", 5) == 0;
    is_get = strncmp(request, "GET ", 4) == 0;
    if (!is_post && !is_get) {
        response(socket_fd, "405 Method Not Allowed", "text/plain", "Method not allowed");
        return;
    }
    body = header_end + 4;
    if (is_post) {
        if (!header_value(request, "Content-Length", content_length,
                          sizeof(content_length))) {
            /* Fetch with no body can legally omit Content-Length. */
            strcpy(content_length, "0");
        }
        {
            const char *digit = content_length;
            while (*digit) {
                if (*digit < '0' || *digit > '9') return;
                body_length = body_length * 10u + (size_t)(*digit++ - '0');
                if (body_length > HTTP_BODY_MAX) return;
            }
        }
        while (used - (size_t)(body - request) < body_length &&
               used < HTTP_REQUEST_MAX) {
            received = recv(socket_fd, request + used, HTTP_REQUEST_MAX - used, 0);
            if (received <= 0) return;
            used += (size_t)received;
        }
        if (used - (size_t)(body - request) < body_length) return;
        body[body_length] = '\0';
    }
    if (!header_value(request, "Host", host, sizeof(host))) return;
    {
        char *port = strchr(host, ':');
        if (port != NULL && strcmp(port, ":80") == 0) *port = '\0';
    }
    local_host(allowed_host, sizeof(allowed_host));
    if (strcmp(host, allowed_host) != 0) {
        if (strcmp(allowed_host, "192.168.4.1") == 0 && is_get) send_redirect(socket_fd);
        else response(socket_fd, "403 Forbidden", "text/plain", "Open the clock by IP address");
        return;
    }
    if (header_value(request, "Origin", origin, sizeof(origin))) {
        char expected[100];
        snprintf(expected, sizeof(expected), "http://%s", allowed_host);
        if (strcmp(origin, expected) != 0) {
            response(socket_fd, "403 Forbidden", "text/plain", "Invalid origin");
            return;
        }
    }
    {
        char path[128];
        const char *begin = request + (is_post ? 5 : 4);
        const char *end = strchr(begin, ' ');
        size_t length;
        int authenticated;
        if (!end || (length = (size_t)(end - begin)) >= sizeof(path)) {
            response(socket_fd, "400 Bad Request", "application/json", "{\"error\":\"Invalid path\"}");
            return;
        }
        memcpy(path, begin, length); path[length] = 0;
        authenticated = header_value(request, "X-Clock-Token", token, sizeof(token)) &&
                        strcmp(token, csrf_token) == 0;
        if (!authenticated && is_post)
            authenticated = form_field(body, "token", token, sizeof(token)) &&
                            strcmp(token, csrf_token) == 0;
        if (is_get) {
            if (!strcmp(path, "/")) { clock_app_portal_client_seen(); send_page(socket_fd); }
            else if (!strcmp(path, "/api/v1/config")) send_config(socket_fd);
            else if (!strcmp(path, "/api/v1/status")) send_status(socket_fd);
            else if (!strcmp(path, "/api/v1/scan")) send_scan(socket_fd, 0);
            else if (!strcmp(path, "/api/v1/scan?start=1") && authenticated) send_scan(socket_fd, 1);
            else if (clock_app_is_portal() && strncmp(path, "/api/", 5)) send_redirect(socket_fd);
            else response(socket_fd, "404 Not Found", "application/json", "{\"error\":\"Not found\"}");
            return;
        }
        if (!authenticated) {
            response(socket_fd, "403 Forbidden", "application/json", "{\"error\":\"Expired request token; reload the page\"}");
            return;
        }
        if (!strcmp(path, "/api/v1/dst-preview")) {
            send_dst_preview(socket_fd, body);
        } else if (!strcmp(path, "/api/v1/config")) {
            struct txw_clock_settings candidate;
            char type[80];
            if (!header_value(request, "Content-Type", type, sizeof(type)) ||
                strcmp(type, "application/x-www-form-urlencoded") != 0) {
                response(socket_fd, "415 Unsupported Media Type", "application/json", "{\"error\":\"Form encoding required\"}");
                return;
            }
            clock_app_settings_copy(&candidate);
            if (!parse_form(body, &candidate))
                response(socket_fd, "400 Bad Request", "application/json",
                         "{\"error\":\"Invalid settings: check SSID, password (8-63 characters), NTP host and POSIX M rules\"}");
            else if (!clock_app_save_settings(&candidate))
                response(socket_fd, "503 Service Unavailable", "application/json", "{\"error\":\"Settings were not saved\"}");
            else response(socket_fd, "200 OK", "application/json", "{\"saved\":true}");
        } else if (!strcmp(path, "/api/v1/factory-reset")) {
            if (clock_app_reset_settings()) response(socket_fd, "200 OK", "application/json", "{\"reset\":true}");
            else response(socket_fd, "503 Service Unavailable", "application/json", "{\"error\":\"Reset failed\"}");
        } else if (!strcmp(path, "/api/v1/keepalive")) {
            clock_app_portal_client_seen();
            response(socket_fd, "204 No Content", "application/json", "");
        } else if (!strcmp(path, "/api/v1/session/close")) {
            /* The session lease expires naturally. A closing tab must not end
               another tab's active session. Never reset the HC32 or network. */
            response(socket_fd, "204 No Content", "application/json", "");
        } else response(socket_fd, "404 Not Found", "application/json", "{\"error\":\"Not found\"}");
    }
}

#ifndef CLOCK_WEB_TEST
static void clock_web_worker(void *arg)
{
    int listener, client, enabled = 1;
    struct sockaddr_in address;
    (void)arg;
    for (;;) {
        listener = socket(AF_INET, SOCK_STREAM, 0);
        if (listener < 0) { os_sleep_ms(1000); continue; }
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(80);
        address.sin_addr.s_addr = INADDR_ANY;
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));
        if (bind(listener, (struct sockaddr *)&address, sizeof(address)) != 0 ||
            listen(listener, 2) != 0) {
            close(listener);
            os_sleep_ms(1000);
            continue;
        }
        for (;;) {
            client = accept(listener, NULL, NULL);
            if (client < 0) break;
            service_client(client);
            close(client);
        }
        close(listener);
        os_sleep_ms(1000);
    }
}

static void clock_dns_worker(void *arg)
{
    int dns_socket, received;
    uint8 packet[512];
    struct sockaddr_in bind_address, peer;
    struct timeval timeout = {1, 0};
    socklen_t peer_length;
    (void)arg;
    for (;;) {
        dns_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (dns_socket < 0) { os_sleep_ms(1000); continue; }
        memset(&bind_address, 0, sizeof(bind_address));
        bind_address.sin_family = AF_INET;
        bind_address.sin_port = htons(53);
        bind_address.sin_addr.s_addr = INADDR_ANY;
        setsockopt(dns_socket, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout));
        if (bind(dns_socket, (struct sockaddr *)&bind_address,
                 sizeof(bind_address)) != 0) {
            close(dns_socket);
            os_sleep_ms(1000);
            continue;
        }
        for (;;) {
            size_t question_end, label_at;
            uint16 qtype, qclass;
            peer_length = sizeof(peer);
            received = recvfrom(dns_socket, packet, sizeof(packet) - 16u,
                                0, (struct sockaddr *)&peer, &peer_length);
            if (received < 18 || !clock_app_is_portal()) continue;
            if (packet[2] & 0x80u || packet[4] != 0u || packet[5] != 1u) continue;
            label_at = 12u;
            while (label_at < (size_t)received) {
                uint8 label_length = packet[label_at];
                if (label_length == 0u) { ++label_at; break; }
                if (label_length > 63u ||
                    label_at + 1u + label_length >= (size_t)received) break;
                label_at += 1u + label_length;
            }
            if (label_at + 4u > (size_t)received ||
                packet[label_at - 1u] != 0u) continue;
            question_end = label_at + 4u;
            qtype = ((uint16)packet[label_at] << 8u) | packet[label_at + 1u];
            qclass = ((uint16)packet[label_at + 2u] << 8u) | packet[label_at + 3u];
            if (qclass != 1u) continue;
            packet[2] = 0x81u;
            packet[3] = 0x80u;
            packet[6] = 0u;
            packet[7] = qtype == 1u ? 1u : 0u;
            packet[8] = packet[9] = packet[10] = packet[11] = 0u;
            if (qtype == 1u) {
                static const uint8 answer[] =
                    {0xc0,0x0c, 0x00,0x01, 0x00,0x01, 0,0,0,0, 0,4,
                     192,168,4,1};
                memcpy(packet + question_end, answer, sizeof(answer));
                question_end += sizeof(answer);
            }
            sendto(dns_socket, packet, question_end, 0,
                   (struct sockaddr *)&peer, peer_length);
        }
    }
}

void clock_web_start(void)
{
    uint8 random_bytes[16];
    static const char hex[] = "0123456789abcdef";
    unsigned i;
    if (web_started) return;
    web_started = 1u;
    os_random_bytes(random_bytes, sizeof(random_bytes));
    for (i = 0; i < sizeof(random_bytes); ++i) {
        csrf_token[2u * i] = hex[random_bytes[i] >> 4u];
        csrf_token[2u * i + 1u] = hex[random_bytes[i] & 15u];
    }
    csrf_token[32] = '\0';
    OS_TASK_INIT("clock-web", &web_task, clock_web_worker, NULL,
                 OS_TASK_PRIORITY_NORMAL, 10240);
    OS_TASK_INIT("clock-dns", &dns_task, clock_dns_worker, NULL,
                 OS_TASK_PRIORITY_NORMAL, 3072);
}
#endif
