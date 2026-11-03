# Destinia 0.17.5 Beta

Destinia é um projeto para jogar no PC usando os dados originais do jogo Android, com melhorias de interface, acessibilidade e recursos opcionais. A versão normal e o Zoom compartilham o mesmo progresso configurado.

**[Baixar a versão 0.17.5 Beta para Windows](https://github.com/sugyua/destinia/releases/tag/v0.17.5-beta)** · [Todas as releases](https://github.com/sugyua/destinia/releases) · [Histórico de mudanças](CHANGELOG.md)

> **Beta:** faça backup dos saves antes de atualizar. Nem toda a campanha e todos os controles físicos foram testados nesta versão.

## Começar no PC

1. Na página da release, baixe o pacote `windows-x64` e confira seu arquivo `.sha256`.
2. Instale o [MSYS2](https://www.msys2.org/) e as dependências UCRT64 indicadas abaixo.
3. Extraia o pacote e coloque a pasta `game` com seus dados originais compatíveis ao lado de `VERSION`. Os arquivos originais do jogo não acompanham a release.
4. Abra `Destinia.exe` para a apresentação normal ou `Destinia Zoom.exe` para a área de jogo ampliada. Use `Configurar pasta de saves.bat` para selecionar seu progresso.

No terminal **MSYS2 UCRT64**, instale as dependências de execução:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_mixer mingw-w64-ucrt-x86_64-unicorn mingw-w64-ucrt-x86_64-zlib
```

Os pacotes são distribuídos sem bibliotecas ou instaladores de terceiros. Consulte [INSTALL.md](INSTALL.md) para detalhes de dependências, estrutura dos arquivos e compilação; as referências a versões antigas nesse guia são históricas.

## Versão de PC

Na candidata 0.17.5 PC, os titulos extras aparecem apos as sete paginas originais. Para testar Cacador de Fungos, derrote 10 Fungos comuns e abra a selecao de titulos no retrato do status; o bonus e de 10 pontos percentuais de chance de critico. Veterano, Mestre do Combate e Matador de Chefes ainda nao possuem regras de desbloqueio definidas. Consulte [notas da candidata](release/NOTAS_0.17.5.md) e [verificacao](docs/VERIFICACAO_0.17.5.md).

Na 0.17.4, a autopoção usa a rotina original e verifica o estoque, o Mago dos Bosses lê o saldo real de ouro e o rastreador de missões no Zoom foi afastado da área superior. A posição do rastreador ainda precisa de confirmação visual durante o combate. Veja as [notas da versão](release/NOTAS_0.17.4.md) e a [verificação](docs/VERIFICACAO_0.17.4.md).

**Limitações conhecidas:** o crédito de contratos e recompensas antigas, a mana acima do máximo e a barra de abas duplicada seguem em investigação. Os resets pagos de atributos e habilidades do Mago estão temporariamente indisponíveis e não cobram ouro. A regra de derrota para bosses e ondas ainda não foi entregue.

## Comandos úteis no PC

| Tecla | Ação |
| --- | --- |
| `H` | Ligar ou desligar a autopoção |
| `Q` | Mostrar ou ocultar o rastreador de missões |
| `F4` | Abrir o catálogo de equipamentos |
| `F5` | Abrir o baú compartilhado |
| `F8` | Abrir os contratos; no inventário, prévia/confirmação de desmontagem em lote |
| `F9` | Abrir o menu do Mago dos Bosses |
| `F11` | Alternar tela cheia |
| `F12` | Mostrar ou ocultar o diagnóstico |
| `Ctrl+F3` | Consultar atributos secundários |

Os comandos podem variar com a tela aberta e as preferências configuradas. O jogo também aceita mouse e controles reconhecidos; configurações ficam em `Configurar Destinia.bat`.

## Saves e testes

Os hosts usam a pasta de saves configurada; a localização padrão é `%LOCALAPPDATA%\Destinia\save`. Guarde uma cópia dos saves e dos arquivos auxiliares antes de atualizar ou testar. `Testar Destinia.bat` oferece roteiros de diagnóstico com cópias; a verificação visual e a campanha completa dependem de uma partida real.

## Web experimental

O acesso existente pela [VPN Tailscale](http://100.108.92.19:8000/?v=0.17.2) permanece na revisão 0.17.2 e exige o PC ligado e a VPN conectada. O servidor estático não confirma sincronização de saves; mantenha a aba aberta se não houver confirmação. O [Site privado](https://destinia-web-experimental.leafyheron257.chatgpt.site/) mantém a versão 0.13.0 anteriormente conhecida. A atualização de PC não altera essas publicações nem sua audiência.

Na Web atual, a autopoção tem falhas conhecidas; desative-a com `H` em cada reinício até haver uma versão corrigida e verificada.

## Desenvolvimento

Os hosts C++ usam SDL2 para vídeo/entrada e Unicorn para executar a biblioteca ARM original; `common/` reúne componentes compartilhados e `tests/` contém testes. O modo normal usa framebuffer 400×240; o Zoom usa 800×480. Os recursos originais, saves e dependências externas não são distribuídos no pacote de fontes.

Para compilar, consulte [INSTALL.md](INSTALL.md). Para acompanhar propostas e limitações, veja o [roadmap](docs/ROADMAP.md), as [issues](https://github.com/sugyua/destinia/issues) e o [arquivo histórico do README](docs/README_HISTORICO_2026-11-01.md). O histórico registra ideias e informações de épocas anteriores, não o estado atual.

| Release histórica | Data |
| --- | --- |
| [0.16.0 Beta — Desmontagem em lote](https://github.com/sugyua/destinia/releases/tag/v0.16.0-beta) | 28/10/2026 |
