# Destinia para Windows — 0.3 Beta

> **O jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves antes de atualizar.**

**Versão atual: 0.3 Beta, distribuída em código-fonte.**

**[Baixar a versão 0.3](https://github.com/sugyua/destinia/releases/tag/v0.3-beta)** · [Versão 0.2](https://github.com/sugyua/destinia/releases/tag/v0.2-beta) · [Todas as releases](https://github.com/sugyua/destinia/releases) O pacote contém os fontes, não um jogo pronto para executar. Veja [instalação](INSTALL.md), [changelog](CHANGELOG.md) e [plano e verificações da release](docs/RELEASE_0.3.md).

## Arquitetura

O host em C++17 executa a biblioteca ARM original do Destinia Android com Unicorn. SDL2 apresenta o framebuffer RGB565 e recebe entradas; SDL2_mixer habilita áudio opcional. A lógica original continua na biblioteca ARM.

| Diretório | Finalidade |
| --- | --- |
| `host` | Host 400×240 |
| `host_zoom` | Resolução original ampliada, tela cheia proporcional com F11, tradução PT e trace de viewport |
| `game` | APK original do Destinia Android extraído; recursos necessários em execução, não incluídos no ZIP curado de fontes |
| `Claude` | Especificação do host e ferramentas de análise para desenvolvimento |
| `docs` | Decisões e histórico técnico; documentos antigos podem estar desatualizados |
| `release` | Manifesto e notas da candidata |
| `scripts` | Empacotamento dos fontes revisados |
| `tests` | Testes do empacotamento |

O repositório atual não inclui saves pessoais, logs de partidas, relatórios locais ou cópias antigas dos fontes. O `.gitignore` impede o envio acidental desses arquivos. A documentação técnica histórica pode citar arquivos removidos na limpeza; os fontes ativos estão em `host` e `host_zoom`.

## Recursos presentes no código

- Entradas por teclado, mouse e gamepad.
- Saves redirecionados; no Windows, o padrão é `%LOCALAPPDATA%\Destinia\save`.
- Música e efeitos quando compilado com SDL2_mixer.
- HUD de experiência, modo sem janela e capturas BMP.
- Tradução opcional `--lang pt` e trace F9 com `--trace-viewport`, apenas em `host_zoom`.

A presença no código não equivale a validação completa de gameplay. O host_zoom amplia a imagem original e oferece tela cheia com `--fullscreen` e alternância por F11. Em 3440×1440, o jogo ocupa 2400×1440 com barras laterais de 520 pixels. A correção ainda precisa de confirmação visual nas passagens do Windows; compras e rede continuam com limitações documentadas.

## Dependências

As dependências externas devem ser instaladas separadamente no PC do usuário. **Não serão incluídos no arquivo da release instaladores, ferramentas de compilação ou bibliotecas externas que o usuário precise instalar.**

| Dependência | Quando é necessária |
| --- | --- |
| Windows 64 bits e MSYS2 UCRT64 | Ambiente de execução atualmente esperado pelos launchers; instalação padrão em `C:\msys64` |
| SDL2 e Unicorn para UCRT64 | Execução do host e emulação ARM, com suas dependências transitivas disponíveis no mesmo ambiente |
| SDL2_mixer e suporte OGG/MP3 | Áudio, quando habilitado na compilação |
| Compilador C++17, CMake ≥ 3.16, Ninja e pkg-config/pkgconf | Somente para quem compilar os fontes |
| Python ≥ 3.9 | Somente para testar ou gerar o pacote de fontes |
| Windows PowerShell 5.1 | Diagnóstico pelo `Testar Destinia.bat`; normalmente já acompanha o Windows |

Instale o MSYS2 pela [página oficial](https://www.msys2.org/) e use o gerenciador de pacotes desse ambiente. Consulte [INSTALL.md](INSTALL.md) para verificar os pacotes e compilar. As versões exatas de runtime e DLLs da futura release executável ainda precisam ser confirmadas por uma compilação Windows.

A pasta `game` é o APK Android extraído, conforme informado pelo autor. Não é um programa para instalar no Windows: o host lê a biblioteca ARM, os assets e o áudio diretamente dessa pasta. Ter esses dados não substitui as dependências listadas acima.

## Desenvolvimento e distribuição

Para testar uma instalação existente, execute **[Testar Destinia.bat](Testar%20Destinia.bat)** na raiz do projeto. Ele verifica a inicialização dos dois hosts, usa cópias dos saves e reúne logs/capturas em `relatorios`. No menu, escolha **1 para o novo teste de câmera do host_zoom** ou **2 para o roteiro geral**. O modo guiado geral acompanha testes de gameplay, salvar/reabrir, câmera, mouse, áudio e controle. Veja o [roteiro completo](diagnostico/COMO_TESTAR.md). Não instala dependências nem altera o código do jogo; testes visuais dependem da sua observação.

O relatório é salvo ao longo da sessão e informa quando está parcial. Após cada execução, o diagnóstico também destaca compras observadas, alterações de cash e a rede aguardando resposta; esses avisos não significam que o problema da loja foi corrigido.

[Baixar o pacote de testes](https://raw.githubusercontent.com/sugyua/destinia/main/diagnostico/pacotes/testes-destinia.zip) e extrair na raiz do projeto existente. O pacote contém somente o BAT, seu executor PowerShell e as instruções.

Consulte [INSTALL.md](INSTALL.md) para compilar e executar com dados próprios compatíveis. Os launchers existentes continuam voltados ao ambiente de desenvolvimento MSYS2 em `C:\msys64\ucrt64\bin`; não são launchers portáteis.

Para verificar o empacotador, com Python 3.9 ou superior:

```sh
python -m unittest discover -s tests -v
python scripts/package_release.py --out dist/candidata-0.3
```

No Linux, o comando pode ser `python3`. O empacotador cria ZIP e SHA-256, não compila executáveis. Não sobrescreve artefatos existentes. A reprodução byte a byte foi verificada com a mesma versão de Python/zlib; outras versões podem mudar a compressão.

O [manifesto](release/manifest.json) usa a 0.2 Beta como base de comparação. As mudanças da 0.3 estão no changelog; zoom out, fundo cinza e UI menor ficam para depois.

## Publicação

Versão **v0.3-beta**, aprovada pelo usuário após revisão das verificações e limitações. A 0.3 é a release principal no GitHub, mas continua **Beta**, com possíveis bugs e recursos incompletos. A primeira release **v0.2-beta** permanece disponível em Releases.

O ZIP curado não inclui biblioteca original, APK, DEX, assets, áudio, saves ou DLLs de terceiros. Consulte [THIRD_PARTY.md](THIRD_PARTY.md). Não foi definida uma licença para o código do host nesta etapa.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: [+55 21 97468-6880](https://wa.me/5521974686880).
