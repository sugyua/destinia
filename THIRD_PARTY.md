# Componentes originais e dependências

`AGENTS.md` exige revisão e autorização antes de enviar arquivos originais de terceiros. O pacote curado desta candidata segue essa restrição.

## Dados originais do jogo

A pasta `game` é o APK original do Destinia Android extraído, conforme informado pelo autor. Contém biblioteca ARM, DEX, recursos e dados do jogo original. Esses arquivos **não integram o ZIP curado de fontes**. O usuário precisa fornecer dados compatíveis para executar o host.

Esta preparação não estabelece autorização de redistribuição desses dados, não altera sua licença e não inclui chaves ou certificados do APK no pacote. Antes de publicar uma distribuição completa, revisar a autorização aplicável aos arquivos originais.

## Bibliotecas do host

Na 0.12.1, o ZIP Windows e os fontes curados não distribuem DLLs nem `web/dist/vendor/unicorn_arm.js`. A exportação automática da tag também exclui esse JavaScript. O arquivo permanece no repositório de desenvolvimento; origem, licença e alterações da ponte constam em `web/dist/vendor/ORIGEM.txt`, `UNICORN-LICENSE.txt` e `BRIDGE-PATCH.txt`. A reprodução da Web exige fornecer essa dependência local separadamente, além dos dados originais. Releases antigas não foram alteradas.

| Componente | Uso |
| --- | --- |
| SDL2 | Janela, framebuffer, entradas e inicialização de áudio |
| SDL2_mixer | Música e efeitos; dependência opcional |
| Unicorn | Emulação da CPU ARM |
| zlib | Descompressão dos sprites originais do Códex; dependência externa das compilações atuais |
| Runtime UCRT64/MinGW e codecs | Dependências do executável e das bibliotecas compiladas |

Nenhuma DLL foi incluída nesta candidata. Por orientação do usuário, dependências externas que exigem instalação no PC devem ficar fora do arquivo da release e ser informadas na seção **Dependências** do README. Isso inclui instaladores, ferramentas de compilação e bibliotecas/runtime externos.

Para um executável futuro, identificar e documentar as versões e dependências transitivas efetivamente usadas, com instruções de instalação separada. Revisar as licenças aplicáveis aos componentes efetivamente distribuídos. Não presumir que instalar apenas SDL2 e Unicorn resolve todas as dependências.

## Código do host

Não foi encontrada uma licença de projeto na árvore consultada. Não foi atribuída uma licença ao código sem decisão do autor. A publicação de uma release e a escolha de uma licença são decisões distintas.

## Limite do pacote curado

Os ZIP/TAR automáticos gerados pelo GitHub para uma tag são distintos do ZIP curado. Esta versão adiciona `.gitattributes` com `export-ignore` para excluir `game` desses arquivos automáticos, além das cópias históricas e logs selecionados. O conteúdo exportado é verificado antes da publicação. Isso não remove arquivos da árvore Git ou do histórico: `game` permanece no repositório.
