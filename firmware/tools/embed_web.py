"""Embed the locally maintained ESP-derived page as a C string (no network assets)."""
import json
from pathlib import Path

root = Path(__file__).resolve().parents[1]
source = root / 'web' / 'index.html'
if not source.exists():
    # One-time mechanical extraction from the preserved ESP generated header.
    original = (root.parent / 'CH899_Hourly_ESP_Test' / 'web_ui.h').read_text(encoding='utf-8')
    source.parent.mkdir(exist_ok=True)
    source.write_text(original.split('R"CH899_WEB(\n', 1)[1].split('\n)CH899_WEB";', 1)[0], encoding='utf-8')
html = source.read_text(encoding='utf-8')
header = '/* Generated from web/index.html; GPL-3.0-or-later. */\nstatic const char clock_web_ui[] =\n'
header += '\n'.join(json.dumps(line + '\n', ensure_ascii=True) for line in html.splitlines()) + ';\n'
# C does not accept JSON Unicode escapes outside identifiers. Encode UTF-8 bytes
# as fixed-width octal so following digits cannot accidentally extend an escape.
import re
header = re.sub(r'\\u([0-9a-fA-F]{4})', lambda m: ''.join('\\%03o' % b for b in chr(int(m[1],16)).encode('utf-8')), header)
(root / 'iot_sdk_work' / 'clock_project' / 'clock_web_ui.h').write_text(header, encoding='ascii')
print(f'Embedded ESP-derived page: {len(html.encode("utf-8"))} bytes')
