# OpenDisplayTransform-OFX for DaVinci Resolve

[Português do Brasil](README.pt-BR.md)

OpenFX ports of selected Resolve DCTL look tools from
[open-display-transform](https://github.com/jedypod/open-display-transform) for
DaVinci Resolve.

This repository provides GPU-accelerated OFX plugins adapted from selected ODT
DCTLs. The plugins are distributed through
[MCNexus](https://github.com/ciqueira/MCNexus), which handles license
activation, downloads, updates, and platform-specific installation.

Original Open Display Transform project and DCTL source by Jed Smith:

https://github.com/jedypod/open-display-transform

## Included Plugins

This build currently includes:

| Plugin | Status | Distribution | Get Key |
| --- | --- | --- | --- |
| ODT N6 Color | Active | MCNexus / OpenKey | [Get Key](https://bridge.magnociqueira.com.br/github/claim?t=odt-ofx&tmpl=d2968490-fbf7-4d21-a291-1e0ce6900ffe&sig=82a3bb4b8f01d281) |

Planned backlog:

| Plugin | Status |
| --- | --- |
| ODT Zone Tone | Planned |

## ODT N6 Color

`ODT N6 Color` is a single OFX plugin that groups the following Resolve DCTL
tools into one Resolve effect:

- `n6Purity.dctl`
- `n6ChromaValue.dctl`
- `n6Vibrance.dctl`
- `n6HueShift.dctl`
- `n6CrossTalk.dctl`

The port keeps the modules separate in the UI, but processes them as parallel
branches from the same clean RGB input. Each module reads the original source
RGB, computes its own result, and contributes through its own `Mix` control.
The final result is blended against the clean input with the global
`Output Mix`.

This keeps the OFX predictable for grading: changing one module does not feed
unexpectedly into the next module as a serial stack.

## Controls

The OFX appears under the `Open Display Transform` group in Resolve.

Current UI groups:

- `Setup`
  - `Transfer Function`
  - `Output Mix`
- `Purity`
  - enable/mix
  - RGB/CMY purity and strength controls
- `Chroma Value`
  - enable/mix
  - hue controls for yellow, red, magenta, blue, cyan, and green
  - hue/chroma strength and chroma limit
  - optional zone targeting
- `Vibrance`
  - enable/mix
  - global, six-hue, and custom-hue vibrance controls
  - optional zone targeting
- `Hue Shift`
  - enable/mix
  - six-hue and custom-hue shift controls
  - strength, chroma limit, and optional zone targeting
- `CrossTalk`
  - enable/mix
  - per-hue power, shift, and scale controls
  - CMY center controls
- `Support`
  - `About and Help`
  - `App MCNexus`

Supported transfer options:

- Linear
- Davinci Intermediate
- ACEScct
- Arri LogC3
- Arri LogC4
- RedLog3G10

## Platform Support

Current builds are planned for:

- macOS
- Windows x64

Supported processing backends:

- Metal on macOS
- CUDA on Windows for NVIDIA GPU acceleration
- CPU fallback/reference path

## Installation

The plugins are distributed through MCNexus. Each plugin will have its own
OpenKey license entry, even when multiple plugins are published from this
repository.

Activation flow:

1. Claim the matching OpenKey license for the plugin.
2. Open MCNexus.
3. Activate the plugin with that key.
4. Install or update the plugin from MCNexus.

Lost your key? Open the same claim link again with the same GitHub account to
recover the same license.

## Credits

Original Open Display Transform project and DCTL source:

Jed Smith  
https://github.com/jedypod/open-display-transform

OFX port, MCNexus distribution, OpenKey integration, and MC OFX architecture:

Magno Ciqueira  
https://github.com/ciqueira

OpenFX SDK:

Academy Software Foundation OpenFX  
https://github.com/AcademySoftwareFoundation/openfx

## License

This repository is licensed under the GNU General Public License v3.0.

The selected DCTL sources come from `open-display-transform`, which is also
licensed under GPLv3. See:

- [LICENSE](LICENSE)
- [NOTICE.md](NOTICE.md)
- [UPSTREAM.md](UPSTREAM.md)

The generic MC OFX scaffold used as an architecture reference is authored by
Magno Ciqueira and is authorized for use in this GPLv3 project. This does not
include `Vector`-specific creative math or unrelated image-processing logic.

## Binary Releases

Binary OFX releases are distributed through MCNexus and GitHub Releases.

Each binary release must include or link to the corresponding source code for
that exact release. Release bundles include GPLv3, upstream attribution, binary
distribution notes, and the OpenFX BSD 3-Clause support license under
`Contents/Resources/Legal`.

See [BINARY_DISTRIBUTION.md](BINARY_DISTRIBUTION.md).

## OpenFX SDK

Builds use OpenFX SDK 1.5.1 at:

- `src/third_party/openfx`

That folder is intentionally not committed to this repository. GitHub Actions
checks out `AcademySoftwareFoundation/openfx` at `OFX_Release_1.5.1` before
building, and local builds should place the same SDK version in that path.
