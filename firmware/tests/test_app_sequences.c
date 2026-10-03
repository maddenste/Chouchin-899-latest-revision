// Copyright (C) 2026 Steve Madden
// SPDX-License-Identifier: GPL-3.0-or-later
#define main baseline_app_main
#include "test_app.c"
#undef main

static void fixture(int credentials, int ready)
{
    txw_clock_settings_defaults(&stored);
    if (credentials) strcpy(stored.ssid, "TestNetwork");
    stored.generation = 1;
    have_stored = credentials;
    fail_save = fail_iface = starts = ntp_ready = 0;
    ticks = 10000; sent[0] = 0;
    memset(&sys_status, 0, sizeof(sys_status));
    clock_app_prepare();
    if (ready) clock_app_network_ready();
}

static void event(unsigned e)
{
    struct txw_clock_settings candidate;
    sent[0] = 0;
    switch (e) {
    case 0: clock_app_command(TXW_HC32_COMMAND_WIFI_ID); break;
    case 1: clock_app_command(TXW_HC32_COMMAND_WIFI_AP); break;
    case 2: clock_app_command(TXW_HC32_COMMAND_WIFI_STA); break;
    case 3: clock_app_command(TXW_HC32_COMMAND_WIFI_RESET); break;
    case 4: clock_app_portal_client_seen(); break;
    case 5: clock_app_dhcp_ready(); break;
    case 6: clock_app_disconnected(); break;
    case 7:
        clock_app_settings_copy(&candidate); strcpy(candidate.ssid, "TestNetwork");
        assert(clock_app_save_settings(&candidate)); break;
    case 8: clock_app_network_ready(); break;
    case 9: ticks += 1300; clock_app_tick(); break;
    }
    assert(!strstr(sent, "+TIME:") || (configured && interface_ready &&
           !clock_app_is_portal() && ntp_ready && !reset_armed));
    assert(!strstr(sent, "WIFIRESET OK") ||
           (!configured && !stored.ssid[0] && !ntp_ready));
}

