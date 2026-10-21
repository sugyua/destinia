> **AVISO: o jogo está em versão Beta e pode apresentar bugs. Faça backup dos saves antes de atualizar.**

Correção dos inicializadores da **0.7 Beta com imagens no Códex**. Os EXEs e a lógica do jogo são os mesmos já verificados; esta atualização corrige os arquivos usados para abrir e testar o jogo.

## Para corrigir sua instalação atual

Baixe **destinia-0.7-corrigir-abertura.zip** e extraia diretamente na raiz da instalação, por exemplo `C:\destinia`, substituindo os quatro arquivos:

- Abrir Destinia.bat
- Abrir Destinia Zoom 800x480.bat
- VERSION
- diagnostico/test_suite.ps1

Depois abra **Abrir Destinia Zoom 800x480.bat** ou **Abrir Destinia.bat**. Não precisa compilar nem substituir a pasta game ou os saves. O patch pressupõe uma instalação 0.7 com EXEs correspondentes. Para evitar mistura com arquivos antigos, há também o pacote Windows completo abaixo.

## Pacote completo

**destinia-0.7-beta-windows-x64.zip** contém os dois EXEs prontos e os BATs corrigidos. Extraia em uma pasta nova e copie sua pasta `game` original para ela. As dependências externas ficam instaladas no MSYS2 UCRT64; consulte README.md e INSTALL.md. Mantenha a mesma configuração da pasta compartilhada de saves para continuar seu progresso.

## O que foi corrigido

- Os BATs exigiam o texto `Destinia 0.6 Beta` e recusavam os EXEs 0.7 válidos. Agora leem VERSION e mantêm a validação de compatibilidade/runtime.
- VERSION ausente/inválido passa a gerar uma mensagem específica.
- Resposta vazia à pergunta de abrir/reabrir o teste não é mais considerada N; a pergunta é repetida até S ou N explícito. Para abrir a janela, digite **S e Enter**.
- Relatório destaca os testes pendentes/não executados mesmo quando o código final é zero. Inicialização sem janela não comprova abertura visual nem gameplay.
- Testes automatizados dos inicializadores no CMD do Windows e do fluxo de respostas do diagnóstico.

Nenhum ajuste de combate, câmera, UI, Códex, progresso ou formato de save. As miniaturas, quatro monstros por página e contadores da atualização anterior permanecem.

Os EXEs reutilizados são os da compilação aprovada `ba18cede75f68ae163754ba6cc03ed1cc6de1004`; BUILD_INFO registra esse commit de compilação. Os arquivos de abertura/diagnóstico e a documentação correspondem à tag fix1. Pacotes anteriores permanecem preservados. Bibliotecas externas, dados originais e saves pessoais não acompanham os downloads.

Se quiser ajudar no projeto, entre em contato comigo pelo WhatsApp: [+55 21 97468-6880](https://wa.me/5521974686880).
