# Binary Distribution Policy

This project is licensed under GPLv3. Binary OFX releases must be distributed in
a way that preserves the user's GPL rights.

## Release Requirements

For each binary release:

- Tag the exact source commit used to build the binary.
- Publish the full corresponding source code for that binary.
- Include the GPLv3 license text.
- Include attribution and upstream notices.
- Include build instructions sufficient to rebuild the OFX from source.
- Keep release notes that identify meaningful source or behavior changes.

## OFX Bundle Notes

If distributing `.ofx.bundle`, `.zip`, `.pkg`, `.exe`, or installer builds:

- The release page must link to the matching source archive.
- The binary package should include a copy of `LICENSE`, `NOTICE.md`, and
  `UPSTREAM.md` where practical.
- The bundle metadata should not imply endorsement by the upstream
  `open-display-transform` project.

## No Closed Binary-Only Releases

Do not publish binary-only releases of this project.

