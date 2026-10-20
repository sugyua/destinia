# Destinia para Windows — 0.2 Beta

> **O jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves antes de atualizar.**

**Primeira release: 0.2 Beta, distribuída em código-fonte.** O pacote contém os fontes, não um jogo pronto para executar. Veja [instalação](INSTALL.md), [changelog](CHANGELOG.md) e [plano e verificações da release](docs/RELEASE_0.2.md).

## Arquitetura

O host em C++17 executa a biblioteca ARM original do Destinia Android com Unicorn. SDL2 apresenta o framebuffer RGB565 e recebe entradas; SDL2_mixer habilita áudio opcional. A lógica original continua na biblioteca ARM.

| Diretório | Finalidade |
| --- | --- |
| `host` | Host 400×240 |
| `host_zoom` | Host experimental 800×480, tradução PT e trace de viewport |
| `game` | APK original do Destinia Android extraído; recursos necessários em execução, não incluídos no ZIP curado de fontes |
| `Claude` | Referência histórica, não é o host atual |
| `docs` | Decisões e histórico técnico; documentos antigos podem estar desatualizados |
| `release` | Manifesto e notas da candidata |
| `scripts` | Empacotamento dos fontes revisados |
| `tests` | Testes do empacotamento |

No repositório completo há backups e logs históricos; o pacote curado usa uma lista explícita de arquivos para não incluí-los.

## Recursos presentes no código

- Entradas por teclado, mouse e gamepad.
- Saves redirecionados; no Windows, o padrão é `%LOCALAPPDATA%\Destinia\save`.
- Música e efeitos quando compilado com SDL2_mixer.
- HUD de experiência, modo sem janela e capturas BMP.
- Tradução opcional `--lang pt` e trace F9 com `--trace-viewport`, apenas em `host_zoom`.

A presença no código não equivale a validação completa de gameplay. Câmera/transição em 800×480, compras e rede continuam com limitações documentadas.

## Dependências

As dependências externas devem ser instaladas separadamente no PC do usuário. **Não serão incluídos no arquivo da release instaladores, ferramentas de compilação ou bibliotecas externas que o usuário precise instalar.**

| Dependência | Quando é necessária |
| --- | --- |
| Windows 64 bits e MSYS2 UCRT64 | Ambiente de execução atualmente esperado pelos launchers; instalação padrão em `C:\msys64` |
| SDL2 e Unicorn para UCRT64 | Execução do host e emulação ARM, com suas dependências transitivas disponíveis no mesmo ambiente |
| SDL2_mixer e suporte OGG/MP3 | Áudio, quando habilitado na compilação |
| Compilador C++17, CMake ≥ 3.16, Ninja e pkg-config/pkgconf | Somente para quem compilar os fontes |
| Python ≥ 3.9 | Somente para testar ou gerar o pacote de fontes |

Instale o MSYS2 pela [página oficial](https://www.msys2.org/) e use o gerenciador de pacotes desse ambiente. Consulte [INSTALL.md](INSTALL.md) para verificar os pacotes e compilar. As versões exatas de runtime e DLLs da futura release executável ainda precisam ser confirmadas por uma compilação Windows.

A pasta `game` é o APK Android extraído, conforme informado pelo autor. Não é um programa para instalar no Windows: o host lê a biblioteca ARM, os assets e o áudio diretamente dessa pasta. Ter esses dados não substitui as dependências listadas acima.

## Desenvolvimento e distribuição

Consulte [INSTALL.md](INSTALL.md) para compilar e executar com dados próprios compatíveis. Os launchers existentes continuam voltados ao ambiente de desenvolvimento MSYS2 em `C:\msys64\ucrt64\bin`; não são launchers portáteis.

Para verificar o empacotador, com Python 3.9 ou superior:

```sh
python -m unittest discover -s tests -v
python scripts/package_release.py --out dist/candidata-0.2
```

No Linux, o comando pode ser `python3`. O empacotador cria ZIP e SHA-256, não compila executáveis. Não sobrescreve artefatos existentes. A reprodução byte a byte foi verificada com a mesma versão de Python/zlib; outras versões podem mudar a compressão.

O [manifesto](release/manifest.json) identifica o commit base anterior à 0.2. O pacote inclui as alterações desta versão, registradas na tag `v0.2-beta`.

## Publicação

Versão **v0.2-beta**, primeira release do port, marcada como **pré-lançamento** no GitHub e aprovada pelo usuário após revisão das verificações e limitações.

O ZIP curado não inclui biblioteca original, APK, DEX, assets, áudio, saves ou DLLs de terceiros. Consulte [THIRD_PARTY.md](THIRD_PARTY.md). Não foi definida uma licença para o código do host nesta etapa.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: [+55 21 97468-6880](https://wa.me/5521974686880).