int main(void)
{
    struct txw_clock_settings candidate;
    unsigned initial, code, i, delay, sequences = 0, timing = 0;
    for (initial = 0; initial < 4; ++initial) for (code = 0; code < 10000; ++code) {
        unsigned sequence = code;
        fixture(initial & 1, initial & 2);
        for (i = 0; i < 4; ++i) { event(sequence % 10); sequence /= 10; }
        ++sequences;
    }
    for (initial = 0; initial < 2; ++initial) for (delay = 0; delay <= 5000; ++delay) {
        fixture(1, 1); ticks = initial ? UINT32_MAX - 100u : 10000u;
        clock_app_command(TXW_HC32_COMMAND_WIFI_RESET); ticks += delay;
        clock_app_command(TXW_HC32_COMMAND_WIFI_RESET);
        assert((!configured) == (delay >= 750 && delay <= 4000)); ++timing;
    }
    /* A stalled task processes the two frames together, with arrival times intact. */
    fixture(1, 1); ticks = 20000;
    clock_app_command_at(TXW_HC32_COMMAND_WIFI_RESET, 10000);
    clock_app_command_at(TXW_HC32_COMMAND_WIFI_RESET, 11300);
    assert(!configured && strstr(sent, "WIFIRESET OK"));
    fixture(1, 1); ticks = 20000;
    clock_app_command_at(TXW_HC32_COMMAND_WIFI_RESET, 10000);
    clock_app_command_at(TXW_HC32_COMMAND_WIFI_RESET, 10100);
    assert(configured && !strstr(sent, "WIFIRESET OK"));
    /* A failed start is never acknowledged. Repeated requests and timer retry recover. */
    fixture(1, 1); fail_iface = 1;
    clock_app_command(TXW_HC32_COMMAND_WIFI_AP); expect("");
    clock_app_command(TXW_HC32_COMMAND_WIFI_AP); expect("");
    assert(starts == 2 && !interface_ready);
    fail_iface = 0; ticks += 1000; clock_app_tick(); expect("WIFIAP OK\r\n");
    assert(starts == 3 && interface_ready);
    fixture(0, 1); clock_app_command(TXW_HC32_COMMAND_WIFI_STA); expect("");
    ticks += 4000; clock_app_tick(); expect("WIFIEXIT\r\n");
    assert(pending_mode == TXW_HC32_COMMAND_NONE);
    fixture(0, 1); clock_app_settings_copy(&candidate); strcpy(candidate.ssid, "TestNetwork");
    assert(clock_app_save_settings(&candidate)); ticks += 500; fail_iface = 1;
    clock_app_tick(); expect("");
    clock_app_command(TXW_HC32_COMMAND_WIFI_STA); expect("");
    assert(starts == 2 && !interface_ready);
    fail_iface = 0; ticks += 1000; clock_app_tick(); expect("WIFISTA OK\r\n");
    assert(starts == 3 && interface_ready);
    /* New AP request supersedes delayed save/reconnect, with no surprise STA switch. */
    fixture(1, 1); clock_app_settings_copy(&candidate); assert(clock_app_save_settings(&candidate));
    ticks += 100; clock_app_command(TXW_HC32_COMMAND_WIFI_AP); expect("WIFIAP OK\r\n");
    ticks += 500; clock_app_tick(); expect("");
    assert(clock_app_is_portal() && !station_switch_pending);
    /* Save invalidates old time; not even one stale +TIME is sent during reconnect. */
    fixture(1, 1); clock_app_dhcp_ready();
    clock_app_settings_copy(&candidate); candidate.daily_hour = 17;
    assert(clock_app_save_settings(&candidate)); clock_app_tick(); expect("");
    assert(!ntp_ready); ticks += 500; clock_app_tick(); expect("WIFIAPPOK\r\n");
    clock_app_dhcp_ready(); clock_app_tick(); assert(strstr(sent, " 17:00 00\r\n"));
    /* Saving cancels an old armed pair, including frames queued before the save. */
    fixture(1, 1); clock_app_command(TXW_HC32_COMMAND_WIFI_RESET);
    ticks += 500; clock_app_settings_copy(&candidate); assert(clock_app_save_settings(&candidate));
    ticks += 800; sent[0] = 0; clock_app_command(TXW_HC32_COMMAND_WIFI_RESET);
    assert(configured && !strstr(sent, "WIFIRESET OK"));
    clock_app_command_at(TXW_HC32_COMMAND_WIFI_RESET, 10000);
    clock_app_command_at(TXW_HC32_COMMAND_WIFI_RESET, 10300);
    assert(configured);
    /* Existing AP reset renews its grace period and clears the old browser lease. */
    fixture(0, 1); ticks += 10000; clock_app_portal_client_seen();
    clock_app_command(TXW_HC32_COMMAND_WIFI_RESET); ticks += 1300;
    clock_app_command(TXW_HC32_COMMAND_WIFI_RESET); expect("WIFIRESET OK\r\n");
    clock_app_tick(); expect(""); ticks += 3999; clock_app_tick(); expect("");
    ++ticks; clock_app_tick(); expect("WIFIEXIT\r\n");
    clock_app_portal_client_seen(); ticks += 1000; clock_app_tick(); expect("WIFIAPPING\r\n");
    /* Reset pause is bounded; failed flash commit preserves settings and time. */
    fixture(1, 1); clock_app_dhcp_ready(); fail_save = 1;
    clock_app_command(TXW_HC32_COMMAND_WIFI_RESET); clock_app_tick(); expect("");
    ticks += 1300; clock_app_command(TXW_HC32_COMMAND_WIFI_RESET); expect("");
    assert(configured && stored.ssid[0]); ticks += 2701; clock_app_tick();
    assert(strstr(sent, "+TIME:") && !reset_armed);
    printf("App sequences: %u command/event sequences, %u timing/rollover cases, buffered reset, start failures/retries, latest-mode priority, save/reset barriers, fresh NTP and AP lease recovery passed\n", sequences, timing);
    return 0;
}
