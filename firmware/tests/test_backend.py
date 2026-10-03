# Copyright (C) 2026 Steve Madden
# SPDX-License-Identifier: GPL-3.0-or-later
"""Execute the actual portable C backend through a native test DLL."""
import ctypes as C
import datetime as dt
import hashlib
import random
import ast
import re
from zoneinfo import ZoneInfo
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
lib = C.CDLL(str(ROOT / 'tests' / 'backend.dll'))

class Settings(C.Structure):
    _fields_ = [('magic', C.c_uint32), ('version', C.c_uint16),
                ('length', C.c_uint16), ('generation', C.c_uint32),
                ('ssid', C.c_char * 33), ('password', C.c_char * 65),
                ('ntp_host', C.c_char * 128), ('ntp_backup_host', C.c_char * 128), ('timezone', C.c_char * 65),
                ('daily_hour', C.c_uint8), ('daily_minute', C.c_uint8),
                ('movement_mode', C.c_uint8), ('night_parking', C.c_uint8),
                ('psk', C.c_uint8 * 32), ('psk_ready', C.c_uint8),
                ('reserved', C.c_uint8 * 3), ('crc32', C.c_uint32)]

class Time(C.Structure):
    _fields_ = [('year', C.c_uint16)] + [(n, C.c_uint8) for n in
                ('month', 'day', 'weekday', 'hour', 'minute', 'second')] + [
                ('utc_offset_minutes', C.c_int16)] + [(n, C.c_uint8) for n in
                ('daily_update_hour', 'daily_update_minute', 'movement_mode', 'night_parking')]

class DstChange(C.Structure):
    _fields_ = [('utc_seconds', C.c_uint32), ('before_minutes', C.c_int16), ('after_minutes', C.c_int16)]
lib.txw_clock_next_dst_change.argtypes = [C.c_uint32, C.c_char_p, C.POINTER(DstChange)]
lib.txw_clock_next_wake.argtypes = [C.c_uint32, C.c_char_p, C.c_uint8, C.c_uint8, C.POINTER(C.c_uint8), C.POINTER(C.c_uint8)]

lib.clock_psk_derive.argtypes = [C.c_char_p, C.c_char_p, C.c_void_p, C.c_void_p]
lib.txw_clock_local_from_unix.argtypes = [C.c_uint32, C.c_char_p, C.POINTER(Time)]
lib.txw_hc32_format_time_for_clock.argtypes = [C.c_void_p, C.c_size_t, C.POINTER(Time)]
lib.txw_hc32_format_time_for_clock.restype = C.c_size_t
lib.txw_ntp_make_request.argtypes = [C.c_void_p, C.c_uint64]
lib.txw_ntp_parse_reply.argtypes = [C.c_void_p, C.c_size_t, C.c_uint64, C.POINTER(C.c_uint32)]

rng = random.Random(813)
vectors = [(b'IEEE', b'password'), (b'ThisIsASSID', b'ThisIsAPassword'),
           (b'A' * 32, b'B' * 63)]
for _ in range(30):
    vectors.append((bytes(rng.randrange(32, 127) for _ in range(rng.randrange(1, 33))),
                    bytes(rng.randrange(32, 127) for _ in range(rng.randrange(8, 64)))))
for ssid, password in vectors:
    out = (C.c_uint8 * 32)()
    assert lib.clock_psk_derive(ssid, password, out, None) == 1
    assert bytes(out) == hashlib.pbkdf2_hmac('sha1', password, ssid, 4096, 32)
assert not lib.clock_psk_derive(b'', b'password', out, None)
assert not lib.clock_psk_derive(b'network', b'short', out, None)
assert not lib.clock_psk_derive(b'A' * 33, b'password', out, None)
print('WPA PSK: 33 independent PBKDF2 reference vectors passed')

s = Settings()
lib.txw_clock_settings_defaults(C.byref(s))
assert s.movement_mode == 0 and s.night_parking == 0
assert C.sizeof(Settings) == 476 and s.ntp_backup_host == b'time.cloudflare.com'
for selector in range(3):
    s.night_parking = selector
    lib.txw_clock_settings_seal(C.byref(s), 1)
    assert lib.txw_clock_settings_valid(C.byref(s))
s.night_parking = 3
assert not lib.txw_clock_settings_values_valid(C.byref(s))
s.night_parking = 0
lib.txw_clock_settings_seal(C.byref(s), 1)
assert lib.txw_clock_settings_valid(C.byref(s))
for field in ('ntp_host', 'ntp_backup_host'):
    original = getattr(s, field)
    for bad_host in (b'', b'https://pool.ntp.org', b'a b', b'.bad', b'bad.', b'server:123'):
        setattr(s, field, bad_host)
        assert not lib.txw_clock_settings_values_valid(C.byref(s))
    for good_host in (b'pool.ntp.org', b'time.cloudflare.com', b'192.168.0.2', b'a' * 127):
        setattr(s, field, good_host)
        assert lib.txw_clock_settings_values_valid(C.byref(s))
    setattr(s, field, original)
