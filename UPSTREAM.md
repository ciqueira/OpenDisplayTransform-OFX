# Upstream Tracking

This file records the upstream source used for planning and future porting.

## Upstream Project

- Name: `open-display-transform`
- URL: https://github.com/jedypod/open-display-transform
- License: GNU General Public License v3.0
- Local planning commit: `af683323e2a8a63501f02c0a724ec538e3228ad0`

## Scope

The first port scope is intentionally limited to two OFX plugins:

- `ODT N6 Color`
- `ODT Zone Tone`

Backlog DCTLs should remain out of implementation until they are explicitly
added to this file and to the port plan.

## Update Procedure

When upstream source changes are imported:

1. Record the upstream commit hash.
2. Record which files changed.
3. Note whether the OFX port changed behavior or only refactored code.
4. Keep source attribution comments in any ported math files.
5. Update release notes if the change affects binaries.

## Imported Source Log

No upstream source has been imported into this repository yet. Current work is
planning and licensing setup only.

