> **AVISO: o jogo está em versão Beta e pode apresentar bugs, travamentos e funcionalidades incompletas. Faça backup dos saves antes de atualizar.**

# Destinia 0.9 Beta — FC, comparador e português padrão

## O que mudou desde a0.8

- **FC — Força de Combate** nos status dentro do jogo, baseado nos atributos derivados reais, com equipamentos e buffs ativos. Perder vida atual não reduz o índice.
- **Comparador inicial no inventário:** ao selecionar um equipamento próprio, mostra FC atual/projetado/diferença e mudanças de ATK, DEF, PV/PM máximos e crítico; ganhos em verde e perdas em vermelho. Mesmos buffs nos dois cálculos; incompatibilidades são indicadas. A previsão restaura o personagem e não grava/equipa/consome recursos. Fórmula e limites em [FC.md](https://github.com/sugyua/destinia/blob/main/docs/FC.md).
- **PT-BR padrão** no PC e na web quando não há preferência; inglês explícito permanece disponível/preservado. A tradução continua parcial.
- **Nomes de drops visíveis:** segureV no PC (remapeável como `nomes_drops`); na web useV ou o botão de nomes. Usa fontes/cores originais, sem ampliar o alcance de coleta nem modificar o inventário. A teclaV já remapeada não é ocupada.
- README simplificado, com histórico anterior nas Releases e próximos itens separados. Removidos ZIPs auxiliares antigos e cópiasWIP web já integradas; saves, dados necessários e testes ativos preservados.
- Protótipo web atualizado com as mesmas novidades. Continua privado, sem áudio e com progresso apenas na memória.

## Download

Baixe **destinia-0.9-beta-windows-x64.zip** para os dois EXEs e inicializadores, ou **destinia-0.9-beta-source.zip** para compilar. Confira o `.sha256`. Extraia em pasta nova e forneça sua pasta `game` compatível; use a mesma configuração global de saves. Abra **Abrir Destinia.bat** ou **Abrir Destinia Zoom 800x480.bat**. Fontes: **Compilar Destinia.bat**, com o jogo fechado.

Dependências externas, DLLs, dados do APK e saves pessoais não acompanham os ZIPs. Consulte **Dependências** no README e INSTALL.md. Releases anteriores são preservadas.

## Idioma e uso

**Configurar Destinia → 9 — Idioma** aceita `pt` ou `en`. `--lang en` altera somente a sessão. Na web, use o seletor de idioma; reiniciar o teste descarta o progresso temporário. UI75/100/125, tela cheia/F11, zoom out, backups e modos clássico/expandido são preservados.

## Verificações e limites

A publicação Windows exige compilação dos dois hosts com áudio, testes ARM/FC/UI/saves/preferências/loja, scripts PowerShell, render e empacotamento. Node e Chromium validam a web. A inspeção usa cenários controlados, não substitui uma campanha nem testes físicos das mudanças novas. O autor confirmou PC/iPhone da base0.8, não todos os recursos0.9.

O FC é um índice informativo inicial; não garante DPS/vitória nem cobre a utilidade de todo efeito condicional. Não há mudanças de combate, novos níveis/classes/itens nesta release. Ritmo original preservado, sem garantia de16FPS em todo aparelho.

## Transferido para0.9.1 por decisão do autor

- Filtros/ordenação de apresentação, favoritos e proteção contra venda/desmanche.
- Filtro visual de drops, coleta de ouro/materiais e poções automáticas opcionais.
- Detalhamento adicional do comparador e organização do Códex por classificação verificada.
- Reset offline consumindo os pergaminhos originais de atributos/especialidades, vendidos no vendedor de poções por **10.000gold cada**. **Ainda não implementado nesta release.**

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: +55 21 97468-6880.
