#ifdef CLOCK_BACKEND_TEST
#include "backend_test_hal.h"
#else
#include "sys_config.h"
#include "typesdef.h"
#include "dev.h"
#include "devid.h"
#include "osal/string.h"
#include "osal/sleep.h"
#include "osal/mutex.h"
#include "hal/netdev.h"
#include "lib/umac/ieee80211.h"
#include "lib/net/dhcpd/dhcpd.h"
#include "lwip/ip_addr.h"
#include "netif/ethernetif.h"
#include "syscfg.h"
#endif
#include "clock_app.h"
#include "clock_storage.h"
#include "clock_ntp.h"
#include "../../clock_calendar.h"
#include "../../clock_psk.h"
#include <stdio.h>
#include <string.h>

#define AP_IP_ADDR 0x0104a8c0u /* 192.168.4.1, lwIP host byte order */
#define AP_NETMASK 0x00ffffffu
#define AP_START_ADDR 0x0204a8c0u
#define AP_END_ADDR 0x6404a8c0u

static struct txw_clock_settings settings;
static char clock_hostname[32];
static struct txw_hc32_time_stream time_stream;
static uint8 configured;
static uint8 network_initialized;
static uint8 reset_armed;
static uint32 reset_at_ms;
static volatile uint8 station_switch_pending;
static uint32 station_switch_at_ms;
static volatile uint8 portal_switch_pending;
static uint32 portal_switch_at_ms;
static uint8 web_client_seen;
static uint32 web_client_last_ms;
static uint32 portal_started_ms;
static uint32 web_last_line_ms;
static struct os_mutex settings_lock;
static enum txw_hc32_command pending_mode;

extern int32 wificfg_flush(uint8 ifidx);
static int reset_settings_locked(void);

static uint32 now_ms(void)
{
    return (uint32)os_jiffies_to_msecs(os_jiffies());
}

static void configure_station(void)
{
    sys_cfgs.wifi_mode = WIFI_MODE_STA;
    sys_cfgs.dhcpc_en = 1u;
    sys_cfgs.dhcpd_en = 0u;
    sys_cfgs.key_mgmt = settings.password[0] ? WPA_KEY_MGMT_PSK :
                                                  WPA_KEY_MGMT_NONE;
    memcpy(sys_cfgs.ssid, settings.ssid, sizeof(sys_cfgs.ssid));
    /* The SDK field is 33 bytes, while settings.password is 65 bytes.
     * Hash the complete passphrase, but never copy past the SDK field. */
    memset(sys_cfgs.passwd, 0, sizeof(sys_cfgs.passwd));
    memcpy(sys_cfgs.passwd, settings.password, sizeof(sys_cfgs.passwd) - 1u);
    memcpy(sys_cfgs.psk, settings.psk, sizeof(sys_cfgs.psk));
    /* Do not reuse a previous network's cached BSSID/channel after a save. */
    sys_cfgs.station_channel = 0u;
    memset(sys_cfgs.bssid, 0, sizeof(sys_cfgs.bssid));
    memset(&sys_cfgs.bss_data, 0, sizeof(sys_cfgs.bss_data));
    sys_cfgs.ipaddr = sys_cfgs.netmask = sys_cfgs.gw_ip = 0u;
    sys_status.channel = 0u;
}

static void configure_portal(void)
{
    sys_cfgs.wifi_mode = WIFI_MODE_AP;
    sys_cfgs.dhcpc_en = 0u;
    sys_cfgs.dhcpd_en = 1u;
    sys_cfgs.key_mgmt = WPA_KEY_MGMT_NONE;
    memset(sys_cfgs.ssid, 0, sizeof(sys_cfgs.ssid));
    snprintf((char *)sys_cfgs.ssid, sizeof(sys_cfgs.ssid), "WiFi-Clock-Setup");
    memset(sys_cfgs.passwd, 0, sizeof(sys_cfgs.passwd));
    memset(sys_cfgs.psk, 0, sizeof(sys_cfgs.psk));
    sys_cfgs.channel = sys_status.channel = 1u;
    sys_cfgs.ipaddr = sys_cfgs.gw_ip = AP_IP_ADDR;
    sys_cfgs.netmask = AP_NETMASK;
    sys_cfgs.dhcpd_startip = AP_START_ADDR;
    sys_cfgs.dhcpd_endip = AP_END_ADDR;
    sys_cfgs.dhcpd_lease_time = 3600u;
}

