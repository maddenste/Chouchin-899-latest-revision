# Install WiFi Clock v2.0

For the newer Chouchin-899 with **HC32L130J8TA + TXW813-320** only.
Identify both chips first; older MM32/ESP boards need
[the older project](https://github.com/maddenste/Chouchin-CH899-Firmware).

1. Download the repository ZIP and firmware from
   [v2.0-rc1](https://github.com/maddenste/Chouchin-899-latest-revision/releases/tag/v2.0-rc1).
2. Keep verified original backups of both chips private.
3. [Install HC32 V15](HC32.md) using the full HEX in XHSC.
4. [Install Wi-Fi R16](FLASHING.md) using the full image at address zero.
5. Restart, join **WiFi-Clock-Setup**, open http://192.168.4.1/ and save settings.
6. Check time, movement, parking and daily wake using [the test guide](TESTING.md).

If one chip already has the correct version, you only need to update the other.
Programming replaces the full flash. Wi-Fi settings start empty and must be
entered after flashing. Original firmware is not supplied.

No compiling is needed for these downloads. The published source omits vendor
startup/build integration and is not a complete reproducible cross-build tree.
See [hardware](HARDWARE.md), [tools](TOOLS.md), [user guide](USER_GUIDE.md) and
[the three-page manual](WiFi-Clock-User-Manual-v2.0.pdf) for reference.
