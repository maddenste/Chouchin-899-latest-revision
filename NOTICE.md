# Licensing and attribution

Project-authored clock backend, protocol, tests, scripts, documentation and
ESP-derived web work are offered under **GPL-3.0-or-later**, copyright
Steve Madden 2026. The full license text is in [LICENSE](LICENSE). Preserve
upstream headers and provenance rather than replacing every file's notice
with a blanket project copyright.

Project-authored scripts, backend modules and tests carry consistent
GPL-3.0-or-later SPDX identifiers. Generated webpage source carries the same
identifier, and its generator preserves it. These headers describe the
project's licensing intent; they do not grant rights over vendor SDK files
or establish clearance for the combined firmware binaries.

The webpage derives from the
[older Chouchin-CH899 project](https://github.com/maddenste/Chouchin-CH899-Firmware).
The public display is **WiFi Clock · v2.0 — Modified by Steve Madden · 2026**.
This attribution is not a manufacturer endorsement.

## What is not cleared by the project license

The application contains vendor-derived integration overlays and links vendor
SDK libraries. No explicit redistribution licence has yet been established for
all those files. A successful compile does not establish either source or
combined-binary redistribution rights. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

Integration overlays are excluded from the repository. Compiled Wi-Fi R17
and patched HC32 V21 images are supplied separately as release assets at the
owner's decision while underlying redistribution rights remain unresolved.
The owner sent Taixin a permission request; a reply is pending. This decision
is not vendor permission or a claim that the combined binaries are GPL-licensed.

HC32 V21 patches an original vendor binary. Our patch logic does not turn the
underlying proprietary firmware into GPL code. Its binary distribution requires
separate review. Original flash dumps, Wi-Fi secrets and per-board factory data
remain private regardless of copyright clearance.

Vendor SDK/libraries, loader/packaging executables, DebugServer, toolchain,
flash algorithms and developer runtimes are not supplied in this repository.
Owners obtain permitted copies themselves.

GPL text is present, but this is **not a statement that all mixed-tree content
or release binaries are cleared for redistribution**. Collected upstream notices
are in [SDK_NOTICES](docs/licenses/SDK_NOTICES.txt); the standard
[Apache 2.0 text](docs/licenses/APACHE-2.0.txt) accompanies applicable components.
Neither document establishes a blanket licence for the proprietary SDK.

“Hold&Start” is a descriptive UI label. This project is unaffiliated with
Chouchin, HDSC, Taixin, OpenAI or any clock trademark owner.
