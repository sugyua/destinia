# Preferencias da 0.6 Beta

Abra `Configurar Destinia.bat` com os dois jogos fechados. Video separado por host, audio, pausa e perfis de comandos ficam em `%LOCALAPPDATA%\Destinia\config.ini`, junto da configuracao de save. Os ajustes valem entre instalacoes 0.6 ou posteriores; jogos antigos ignoram essas preferencias.

O inicializador normal abre em janela 1200x720 e o zoom em tela cheia com janela 800x480 por padrao. F11 alterna o modo. Tamanho e modo sao lembrados ao fechar normalmente. A imagem preserva a proporcao e os cliques acompanham a area central. Nao ha mudanca de resolucao fisica do monitor, camera ou UI nativa.

Musica e efeitos possuem volumes de 0 a 100%, multiplicados pelo volume escolhido no menu original. Silenciar ao perder foco e pausar ao perder foco ou controle sao opcionais e vem desativados. F10 pausa/retoma por padrao. Desconectar um controle libera comandos; F10 permite continuar pelo teclado. Recoloque os analogicos no centro antes de voltar a usa-los.

Cada perfil novo herda os comandos originais. O configurador aceita teclas SDL em ingles, alternativas separadas por virgula e `none` para desassociar. F9/F11 sao reservados. Para trocar duas teclas ja ocupadas, primeiro desassocie uma das acoes ou edite as duas linhas no config.ini com o jogo fechado. Os hosts rejeitam configuracoes invalidas antes de abrir o save. Os analogicos mantem movimento/habilidades e zona morta originais.

Exemplo de configuracao (nao substitua a linha `save_dir` existente):

```ini
audio.musica=60
audio.efeitos=85
audio.silenciar_sem_foco=1
pausa.sem_foco=1
pausa.sem_controle=1
controle_perfil=meu_controle
perfil.meu_controle.tecla.ataque=Return,Space,Z
perfil.meu_controle.botao.ataque=a
video.host.largura=1200
video.host.altura=720
video.host.tela_cheia=0
video.host_zoom.largura=800
video.host_zoom.altura=480
video.host_zoom.tela_cheia=1
```

Acoes: `cima`, `baixo`, `esquerda`, `direita`, `ataque`, `voltar`, `inventario`, `iniciar`, `habilidade_esquerda`, `habilidade_direita`, `habilidade_cima`, `habilidade_baixo`, `acao_extra`, `salvar`, `acelerar`, `pausar`. Botoes SDL: `a`, `b`, `x`, `y`, `back`, `start`, `leftshoulder`, `rightshoulder`, `dpup`, `dpdown`, `dpleft`, `dpright`.

Restaurar padroes remove apenas preferencias conhecidas e perfis; preserva a pasta de saves e chaves desconhecidas. O configurador guarda a configuracao anterior em `config.ini.previous`. Nao restaure esse arquivo sem conferir `save_dir`.

Opcoes avancadas: `--settings ARQUIVO` seleciona preferencias, `--check-settings ARQUIVO` valida sem abrir jogo/save, `--windowed`/`--fullscreen` prevalecem sobre modo salvo e `--scale N` sobre tamanho salvo. `--no-save-preferences` impede gravar video ao sair, usado pelo diagnostico. O local do save continua seguindo `--save`, `DESTINIA_SAVE_DIR`, `save_dir` global e padrao, nessa ordem; `--settings` nao troca a configuracao de save.

## Em preparação para 0.8 — controles virtuais

Com os dois jogos fechados, abra `Configurar Destinia.bat`, escolha **6 — Controles virtuais**, selecione o host normal ou zoom e informe **0** para ocultar ou **1** para mostrar. Reinicie o jogo para aplicar. O padrão continua sendo mostrar.

A escolha é independente para cada host e permanece entre instalações que suportem a opção:

```ini
interface.host.controles_virtuais=1
interface.host_zoom.controles_virtuais=0
```

Durante a partida, a opção oculta os controles virtuais e desativa suas áreas de toque. Teclado e controle físico continuam disponíveis; os botões de navegação dos menus permanecem. A escala do HUD e o zoom do cenário não mudam nesta primeira etapa. Restaurar padrões volta a mostrar os controles e preserva `save_dir`.

Implementação nativa verificada com a biblioteca Android 1.0.6 conhecida. Uma biblioteca incompatível é rejeitada ao solicitar ocultação, em vez de aplicar interceptações desconhecidas. Verificações automáticas locais não substituem o teste visual e de campanha no Windows. A versão web publicada também oferece o botão **Ocultar botões do jogo**, com painel externo de toque preservado e preferência própria do navegador. Não compartilha configuração nem saves com Windows. Compilação e configurador PowerShell passaram na [CI Windows](https://github.com/sugyua/destinia/actions/runs/39933223943); campanha e inspeção visual no PC seguem pendentes.


## Em preparação para 0.8 — modos de jogo

Em **7 — Modo de jogo**, escolha clássico ou expandido. O clássico continua na pasta de save já configurada e mantém seu progresso. O expandido usa uma pasta irmã com sufixo `_expandido`, separada entre modos e compartilhada entre instalações/hosts compatíveis. Ambos ainda usam o conteúdo e os limites originais nesta versão; a separação prepara expansões futuras.

A opção **Copiar a partida clássica agora?** só copia após você responder **S**. Exige destino novo, recusa uma partida expandida já existente e não substitui arquivos clássicos. Depois da cópia, os progressos passam a divergir. Feche os jogos antes de importar. Cópia recusada não muda o modo selecionado. Para começar expandido do zero, responda **N**.

```ini
jogo.modo=classico
```

Valores aceitos: `classico` e `expandido`. O perfil de comandos (`controle_perfil`) é independente do modo de jogo. Restaurar padrões volta ao clássico e mantém as duas pastas de progresso. O marcador `.destinia-mode` impede carregar um save identificado como pertencente ao outro modo. Não apague esse marcador nem aponte versões antigas à pasta expandida. `--save` continua escolhendo a pasta exata de diagnóstico, com validação do modo; `--import-classic` é a ação explícita de cópia para a pasta expandida global.

Na web, o seletor **Modo** encerra a sessão atual mediante confirmação e abre uma partida vazia em memória. A seleção é lembrada, mas o progresso não é salvo no navegador nem importado do PC.

## Opções da 0.8

O configurador oferece 6 controles virtuais, 7 modo clássico/expandido, 8 escala UI75/100/125 e 9 idioma inglês/português parcial. A escala não muda o cenário nem o XP. O modo expandido prepara futuras alterações e usa save separado; importação explícita somente para destino novo. Idioma é global; UI/controles são por host. Restaurar padrões preserva o caminho de saves. Consulte [RELEASE_0.8.md](RELEASE_0.8.md).

## Desenvolvimento0.9

PT-BR é o padrão quando não há idioma configurado; inglês explícito é preservado. Para inglês use opção9/`idioma=en` ou `--lang en` (somente a sessão). `nomes_drops` permite mostrar os nomes/cores originais dos drops visíveis segurandoV; se V já estiver remapeado, esse padrão não ocupa a tecla. Configure outra tecla/botão na opção4. Perda de foco/pausa libera o comando; a coleta mantém seu alcance original. No navegador há botão de alternância e V para segurar.
