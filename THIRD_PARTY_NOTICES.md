# Third-party inventory and unresolved rights

The project GPL licence covers project-authored work, not all linked firmware
components. It does not grant rights to original vendor firmware.

| Material | Origin / treatment |
| --- | --- |
| Clock application, UI, tests and scripts | Steve Madden; UI derived from older CH899 project; GPL-3.0-or-later |
| TXW integration overlays | IOT SDK 2.5.2.6-31320; omitted from repository |
| SDK libraries | Vendor libraries incorporated in APP; redistribution rights unconfirmed |
| lwIP, C-SKY CSI and Rhino source components | Collected upstream headers preserved in SDK_NOTICES; individual terms apply |
| Runtime archives | Linked libgcc.a, libc.a and libm.a from C-SKY toolchain; toolchain not supplied; notice completeness still requires review |
| HC32 V15 | Patch of original board vendor firmware; underlying redistribution rights unconfirmed |
| Bootloader / parameters / packaging tools | Obtained separately; no personal FULL supplied |
| Flash algorithm, DebugServer, compiler, Keil, XHSC | External dependencies, not bundled |
| CKLink reference | wuxx/CKLink-lite linked, not copied |

Observed vendor archives include libcore, libFLASH, libnetutils, libcommon,
libosal, libatcmd, liblmac and libwifi. The collected open-source notices do not
establish permission for these proprietary archives.

[SDK_NOTICES](docs/licenses/SDK_NOTICES.txt) preserves collected upstream comment
notices. [Apache 2.0](docs/licenses/APACHE-2.0.txt) supplies the standard text for
components with applicable headers. This conservative collection may include
unused SDK components; it is not an exhaustive clearance audit.
See [NOTICE](NOTICE.md) for unresolved rights and project attribution.
