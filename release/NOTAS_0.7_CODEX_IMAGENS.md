> **AVISO: o jogo está em versão Beta e pode apresentar diversos bugs, travamentos e funcionalidades incompletas. Faça backup dos saves antes de atualizar.**

Esta é a compilação atualizada da **0.7 Beta**, com o Códex dentro do jogo e imagens dos monstros. Os dois executáveis Windows x64 já estão compilados.

## Download para jogar no Windows

Baixe **destinia-0.7-beta-windows-x64.zip**, na seção Assets abaixo. O pacote contém os hosts normal e zoom e seus inicializadores BAT. Não é necessário compilar.

1. Extraia o ZIP em uma pasta nova.
2. Copie a pasta `game` da sua instalação anterior para essa pasta nova. Ela contém os dados originais do APK extraído e não acompanha este download.
3. Abra **Abrir Destinia.bat** ou **Abrir Destinia Zoom 800x480.bat**.
4. Durante a partida, abra **Menu → CODEX**, junto dos ícones de Craft e Pets.

Os registros de progresso e `codex-v1.txt` continuam na pasta compartilhada de saves configurada. O padrão é `%LOCALAPPDATA%\Destinia\save`. Mantenha a mesma configuração de pasta se você usa um local personalizado. Atualizar os EXEs não exige começar outra partida.

## O que mudou desde a release original 0.7

- Códex transferido para o menu do próprio jogo, junto de Craft/Pets, com livro CODEX à direita da barra de ícones.
- Lista, estado visto/derrotado, contador de mortes, paginação e VOLTAR desenhados dentro da imagem do jogo.
- Imagem original ao lado do nome de cada monstro descoberto, incluindo variantes e chefes. Montagem das partes dos sprites originais e redução proporcional, preservando cores e transparência.
- Quatro monstros por página para deixar as imagens legíveis; navegação por mouse/toque, teclado e controle.
- Consulta suspende a partida, preserva os contadores e retorna à opção anterior do menu. A abertura libera um toque ainda pressionado para evitar comandos presos.
- Cache das imagens e tratamento de recursos ausentes ou incompatíveis: aparece `?`, enquanto nome, estado e contador continuam disponíveis.
- Leitura dos retratos separada da execução ARM: nenhuma alteração de combate, XP, drops ou formato do save.
- Documentação, changelog, instruções de dependências e testes atualizados. A versão web também recebeu as miniaturas dentro do Códex.

## Dependências

Instale separadamente os pacotes do **MSYS2 UCRT64**: SDL2, Unicorn, SDL2_mixer e **zlib**, com suas dependências transitivas. O comando atualizado está em **INSTALL.md**; a lista está na seção **Dependências** do **README.md**. Se o seu ambiente já possui zlib como dependência de outra biblioteca, não precisa reinstalá-la.

O ZIP não inclui DLLs, instaladores, ferramentas externas, dados originais do jogo nem saves pessoais. O ZIP `source` é destinado a quem quer consultar ou compilar os fontes.

## Verificações

- Compilação dos dois EXEs e testes automatizados no Windows aprovados no [workflow do GitHub](https://github.com/sugyua/destinia/actions/runs/39676483793).
- 34 testes locais dos hosts e 14 testes web aprovados; verificação por toque em retrato/paisagem no Chromium.
- Leitura dos 73 IDs originais de monstros/chefes; cores, transparência, variantes, paginação e preservação de contadores/memória verificadas.
- ZIPs e SHA-256 conferidos; dois EXEs Windows x64 presentes, sem bibliotecas externas ou saves.
- Campanha completa no Windows, controle físico e Safari/iPhone físico ainda dependem de validação manual.

Compilação: `ba18cede75f68ae163754ba6cc03ed1cc6de1004`. O commit seguinte na main altera somente a proteção do workflow de publicação e seu registro no changelog. A release original `v0.7-beta` permanece preservada.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: [+55 21 97468-6880](https://wa.me/5521974686880).