s.ssid, s.password = b'network', b'password'
lib.txw_clock_settings_seal(C.byref(s), 2)
assert not lib.txw_clock_settings_valid(C.byref(s))  # No stored PSK yet.
s.psk_ready = 1
lib.txw_clock_settings_seal(C.byref(s), 3)
assert lib.txw_clock_settings_valid(C.byref(s))
raw = bytes(s)
for i in range(len(raw)):
    bad = Settings.from_buffer_copy(raw)
    C.cast(C.byref(bad), C.POINTER(C.c_uint8))[i] ^= 1
    assert not lib.txw_clock_settings_valid(C.byref(bad)), i
print(f'Settings: defaults, cached-key requirement and all {len(raw)} corruption positions passed')

def timestamp(text):
    return int(dt.datetime.fromisoformat(text).replace(tzinfo=dt.timezone.utc).timestamp())

def wake(utc, zone, hour=10, minute=0):
    out_h, out_m = C.c_uint8(), C.c_uint8()
    overridden = lib.txw_clock_next_wake(timestamp(utc), zone.encode(), hour, minute, C.byref(out_h), C.byref(out_m))
    return overridden, out_h.value, out_m.value

uk = 'GMT0BST,M3.5.0/1,M10.5.0/2'
assert wake('2026-03-28T10:00:00', uk) == (1, 1, 0)
assert wake('2026-03-29T01:00:00', uk) == (0, 10, 0)
assert wake('2026-10-24T09:00:00', uk) == (1, 2, 0)
assert wake('2026-10-25T01:00:00', uk) == (0, 10, 0)
assert wake('2026-10-24T09:00:00', uk, 0) == (0, 0, 0)  # Normal wake earlier.
assert wake('2026-10-25T00:00:00', uk, 0) == (1, 2, 0)
assert wake('2026-10-24T09:00:00', uk, 2) == (0, 2, 0)  # Already scheduled there.
assert wake('2026-10-23T09:00:00', uk) == (0, 10, 0)  # Not the last daily wake.
assert wake('2026-10-24T09:00:00', 'GMT0') == (0, 10, 0)  # Disabled.
assert wake('2026-10-24T09:00:00', 'UTC+05:30') == (0, 10, 0)
fractional = 'AAA0BBB,M3.5.0/1:30,M10.5.0/2:30'
assert wake('2026-03-28T10:00:00', fractional) == (1, 1, 30)
assert wake('2026-10-24T09:00:00', fractional) == (1, 2, 30)
assert wake('2026-03-28T10:00:00', 'AAA0BBB,M3.5.0/1:00:01,M10.5.0/2') == (1, 1, 1)
assert wake('2026-03-28T10:00:00', 'AAA0BBB,M3.5.0/1:59:59,M10.5.0/2') == (1, 2, 0)
assert wake('2026-03-29T10:00:00', 'AAA0BBB,M3.5.0/23:59:59,M10.5.0/2') == (1, 0, 0)
assert wake('2026-03-29T00:00:00', fractional, 1, 29) == (0, 1, 29)
assert wake('2026-03-29T00:00:00', fractional, 1, 30) == (0, 1, 30)
assert wake('2026-03-29T00:00:00', fractional, 1, 31) == (1, 1, 30)
assert wake('2026-03-29T01:30:00', fractional, 10, 45) == (0, 10, 45)
assert wake('2026-10-25T01:30:00', fractional, 10, 45) == (0, 10, 45)
assert wake('2026-10-03T00:00:00', 'AEST-10AEDT,M10.1.0,M4.1.0/3') == (1, 2, 0)
change = DstChange()
assert lib.txw_clock_next_dst_change(timestamp('2026-10-01T00:00:00'), uk.encode(), C.byref(change))
assert (change.utc_seconds, change.before_minutes, change.after_minutes) == (timestamp('2026-10-25T01:00:00'), 60, 0)
assert not lib.txw_clock_next_dst_change(timestamp('2026-10-01T00:00:00'), b'GMT0', C.byref(change))
print('DST wake: UK forward/back, exact minutes, second rounding, midnight carry, southern hemisphere, daily-wake ordering, restoration and Disabled passed')

