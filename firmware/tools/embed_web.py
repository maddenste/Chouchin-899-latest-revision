# Copyright (C) 2026 Steve Madden
# SPDX-License-Identifier: GPL-3.0-or-later
"""Embed the locally maintained ESP-derived page as a C string (no network assets)."""
import json
import argparse
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--check', action='store_true', help='Check the generated header without writing it')
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]
source = root / 'web' / 'index.html'
if not source.exists():
    raise SystemExit('Missing web/index.html; restore the tracked webpage before generating its header.')
html = source.read_text(encoding='utf-8')
header = '// Copyright (C) 2026 Steve Madden\n// SPDX-License-Identifier: GPL-3.0-or-later\n'
header += '/* Generated from web/index.html; see NOTICE.md for webpage provenance. */\nstatic const char clock_web_ui[] =\n'
header += '\n'.join(json.dumps(line + '\n', ensure_ascii=True) for line in html.splitlines()) + ';\n'
# C does not accept JSON Unicode escapes outside identifiers. Encode UTF-8 bytes
# as fixed-width octal so following digits cannot accidentally extend an escape.
import re
header = re.sub(r'\\u([0-9a-fA-F]{4})', lambda m: ''.join('\\%03o' % b for b in chr(int(m[1],16)).encode('utf-8')), header)
target = root / 'iot_sdk_work' / 'clock_project' / 'clock_web_ui.h'
if args.check:
    if not target.exists() or target.read_text(encoding='ascii') != header:
        raise SystemExit('Generated webpage is stale; run firmware/tools/embed_web.py without --check.')
    print('PASS: embedded webpage matches web/index.html; no files changed')
else:
    target.write_text(header, encoding='ascii')
    print(f'Embedded ESP-derived page: {len(html.encode("utf-8"))} bytes')
