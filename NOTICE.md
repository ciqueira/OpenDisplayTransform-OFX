# Notices and Attribution

This project is an OpenFX port derived from selected Resolve DCTL tools from:

- Project: `open-display-transform`
- Upstream URL: https://github.com/jedypod/open-display-transform
- Author: Jed Smith
- License: GNU General Public License v3.0
- Local upstream commit used for planning: `af683323e2a8a63501f02c0a724ec538e3228ad0`

## Selected Upstream Files

Current planned DCTL source scope:

### ODT N6 Color

- `look-transforms/tools/resolve/n6Purity.dctl`
- `look-transforms/tools/resolve/n6ChromaValue.dctl`
- `look-transforms/tools/resolve/n6Vibrance.dctl`
- `look-transforms/tools/resolve/n6HueShift.dctl`
- `look-transforms/tools/resolve/n6CrossTalk.dctl`
- `look-transforms/tools/resolve/libLMT.h`

### ODT Zone Tone

- `look-transforms/tools/resolve/ZoneGrade.dctl`
- `look-transforms/tools/resolve/ZoneExposureHigh.dctl`
- `look-transforms/tools/resolve/ZoneExposureLow.dctl`
- `look-transforms/tools/resolve/ShadowContrast.dctl`
- `look-transforms/tools/resolve/HighlightContrast.dctl`
- `look-transforms/tools/resolve/MidtoneContrast.dctl`
- `look-transforms/tools/resolve/libLMT.h`

## Modification Notice

This repository is not the upstream `open-display-transform` project. It is a
separate OpenFX port that will adapt selected DCTL math and UI concepts to an
OFX plugin architecture.

When implementation begins, source files that contain ported upstream math
should keep clear attribution comments and note that they are modified/ported
from the upstream DCTL sources.

## MC OFX Architecture Note

The planned OFX architecture is based on the local MC OFX plugin pattern used
by the `vector` core plugin.

Magno Ciqueira is the author/rightsholder of the MC OFX `Vector` scaffold used
as the local architecture reference. For this project, the generic scaffold
needed to build the new ODT OFXs is authorized for use under GPLv3.

Scope of authorized reuse:

- OFX plugin/factory structure
- CPU/Metal/CUDA render dispatch pattern
- params struct pattern
- Support group/buttons pattern
- build/scaffold organization

Out of scope:

- `Vector`-specific creative math
- `Vector`-specific image-processing logic
- unrelated presets or algorithms

The ODT math remains attributed to the upstream `open-display-transform`
project and must keep GPLv3-compatible attribution/modification notices.

## OpenFX SDK Notice

This repository vendors OpenFX SDK 1.5.1 under:

- `src/third_party/openfx`

The OpenFX SDK is licensed under the BSD 3-Clause License. Its license and
notices must be preserved in the vendored copy.

Initial local source for the vendored SDK:

- `/Volumes/DataMEDIA/DEV/_OFX/MC OFX/MCPlugins/third_party/openfx`
