# Verificação da candidata local 0.14.0

## Escopo

Painel de diagnóstico opcional em F12 nos hosts normal e Zoom. Exibe FPS efetivo do loop, média do tempo gasto na emulação e Working Set do processo Windows. As medições só são coletadas enquanto o painel está visível; nenhuma medição altera jogo ou save.

## Verificações locais

- Teste C++ do cálculo da janela FPS, média de emulação, Working Set, descarte de amostras inválidas, alternância e renderização do painel.
- Compilação Release CMake/Ninja dos dois hosts.
- Integração F12 e pintura conferidas nos dois arquivos de host.
- Teste Windows confirmou Working Set maior que zero. Temperaturas acompanhadas por MSI Afterburner: pico CPU 55,25 °C e GPU principal 40 °C; hotspot e VRAM não entram no limite.

## Limites

Não houve teste físico de gameplay ou inspeção visual em uma partida. F12 está disponível apenas nos hosts PC. A API do Windows pode retornar zero se a consulta de Working Set falhar. Testes avançados de falhas do host/Unicorn seguem pendentes.