# Signed/extended transition times can cross a rule-year boundary.
for zone, before, boundary, old_offset, new_offset in [
    (b'AAA0BBB,M1.1.4/-2,M6.1.0/2', '2025-12-31T21:59:59', '2025-12-31T22:00:00', 0, 60),
    (b'AAA0BBB,M12.5.4/26,M6.1.0/2', '2027-01-01T01:59:59', '2027-01-01T02:00:00', 0, 60),
    (b'AAA0BBB,M6.1.0/2,M1.1.4/-2', '2025-12-31T20:59:59', '2025-12-31T21:00:00', 60, 0),
]:
    assert lib.txw_clock_timezone_supported(zone)
    for instant, expected in [(before, old_offset), (boundary, new_offset)]:
        t = Time()
        assert lib.txw_clock_local_from_unix(timestamp(instant), zone, C.byref(t))
        assert t.utc_offset_minutes == expected, (zone, instant, t.utc_offset_minutes)
        assert lib.txw_hc32_format_time_for_clock(C.create_string_buffer(64), 64, C.byref(t))
    assert lib.txw_clock_next_dst_change(timestamp(before), zone, C.byref(change))
    assert change.utc_seconds == timestamp(boundary)
for unsupported in (b'AAA-23BBB,M3.5.0/2,M10.5.0/2',
                    b'AAA0BBB-23:59:01,M3.5.0/2,M10.5.0/2'):
    assert not lib.txw_clock_timezone_supported(unsupported)
assert lib.txw_clock_timezone_supported(b'AAA-22:59BBB,M3.5.0/2,M10.5.0/2')
assert lib.txw_clock_timezone_supported(b'AAA0BBB23:59,M3.5.0/2,M10.5.0/2')
print('Custom DST: adjacent-year transitions and wire-offset limits passed')

for instant, expected in [
    ('2026-03-29T00:59:59', (0, 59, 59, 0)),
    ('2026-03-29T01:00:00', (2, 0, 0, 60)),
    ('2026-10-25T00:59:59', (1, 59, 59, 60)),
    ('2026-10-25T01:00:00', (1, 0, 0, 0))]:
    t = Time()
    assert lib.txw_clock_local_from_unix(timestamp(instant), s.timezone, C.byref(t))
    assert (t.hour, t.minute, t.second, t.utc_offset_minutes) == expected
t = Time()
assert lib.txw_clock_local_from_unix(timestamp('2026-10-01T12:34:56'), s.timezone, C.byref(t))
t.daily_update_hour = 10
t.night_parking = 1
line = C.create_string_buffer(64)
assert lib.txw_hc32_format_time_for_clock(line, 64, C.byref(t))
assert line.value == b'+TIME:Thu Oct  1 13:34:56 2026 +0100 10:00 01\r\n', line.value
print('Calendar/UART: both DST transitions and exact HC32 V11 01 record passed')

# Every webpage preset is exercised against an independent IANA reference.
html = (ROOT / 'web' / 'index.html').read_text(encoding='utf-8')
presets = ast.literal_eval(re.search(r'const TIMEZONE_PRESETS = (\[.*?\]);', html, re.S)[1])
iana = ['Etc/GMT+12','Pacific/Pago_Pago','Pacific/Honolulu','America/Anchorage',
        'America/Los_Angeles','America/Phoenix','America/Denver','America/Chicago',
        'America/Mexico_City','America/New_York','America/Lima','America/Halifax',
        'America/Santiago','America/St_Johns','America/Argentina/Buenos_Aires',
        'Atlantic/South_Georgia','Atlantic/Azores','UTC','Europe/London',
        'Europe/Paris','Europe/Athens','Africa/Johannesburg','Europe/Moscow',
        'Asia/Tehran','Asia/Dubai','Asia/Karachi','Asia/Almaty','Asia/Kolkata',
        'Asia/Dhaka','Asia/Yangon','Asia/Bangkok','Asia/Singapore','Asia/Tokyo',
        'Australia/Darwin','Australia/Sydney','Pacific/Guadalcanal','Pacific/Auckland',
        'Pacific/Fiji','Pacific/Apia','Pacific/Kiritimati']
