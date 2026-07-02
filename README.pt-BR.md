# OpenDisplayTransform-OFX para DaVinci Resolve

Ports OpenFX de ferramentas DCTL selecionadas do
[open-display-transform](https://github.com/jedypod/open-display-transform)
para uso no DaVinci Resolve.

Este repositório oferece plugins OFX acelerados por GPU, adaptados a partir de
DCTLs selecionadas do projeto ODT. A distribuição será feita pelo
[MCNexus](https://github.com/ciqueira/MCNexus), responsável por ativação de
licença, downloads, atualizações e instalação específica para cada plataforma.

Projeto Open Display Transform original e DCTLs por Jed Smith:

https://github.com/jedypod/open-display-transform

## Plugins Incluídos

Esta versão inclui:

| Plugin | Status | Distribuição | Obter Chave |
| --- | --- | --- | --- |
| ODT N6 Color | Ativo | MCNexus / OpenKey | Em breve |

Backlog planejado:

| Plugin | Status |
| --- | --- |
| ODT Zone Tone | Planejado |

## ODT N6 Color

`ODT N6 Color` é um único plugin OFX que agrupa as seguintes ferramentas DCTL
do Resolve em um só efeito:

- `n6Purity.dctl`
- `n6ChromaValue.dctl`
- `n6Vibrance.dctl`
- `n6HueShift.dctl`
- `n6CrossTalk.dctl`

O port mantém os módulos separados na interface, mas processa todos como
ramos paralelos a partir do mesmo RGB original limpo. Cada módulo lê o RGB de
entrada, calcula seu próprio resultado e contribui através do seu controle
`Mix`. O resultado final é misturado com a entrada limpa usando o controle
global `Output Mix`.

Isso torna a OFX mais previsível para color grading: alterar um módulo não
alimenta inesperadamente o próximo módulo como aconteceria em uma pilha serial.

## Controles

A OFX aparece no Resolve dentro do grupo `Open Display Transform`.

Grupos atuais da interface:

- `Setup`
  - `Transfer Function`
  - `Output Mix`
- `Purity`
  - habilitar/mix
  - controles de pureza e força para RGB/CMY
- `Chroma Value`
  - habilitar/mix
  - controles por matiz para yellow, red, magenta, blue, cyan e green
  - força de matiz/croma e limite de croma
  - zone targeting opcional
- `Vibrance`
  - habilitar/mix
  - controles de vibrance global, por seis matizes e por matiz customizado
  - zone targeting opcional
- `Hue Shift`
  - habilitar/mix
  - shift por seis matizes e por matiz customizado
  - força, limite de croma e zone targeting opcional
- `CrossTalk`
  - habilitar/mix
  - controles de power, shift e scale por matiz
  - controles de centro CMY
- `Support`
  - `About and Help`
  - `App MCNexus`

Opções de transferência suportadas:

- Linear
- Davinci Intermediate
- ACEScct
- Arri LogC3
- Arri LogC4
- RedLog3G10

## Suporte de Plataforma

Os builds atuais são planejados para:

- macOS
- Windows x64

Backends de processamento suportados:

- Metal no macOS
- CUDA no Windows para aceleração em GPUs NVIDIA
- caminho CPU como fallback/referência

## Instalação

Os plugins serão distribuídos pelo MCNexus. Cada plugin terá sua própria licença
OpenKey, mesmo quando vários plugins forem publicados a partir deste mesmo
repositório.

Fluxo de ativação:

1. Solicite a licença OpenKey correspondente ao plugin.
2. Abra o MCNexus.
3. Ative o plugin usando essa chave.
4. Instale ou atualize o plugin pelo MCNexus.

O link final para obter a chave do `ODT N6 Color` ainda não foi publicado.

## Créditos

Projeto Open Display Transform original e DCTLs:

Jed Smith  
https://github.com/jedypod/open-display-transform

Port OFX, distribuição via MCNexus, integração OpenKey e arquitetura MC OFX:

Magno Ciqueira  
https://github.com/ciqueira

OpenFX SDK:

Academy Software Foundation OpenFX  
https://github.com/AcademySoftwareFoundation/openfx

## Licença

Este repositório é licenciado sob a GNU General Public License v3.0.

As DCTLs selecionadas vêm do projeto `open-display-transform`, que também é
licenciado sob GPLv3. Consulte:

- [LICENSE](LICENSE)
- [NOTICE.md](NOTICE.md)
- [UPSTREAM.md](UPSTREAM.md)

O scaffold genérico MC OFX usado como referência de arquitetura é de autoria de
Magno Ciqueira e está autorizado para uso neste projeto GPLv3. Isso não inclui
matemática criativa específica do `Vector` nem lógica de processamento de imagem
não relacionada ao port ODT.

## Releases Binários

Os binários OFX serão distribuídos pelo MCNexus e pelo GitHub Releases.

Cada release binário deve incluir ou apontar para o código-fonte correspondente
àquela versão exata. Os bundles de release incluem GPLv3, atribuição ao projeto
upstream, notas de distribuição binária e a licença BSD 3-Clause do suporte
OpenFX em `Contents/Resources/Legal`.

Consulte [BINARY_DISTRIBUTION.md](BINARY_DISTRIBUTION.md).

## OpenFX SDK

Os builds usam o OpenFX SDK 1.5.1 em:

- `src/third_party/openfx`

Essa pasta intencionalmente não é commitada neste repositório. O GitHub Actions
faz checkout de `AcademySoftwareFoundation/openfx` em `OFX_Release_1.5.1` antes
do build, e builds locais devem colocar a mesma versão do SDK nesse caminho.
