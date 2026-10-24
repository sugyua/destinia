[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'game_preferences.ps1')
try {
    if(-not $env:LOCALAPPDATA) {throw 'LOCALAPPDATA ausente.'}
    $root=Split-Path $PSScriptRoot -Parent
    $config=Join-Path $env:LOCALAPPDATA 'Destinia/config.ini'
    $executables=@((Join-Path $root 'host/build/destinia_host.exe'),(Join-Path $root 'host_zoom/build/destinia_host.exe'))
    foreach($exe in $executables) {if(-not (Test-Path -LiteralPath $exe)) {throw 'Compile os dois hosts ou baixe o pacote Windows antes de configurar.'}}
    $msys=$env:DESTINIA_MSYS2_ROOT;if(-not $msys) {$msys='C:\msys64'}
    $env:PATH=(Join-Path $msys 'ucrt64/bin')+[IO.Path]::PathSeparator+$env:PATH
    $validate={param($file)
        foreach($exe in $executables) {
            $output=& $exe --check-settings $file 2>&1
            if($LASTEXITCODE -ne 0) {throw ($output -join "`n")}
        }
    }
    Write-Host 'DESTINIA - PREFERENCIAS DOS DOIS HOSTS'
    Write-Host 'Feche os jogos antes de alterar. As preferencias valem tambem nas proximas atualizacoes.'
    while($true) {
        Write-Host "`n[1] Video [2] Audio [3] Pausa [4] Perfil e comandos [5] Restaurar padroes [6] Controles virtuais [7] Modo de jogo [8] Tamanho da interface [9] Idioma [0] Sair"
        $option=Read-Host 'Opcao'
        if($option -eq '0') {break}
        try {
            $changes=@{}
            switch($option) {
                '1' {
                    $hostName=Read-Host 'Host: 1 normal ou 2 zoom'
                    if($hostName -eq '1') {$hostName='host'} elseif($hostName -eq '2') {$hostName='host_zoom'} else {throw 'Escolha 1 ou 2.'}
                    $changes["video.$hostName.largura"]=Read-Host 'Largura da janela (400 a 7680)'
                    $changes["video.$hostName.altura"]=Read-Host 'Altura da janela (240 a 4320)'
                    $changes["video.$hostName.tela_cheia"]=Read-Host 'Abrir em tela cheia: 1 sim, 0 nao'
                }
                '2' {
                    $changes['audio.musica']=Read-Host 'Volume da musica (0 a 100)'
                    $changes['audio.efeitos']=Read-Host 'Volume dos efeitos (0 a 100)'
                    $changes['audio.silenciar_sem_foco']=Read-Host 'Silenciar ao usar Alt+Tab: 1 sim, 0 nao'
                }
                '3' {
                    $changes['pausa.sem_foco']=Read-Host 'Pausar ao perder foco: 1 sim, 0 nao'
                    $changes['pausa.sem_controle']=Read-Host 'Pausar ao desconectar controle: 1 sim, 0 nao'
                    Write-Host 'F10 pausa/retoma por padrao; apos desconectar, permite continuar pelo teclado.'
                }
                '4' {
                    $profile=Read-Host 'Nome do perfil (letras sem acento, numeros, _ ou -, ate 32 caracteres)'
                    if($profile -cnotmatch '^[A-Za-z0-9_-]{1,32}$') {throw 'Nome de perfil invalido.'}
                    $changes['controle_perfil']=$profile
                    Write-Host 'Enter na acao apenas seleciona o perfil. Cada perfil novo herda os comandos padrao.'
                    Write-Host 'Acoes: cima, baixo, esquerda, direita, ataque, voltar, inventario, iniciar, habilidade_esquerda, habilidade_direita, habilidade_cima, habilidade_baixo, acao_extra, salvar, acelerar, pausar, nomes_drops.'
                    $action=Read-Host 'Acao a remapear'
                    if($action) {
                        if($action -cnotmatch '^(cima|baixo|esquerda|direita|ataque|voltar|inventario|iniciar|habilidade_(esquerda|direita|cima|baixo)|acao_extra|salvar|acelerar|pausar|nomes_drops)$') {throw 'Acao desconhecida.'}
                        $device=Read-Host 'Dispositivo: tecla ou botao'
                        if($device -cnotmatch '^(tecla|botao)$') {throw 'Escolha tecla ou botao.'}
                        Write-Host 'Teclas SDL em ingles: Return, Space, Z, W, Up, F5, F10, Left Shift. F9/F11 reservados.'
                        Write-Host 'Botoes: a, b, x, y, back, start, leftshoulder, rightshoulder, dpup, dpdown, dpleft, dpright.'
                        Write-Host 'Separe alternativas por virgula ou use none para desassociar. Conflitos serao rejeitados.'
                        $changes["perfil.$profile.$device.$action"]=Read-Host 'Novos comandos'
                    }
                }
                '6' {
                    $hostName=Read-Host 'Host: 1 normal ou 2 zoom'
                    if($hostName -eq '1') {$hostName='host'} elseif($hostName -eq '2') {$hostName='host_zoom'} else {throw 'Escolha 1 ou 2.'}
                    $changes["interface.$hostName.controles_virtuais"]=Read-Host 'Mostrar controles virtuais na partida: 1 sim, 0 nao'
                    Write-Host 'Menus mantem seus botoes. Teclado e controle continuam disponiveis. Reinicie o jogo para aplicar.'
                }
                '9' {$changes['idioma']=Read-Host 'Idioma: en ou pt (traducao parcial, reinicie o jogo)'}
                '8' {
                    $hostName=Read-Host 'Host: 1 normal ou 2 zoom'
                    if($hostName -eq '1') {$hostName='host'} elseif($hostName -eq '2') {$hostName='host_zoom'} else {throw 'Escolha 1 ou 2.'}
                    $changes["interface.$hostName.escala"]=Read-Host 'Tamanho da UI: 75, 100 ou 125 (%)'
                    Write-Host 'Mapa e XP mantem o tamanho atual. Paineis grandes limitam a ampliacao para caber na tela. Reinicie para aplicar.'
                }
                '7' {
                    $mode=Read-Host 'Modo: 1 classico, 2 expandido (partida separada)'
                    if($mode -eq '1') {$changes['jogo.modo']='classico'}
                    elseif($mode -eq '2') {
                        Write-Host 'Expandido prepara um save separado para futuras expansoes; ainda usa o conteudo e limites originais. Nao aumenta nivel nem cria itens nesta versao.'
                        Write-Host 'A copia e opcional, somente para destino novo. Depois as partidas divergem. Nada sera sobrescrito.'
                        if((Read-Host 'Copiar a partida classica agora? Digite S para autorizar') -match '^[sS]$') {
                            & $executables[0] --import-classic
                            if($LASTEXITCODE -ne 0) {throw 'Importacao nao concluida; o modo nao foi alterado.'}
                        }
                        $changes['jogo.modo']='expandido'
                    } else {throw 'Escolha 1 ou 2.'}
                }
                '5' {Reset-DestiniaPreferences $config $validate;Write-Host 'Preferencias restauradas. Pasta de saves preservada.';continue}
                default {throw 'Opcao desconhecida.'}
            }
            Set-DestiniaPreferences $config $changes $validate
            Write-Host ('Preferencias salvas: '+$config)
        } catch {Write-Host ('Nao foi alterado: '+$_.Exception.Message)}
    }
    exit 0
} catch {Write-Host ('FALHA: '+$_.Exception.Message);exit 1}