assert len(presets) == len(iana)
checks = 0
for preset, location in zip(presets, iana):
    for rule in preset[2:]:
        assert lib.txw_clock_timezone_supported(rule.encode()), rule
    zone = ZoneInfo(location)
    cursor = timestamp('2026-10-01T00:00:00')
    boundary = timestamp('2036-01-01T00:00:00')
    event = DstChange()
    while lib.txw_clock_next_dst_change(cursor, preset[3].encode(), C.byref(event)) and event.utc_seconds < boundary:
        assert event.utc_seconds > cursor
        before = dt.datetime.fromtimestamp(event.utc_seconds - 1, dt.timezone.utc).astimezone(zone)
        after = dt.datetime.fromtimestamp(event.utc_seconds, dt.timezone.utc).astimezone(zone)
        assert event.before_minutes == int(before.utcoffset().total_seconds() / 60), preset[0]
        assert event.after_minutes == int(after.utcoffset().total_seconds() / 60), preset[0]
        assert event.before_minutes != event.after_minutes
        # At the preceding daily wake, choose the next whole old-clock hour.
        now = event.utc_seconds - 20 * 3600
        saved_h = ((now + event.before_minutes * 60) // 3600) % 24
        out_h, out_m = C.c_uint8(), C.c_uint8()
        assert lib.txw_clock_next_wake(now, preset[3].encode(), saved_h, 0, C.byref(out_h), C.byref(out_m))
        expected_h = ((event.utc_seconds + event.before_minutes * 60 + 3599) // 3600) % 24
        assert (out_h.value, out_m.value) == (expected_h, 0), preset[0]
        assert not lib.txw_clock_next_wake(event.utc_seconds, preset[3].encode(), saved_h, 0, C.byref(out_h), C.byref(out_m))
        assert (out_h.value, out_m.value) == (saved_h, 0)
        cursor = event.utc_seconds
    instant = dt.datetime(2026, 10, 1, tzinfo=dt.timezone.utc)
    end = dt.datetime(2036, 1, 1, tzinfo=dt.timezone.utc)
    while instant < end:
        for hour in (0, 1, 2, 3, 10, 12, 22, 23):
            utc = instant.replace(hour=hour)
            expected = utc.astimezone(zone)
            actual = Time()
            assert lib.txw_clock_local_from_unix(int(utc.timestamp()), preset[3].encode(), C.byref(actual))
            got = (actual.year, actual.month, actual.day, actual.hour, actual.minute, actual.utc_offset_minutes)
            want = (expected.year, expected.month, expected.day, expected.hour, expected.minute, int(expected.utcoffset().total_seconds()/60))
            assert got == want, (preset[0], utc, got, want)
            checks += 1
        instant += dt.timedelta(days=1)
for invalid in ('GMT0BST', 'GMT0BST,M13.1.0,M10.1.0', 'GMT0BST,M3.0.0,M10.5.0',
                'GMT0BST,M3.5.7,M10.5.0', 'GMT0BST,M3.5.0/,M10.5.0', 'X0',
                'IST-5:60', 'UTC24', 'UTC0junk', 'GMT0BST,J60,J300'):
    assert not lib.txw_clock_timezone_supported(invalid.encode()), invalid
print(f'Timezones: all 40 webpage locations, 80 rules, {checks} IANA comparisons through 2035 passed')

nonce = 0x0102030405060708
request = (C.c_uint8 * 48)()
lib.txw_ntp_make_request(request, nonce)
assert bytes(request)[40:48] == nonce.to_bytes(8, 'big')
def reply_for(unix):
    packet = bytearray(48)
    packet[0], packet[1] = 0x24, 2
    packet[24:32] = nonce.to_bytes(8, 'big')
    packet[40:44] = ((unix + 2208988800) & 0xffffffff).to_bytes(4, 'big')
    return packet
def parse(packet, count=48):
    result = C.c_uint32()
    buf = C.create_string_buffer(bytes(packet))
    ok = lib.txw_ntp_parse_reply(buf, count, nonce, C.byref(result))
    return ok, result.value
for instant in ('2026-10-01T12:34:56', '2036-02-08T00:00:00'):
    unix = timestamp(instant)
    assert parse(reply_for(unix)) == (1, unix)
for pos, value in ((0, 0xe4), (0, 0x23), (1, 0), (1, 16), (24, 99)):
    bad = reply_for(timestamp('2026-10-01T12:34:56'))
    bad[pos] = value
    assert not parse(bad)[0]
assert not parse(reply_for(timestamp('2026-10-01T12:34:56')), 47)[0]
assert not parse(reply_for(timestamp('2023-01-01T00:00:00')))[0]
# A redirect must echo every byte of the outstanding request token. Check
# every position, including stale responses to a different request.
for pos in range(24, 32):
    bad = reply_for(timestamp('2026-10-01T12:34:56'))
    bad[pos] ^= 1
    assert not parse(bad)[0]
valid = reply_for(timestamp('2026-10-01T12:34:56'))
valid[0] = 0x1c  # NTPv3 server replies are also supported by local redirects.
assert parse(valid) == (1, timestamp('2026-10-01T12:34:56'))
print('NTP: validated replies, 2036 rollover, short packets, bad nonce/stratum/mode rejected')
print('All portable backend execution tests passed')
