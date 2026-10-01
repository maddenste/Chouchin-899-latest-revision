#ifndef CLOCK_NTP_H
#define CLOCK_NTP_H

#include "typesdef.h"

void clock_ntp_begin(const char *host, const char *backup_host);
void clock_ntp_disconnected(void);
int clock_ntp_fresh(void);
int clock_ntp_utc_now(uint32 *unix_seconds);
/* 0 idle, 1 pending, 2 resolved, 3 failed; last lwIP error per lookup. */
void clock_ntp_dns_status(unsigned slot, uint32 *address, int *state, int *error);

#endif
