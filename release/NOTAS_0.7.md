# Destinia 0.7 Beta — Códex inicial de monstros

> **AVISO: o jogo está em versão Beta e pode apresentar diversos bugs, travamentos ou funcionalidades incompletas. Faça backup dos saves.**

## O que mudou desde a 0.6

- Códex com espécies descobertas, estados visto/derrotado e **contador de mortes por espécie**, nos hosts normal e zoom.
- Botão **CODEX F8** no canto superior direito e atalho F8 quando livre; consulta com paginação e pausa da partida.
- Descobertas gravadas em `codex-v1.txt`, junto à pasta compartilhada de saves, separadas por slot; preservação entre updates que usam a mesma pasta.
- Observação passiva de rotinas ARM originais, identificação de monstro visível e proteção contra contagem repetida de uma mesma morte. Não altera XP, combate, drops ou o save nativo.
- Gravação do auxiliar por arquivo temporário e substituição; arquivo inválido preservado com aviso no log.
- Ajuste de portabilidade do PATH no script de compilação, testes e documentação em português.
- O protótipo web privado também recebeu o botão **Códex** no topo, com contador e pausa. Na web, os registros continuam apenas na memória da sessão.

## Downloads e instalação

Em **Assets**, baixe `destinia-0.7-beta-windows-x64.zip` e o checksum para usar os dois EXEs. O ZIP `destinia-0.7-beta-source.zip` contém os fontes para compilação. Instale separadamente as dependências da seção **Dependências** do README e forneça sua pasta `game` compatível. Não acompanham os ZIPs: DLLs externas, instaladores, ferramentas, APK, dados originais ou saves pessoais.

Leia `docs/CODEX.md` e `INSTALL.md`. A pasta padrão do save permanece `%LOCALAPPDATA%\Destinia\save`; não substitua o seu progresso ao atualizar.

## Testes e limites

A publicação automática só ocorre após compilação e testes Windows do workflow. Testes do modelo e callbacks ARM com estruturas controladas não equivalem a uma campanha completa. A inspeção manual no Windows e no Safari/iPhone físico permanece pendente. O Códex inicia sem histórico antigo, usa os nomes originais em inglês e ainda não inclui equipamentos. Carregar um save antigo não desfaz contagens de eventos já registradas. Leia as limitações de importação de personagens por slot em `docs/CODEX.md`.

Escala independente da UI, ocultar botões virtuais, atalhos nas habilidades e expansão da tradução ficaram para a 0.8. O Códex completo permanece parcialmente entregue no roteiro; armas/armaduras e integração com drops exclusivos continuam previstos para a 0.11.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: [+55 21 97468-6880](https://wa.me/5521974686880).
