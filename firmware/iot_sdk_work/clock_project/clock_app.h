#ifndef CLOCK_APP_H
#define CLOCK_APP_H

#include "typesdef.h"
#include "clock_uart.h"
#include "../../clock_settings.h"

void clock_app_prepare(void);
void clock_app_network_ready(void);
void clock_app_command(enum txw_hc32_command command);
void clock_app_tick(void);
void clock_app_dhcp_ready(void);
void clock_app_disconnected(void);
void clock_app_portal_client_seen(void);
int clock_app_is_portal(void);
const char *clock_app_hostname(void);
void clock_app_settings_copy(struct txw_clock_settings *out);
int clock_app_save_settings(const struct txw_clock_settings *candidate);
int clock_app_reset_settings(void);

#endif
