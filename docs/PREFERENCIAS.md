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
