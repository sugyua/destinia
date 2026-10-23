# Gravação recuperável — desenvolvimento da0.8

Esta mudança está em branch de desenvolvimento. Não é uma release0.8 completa e não foi disponibilizada no site.

Antes, fopen com modo de escrita modificava/truncava o save ativo imediatamente. Agora os dois hosts preparam o lote fora do destino e somente publicam depois da chamada ARM terminar com sucesso, com streams fechados e sem erro. Exclusões também são preparadas, e a remoção dos registros do Códex é adiada até confirmar a exclusão do personagem.

O lote usa `<pasta-do-save>.transaction`, uma pasta irmã no mesmo volume. Ela contém cópias anteriores, arquivos candidatos e um manifest limitado. Na próxima inicialização, depois da trava e antes de backup/carregamento, um lote interrompido recupera os arquivos anteriores. Lotes já confirmados apenas limpam o journal. Conteúdo desconhecido, links ou manifest corrompido são preservados e impedem carregar o jogo; não são apagados como lixo.

O rollback cobre uma chamada ARM completa do host, não uma sequência arbitrária de múltiplos frames. Uma versão antiga não entende o journal e não deve ser aberta sobre o save durante recuperação pendente. Backups de inicialização permanecem complementares. Não há conversão do formato original dos saves.

Testes usam temporários, stdio por ABI Unicorn e subprocessos encerrados no meio de substituição/exclusão. Não são campanha completa. Sincronização de arquivos e substituições foi implementada, mas queda de energia/reboot e persistência de todos os metadados de diretório não foram comprovadas. Não anunciar essa garantia.

Pendências para entregar0.8: validar salvamento em campanha/Windows, callback de loja sem cash/espera, perfis clássico/expandido, escala independente de UI/hitboxes, atalhos e português/fontes, versão e pacotes, atualização web correspondente.0.9 só será publicada depois da0.8, conforme aprovação do autor.