static void portal_dhcp_start(void)
{
    struct dhcpd_param params;
    memset(&params, 0, sizeof(params));
    params.start_ip = AP_START_ADDR;
    params.end_ip = AP_END_ADDR;
    params.netmask = AP_NETMASK;
    params.lease_time = 3600u;
    params.dns1 = AP_IP_ADDR;
    params.dns2 = AP_IP_ADDR;
    params.router = AP_IP_ADDR;
    dhcpd_start("w0", &params);
}

void clock_app_prepare(void)
{
    os_mutex_init(&settings_lock);
    web_client_seen = 0u;
    web_client_last_ms = web_last_line_ms = 0u;
    configured = clock_storage_load(&settings) &&
                 txw_clock_settings_credentials_valid(&settings) &&
                 (!settings.password[0] || settings.psk_ready);
    // syscfg_init is intentionally absent: preserve 0x1FE000/0x1FF000.
    memset(&sys_cfgs, 0, sizeof(sys_cfgs));
    sysctrl_efuse_mac_addr_calc(sys_cfgs.mac);
    snprintf(clock_hostname, sizeof(clock_hostname), "WiFi-Clock-%02X%02X%02X",
             (unsigned)sys_cfgs.mac[3], (unsigned)sys_cfgs.mac[4],
             (unsigned)sys_cfgs.mac[5]);
    sys_cfgs.wifi_hwmode = 0u;
    txw_hc32_time_stream_init(&time_stream);
    if (configured) configure_station();
    else {
        configure_portal();
        portal_started_ms = now_ms();
    }
}

void clock_app_network_ready(void)
{
    network_initialized = 1u;
    if (sys_cfgs.wifi_mode == WIFI_MODE_AP) portal_dhcp_start();
    else if (lwip_netif_get_ip2("w0").addr != 0u) clock_app_dhcp_ready();
}

static int switch_to_portal(void)
{
    struct netdev *netdev = (struct netdev *)dev_get(HG_WIFI0_DEVID);
    ip_addr_t ip, mask, gateway;
    if (!network_initialized || netdev == NULL) return 0;
    if (sys_cfgs.wifi_mode == WIFI_MODE_AP) return 1;
    lwip_netif_set_dhcp2("w0", 0);
    sys_status.dhcpc_done = sys_status.wifi_connected = 0u;
    clock_ntp_disconnected();
    ieee80211_iface_stop(WIFI_MODE_STA);
    configure_portal();
    wificfg_flush(WIFI_MODE_AP);
    netdev_set_wifi_mode(netdev, WIFI_MODE_AP);
    if (ieee80211_iface_start(WIFI_MODE_AP) != RET_OK) return 0;
    ip.addr = gateway.addr = AP_IP_ADDR;
    mask.addr = AP_NETMASK;
    lwip_netif_set_ip2("w0", &ip, &mask, &gateway);
    portal_dhcp_start();
    web_client_seen = 0u;
    portal_started_ms = now_ms();
    web_last_line_ms = portal_started_ms;
    return 1;
}

static int switch_to_station(int restart_existing)
{
    struct netdev *netdev = (struct netdev *)dev_get(HG_WIFI0_DEVID);
    ip_addr_t empty;
    if (!network_initialized || netdev == NULL || !configured) return 0;
    if (sys_cfgs.wifi_mode == WIFI_MODE_STA && !restart_existing) return 1;
    // Saving new Wi-Fi credentials while already in STA mode must restart
    // the interface; returning early would leave the old network active.
    clock_ntp_disconnected();
    if (sys_cfgs.wifi_mode == WIFI_MODE_AP) {
        dhcpd_stop(NULL);
        ieee80211_iface_stop(WIFI_MODE_AP);
    } else {
        ieee80211_iface_stop(WIFI_MODE_STA);
    }
    configure_station();
    sys_status.dhcpc_done = sys_status.wifi_connected = 0u;
    wificfg_flush(WIFI_MODE_STA);
    netdev_set_wifi_mode(netdev, WIFI_MODE_STA);
    if (ieee80211_iface_start(WIFI_MODE_STA) != RET_OK) return 0;
    web_client_seen = 0u;
    empty.addr = 0u;
    lwip_netif_set_ip2("w0", &empty, &empty, &empty);
    lwip_netif_set_dhcp2("w0", 1);
    return 1;
}

