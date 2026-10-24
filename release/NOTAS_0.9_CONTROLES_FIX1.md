> **AVISO: o jogo está em versão Beta e pode apresentar bugs. Faça backup dos saves.**

# Destinia 0.9 Beta — controles PC (fix1)

Atualização corretiva da0.9; as funcionalidades restantes da0.9.1 continuam pendentes.

- L3+R3 alterna mostrar/ocultar controles virtuais e suas áreas de clique durante gameplay, nos dois hosts. Solte ambos antes de alternar novamente.
- HUD, menus, teclado e controle continuam ativos. A combinação não dispara os remapeamentos individuais; cliques isolados de L3/R3 remapeados disparam ao soltar.
- Reutiliza a opção6 de Configurar Destinia, gravada separadamente por host, preservando as demais configurações. --no-save-preferences limita a mudança à sessão. A opção web é independente.
- README destaca o link para a [versão web](https://destinia-web-experimental.leafyheron257.chatgpt.site/), já atualizada com Gamepad API padrão. O site permanece privado, sem áudio/progresso persistente. USB no iPhone ainda exige teste físico de controle/adaptador/Safari.

## Download e instalação

Baixe **destinia-0.9-beta-windows-x64.zip** nesta Release; contém os dois EXEs atualizados e inicializadores. Não precisa compilar. O ZIP source é opcional para desenvolvimento. Verifique os arquivos .sha256.

Extraia em pasta nova, forneça seus dados compatíveis na pasta game e use a mesma configuração global de saves. Abra Abrir Destinia.bat ou Abrir Destinia Zoom 800x480.bat. Dependências externas, DLLs, APK e saves pessoais não acompanham os pacotes; consulte Dependências no README e INSTALL.md. A Release0.9 anterior permanece disponível.

## Validação

EXEs idênticos aos aprovados no commitff840589b6cf98e161cfd33ccadf8013f1a87eb8, CIWindows40114929492: compilação/áudio dos dois hosts, ARM, controles, preferências, PowerShell, render e empacotamento. Pacotes reúnem esses EXEs com documentação atualizada. Linux48aprovados/1render reservado ao Windows; web57aprovados e Chromium emulado aprovados. Não substitui teste físico Windows/iPhone.

Filtros, favoritos, automações e reset por pergaminho permanecem pendentes para0.9.1; não estão nesta correção.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: +55 21 97468-6880.
