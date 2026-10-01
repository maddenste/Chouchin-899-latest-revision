#ifndef BACKEND_TEST_HAL_H
#define BACKEND_TEST_HAL_H
#include "typesdef.h"
#include <stddef.h>
#define RET_OK 0
#define WIFI_MODE_STA 1
#define WIFI_MODE_AP 2
#define WPA_KEY_MGMT_PSK 2
#define WPA_KEY_MGMT_NONE 0
#define HG_WIFI0_DEVID 1
struct netdev { int dummy; };
struct os_mutex { int locked; };
typedef struct { uint32 addr; } ip_addr_t;
struct dhcpd_param { uint32 start_ip, end_ip, netmask, lease_time, dns1, dns2, router; };
struct test_config {
    uint8 wifi_mode, dhcpc_en, dhcpd_en, key_mgmt;
    char ssid[33], passwd[33];
    uint8 psk[32], mac[6], bssid[6], station_channel, bss_data[16], channel, wifi_hwmode;
    uint32 ipaddr, netmask, gw_ip, dhcpd_startip, dhcpd_endip, dhcpd_lease_time;
};
struct test_status { uint8 channel, dhcpc_done, wifi_connected; };
extern struct test_config sys_cfgs;
extern struct test_status sys_status;
uint32 os_jiffies(void);
uint32 os_jiffies_to_msecs(uint32 tick);
void os_sleep_ms(uint32 delay);
void mcu_watchdog_feed(void);
int os_mutex_init(struct os_mutex *m);
int os_mutex_lock(struct os_mutex *m, int timeout);
int os_mutex_unlock(struct os_mutex *m);
void *dev_get(int id);
void sysctrl_efuse_mac_addr_calc(uint8 *mac);
void dhcpd_start(const char *name, struct dhcpd_param *params);
void dhcpd_stop(const char *name);
void ieee80211_iface_stop(int mode);
int ieee80211_iface_start(int mode);
void netdev_set_wifi_mode(struct netdev *dev, int mode);
void lwip_netif_set_ip2(const char *name, ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw);
void lwip_netif_set_dhcp2(const char *name, int enabled);
ip_addr_t lwip_netif_get_ip2(const char *name);
#endif