static void send_literal(const char *line)
{
    clock_uart_send(line, (uint32)strlen(line));
}

void clock_app_command(enum txw_hc32_command command)
{
    os_mutex_lock(&settings_lock, -1);
    switch (command) {
    case TXW_HC32_COMMAND_WIFI_ID:
        send_literal(configured ? txw_hc32_wifi_id_ok :
                                  txw_hc32_wifi_id_no_credentials);
        break;
    case TXW_HC32_COMMAND_WIFI_AP:
        if (!network_initialized) pending_mode = command;
        else if (switch_to_portal()) send_literal(txw_hc32_wifi_ap_ok);
        break;
    case TXW_HC32_COMMAND_WIFI_STA:
        if (!network_initialized) pending_mode = command;
        else if (switch_to_station(0)) send_literal(txw_hc32_wifi_sta_ok);
        break;
    case TXW_HC32_COMMAND_WIFI_RESET:
        // Require the second HC32 request in the observed 1.3 s sequence;
        // do not erase credentials on a single garbled UART record.
        if (!reset_armed || (uint32)(now_ms() - reset_at_ms) > 4000u) {
            reset_armed = 1u;
            reset_at_ms = now_ms();
        } else if ((uint32)(now_ms() - reset_at_ms) >= 750u) {
            if (reset_settings_locked()) {
                reset_armed = 0u;
                send_literal(txw_hc32_wifi_reset_ok);
            }
        }
        break;
    default:
        break;
    }
    os_mutex_unlock(&settings_lock);
}

void clock_app_dhcp_ready(void)
{
    os_mutex_lock(&settings_lock, -1);
    if (configured && sys_cfgs.wifi_mode == WIFI_MODE_STA)
        clock_ntp_begin(settings.ntp_host, settings.ntp_backup_host);
    os_mutex_unlock(&settings_lock);
}

void clock_app_disconnected(void)
{
    clock_ntp_disconnected();
}

void clock_app_portal_client_seen(void)
{
    /* A page at the station IP needs the same HC32 power lease as the AP.
     * Keep the legacy API name, but protect lease state in both modes. */
    os_mutex_lock(&settings_lock, -1);
    web_client_last_ms = now_ms();
    web_client_seen = 1u;
    os_mutex_unlock(&settings_lock);
}

int clock_app_is_portal(void)
{
    return sys_cfgs.wifi_mode == WIFI_MODE_AP;
}

const char *clock_app_hostname(void)
{
    /* Stable storage: lwIP retains this pointer for DHCP option 12. */
    return clock_hostname;
}

void clock_app_tick(void)
{
    struct txw_hc32_time local;
    uint32 utc;
    char line[TXW_HC32_TIME_LINE_CAPACITY];
    size_t length;
    /* Credential derivation happens in the web task, outside this lock. */
    os_mutex_lock(&settings_lock, -1);
    if (network_initialized && pending_mode != TXW_HC32_COMMAND_NONE) {
        if (pending_mode == TXW_HC32_COMMAND_WIFI_AP && switch_to_portal())
            send_literal(txw_hc32_wifi_ap_ok);
        else if (pending_mode == TXW_HC32_COMMAND_WIFI_STA && switch_to_station(0))
            send_literal(txw_hc32_wifi_sta_ok);
        pending_mode = TXW_HC32_COMMAND_NONE;
    }
    if (station_switch_pending &&
        (int32)(now_ms() - station_switch_at_ms) >= 0) {
        station_switch_pending = 0u;
        if (switch_to_station(1)) send_literal(txw_hc32_wifi_app_ok);
    }
    if (portal_switch_pending &&
        (int32)(now_ms() - portal_switch_at_ms) >= 0) {
        portal_switch_pending = 0u;
        switch_to_portal();
    }
    if (network_initialized &&
        (uint32)(now_ms() - web_last_line_ms) >= 1000u) {
        if (web_client_seen &&
            (uint32)(now_ms() - web_client_last_ms) < 15000u) {
            web_last_line_ms = now_ms();
            send_literal(txw_hc32_wifi_pairing);
        } else if (sys_cfgs.wifi_mode == WIFI_MODE_AP &&
                   (uint32)(now_ms() - portal_started_ms) >= 4000u) {
            web_last_line_ms = now_ms();
            send_literal(txw_hc32_wifi_exit);
        }
        /* Expired station sessions become quiet, not WIFIEXIT: normal
         * station wake/+TIME behaviour must remain under HC32 control. */
    }
    if (!txw_hc32_time_due(&time_stream, now_ms(), clock_ntp_fresh()) ||
        !clock_ntp_utc_now(&utc)) goto done;
    txw_hc32_time_init(&local);
    if (!txw_clock_local_from_unix(utc, settings.timezone, &local)) goto done;
    local.daily_update_hour = settings.daily_hour;
    local.daily_update_minute = settings.daily_minute;
    txw_clock_next_wake(utc, settings.timezone, settings.daily_hour,
                        settings.daily_minute, &local.daily_update_hour,
                        &local.daily_update_minute);
    local.movement_mode = settings.movement_mode;
    local.night_parking = settings.night_parking;
    length = txw_hc32_format_time_for_clock(line, sizeof(line), &local);
    if (length != 0u) clock_uart_send(line, (uint32)length);
done:
    os_mutex_unlock(&settings_lock);
}

void clock_app_settings_copy(struct txw_clock_settings *out)
{
    os_mutex_lock(&settings_lock, -1);
    *out = settings;
    os_mutex_unlock(&settings_lock);
}

static void psk_yield(void)
{
    mcu_watchdog_feed();
    os_sleep_ms(1);
}

int clock_app_save_settings(const struct txw_clock_settings *candidate)
{
    struct txw_clock_settings copy;
    struct txw_clock_settings saved;
    if (candidate == NULL) return 0;
    copy = *candidate;
    if (!txw_clock_settings_values_valid(&copy) ||
        !txw_clock_settings_credentials_valid(&copy)) return 0;
    clock_app_settings_copy(&saved);
    memset(copy.psk, 0, sizeof(copy.psk));
    copy.psk_ready = 0u;
    if (copy.password[0]) {
        if (saved.psk_ready && strcmp(saved.ssid, copy.ssid) == 0 &&
            strcmp(saved.password, copy.password) == 0)
            memcpy(copy.psk, saved.psk, sizeof(copy.psk));
        else if (!clock_psk_derive(copy.ssid, copy.password, copy.psk, psk_yield)) return 0;
        copy.psk_ready = 1u;
    }
    os_mutex_lock(&settings_lock, -1);
    if (settings.generation != saved.generation || !clock_storage_save(&copy)) {
        os_mutex_unlock(&settings_lock);
        return 0;
    }
    settings = copy;
    configured = 1u;
    portal_switch_pending = 0u;
    // Let the HTTP response leave before tearing down the setup AP. The HC32
    // sends WIFISTA only after WIFIAPPOK, so send that once STA is started.
    station_switch_at_ms = now_ms() + 500u;
    station_switch_pending = 1u;
    os_mutex_unlock(&settings_lock);
    return 1;
}

static int reset_settings_locked(void)
{
    if (!clock_storage_reset(&settings)) return 0;
    configured = 0u;
    station_switch_pending = 0u;
    pending_mode = TXW_HC32_COMMAND_NONE;
    clock_ntp_disconnected();
    if (sys_cfgs.wifi_mode != WIFI_MODE_AP) {
        portal_switch_at_ms = now_ms() + 500u;
        portal_switch_pending = 1u;
    }
    return 1;
}

int clock_app_reset_settings(void)
{
    int result;
    os_mutex_lock(&settings_lock, -1);
    result = reset_settings_locked();
    os_mutex_unlock(&settings_lock);
    return result;
}
