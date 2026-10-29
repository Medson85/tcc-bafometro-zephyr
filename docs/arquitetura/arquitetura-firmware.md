# Arquitetura do software embarcado

Base: escopo consolidado v2 (24/07/2026), conferido contra o código do Zephyr 4.4.0 em 28/10/2026. As decisões vinculantes estão na primeira seção. O restante é o desenho do escopo v2 e pode ser ajustado durante a implementação; itens marcados como **proposta** aguardam confirmação.

## Decisões vinculantes

- linguagem C e alocação estática;
- uma thread de aplicação (`main`) e máquina de estados plana com SMF (`CONFIG_SMF`, sem `CONFIG_SMF_ANCESTOR_SUPPORT`);
- uma `k_msgq` para todos os eventos e um único `k_timer` reutilizado;
- sem camada de abstração de hardware própria: hardware descrito por devicetree, com um overlay por placa;
- nomes em português: camelCase em variáveis e funções, prefixo de módulo nas funções públicas (`sensor_calcularRs`), PascalCase em tipos (`ResultadoMedicao`), macros em maiúsculas com a unidade (`TEMPO_SOPRO_MINIMO_MS`) e guardas `BAFOMETRO_<ARQUIVO>_H`;
- fases do aquecedor: Q7 baixo e Q5 alto no aquecimento; Q7 alto e Q5 baixo na operação;
- sopro válido com 5 s sustentados;
- inibidor *fail-active*, com saída mantida e histerese na liberação;
- modo economia por ociosidade e tempo mínimo de recuperação entre testes;
- R0 persistido entre ligações;
- Zephyr 4.4.0 fixado ([ADR 0001](decisoes/0001-versao-do-zephyr.md)).

O código convive com o `snake_case` das APIs do Zephyr apenas na fronteira de chamada.

## Alvos de compilação

| Alvo | Uso |
|---|---|
| `esp32c3_devkitm/esp32c3` | Protótipo RISC-V |
| `weact_stm32wb55_core/stm32wb55xx` | Protótipo Arm |
| `native_sim` | Domínio e máquina de estados sem hardware; exige Linux (WSL 2 no Windows) |

Os overlays ficam em `app/boards/` com o nome da placa (`esp32c3_devkitm.overlay`, `weact_stm32wb55_core.overlay`); o Zephyr aceita o nome sem o qualificador do SoC.

Nomes sensíveis à versão: no Zephyr 4.4, `solomon,ssd1327fb` passou a `solomon,ssd1327` e `CONFIG_SSD1327` passou a `CONFIG_SSD1327_5`. Instruções copiadas de outras versões devem ser conferidas contra a 4.4.0.

## Organização dos arquivos (proposta)

```text
app/
  CMakeLists.txt
  prj.conf
  boards/
    esp32c3_devkitm.overlay
    weact_stm32wb55_core.overlay
    native_sim.overlay
  src/
    main.c
    config_produto.h          fonte única dos parâmetros
    aplicacao/
      bafometro.c, .h         máquina de estados, fila e temporizador
    dominio/                  sem dependência do Zephyr
      filtro.h                contrato do filtro
      filtro_media_movel.c
      estatistica.c
      conversao.c             tabela Rs/R0 -> concentração
      qualidade.c
      decisao.c               decisão de inibição com histerese
    dispositivos/
      sensor_alcool.c         fases Q5/Q7, ADC e persistência do R0
      detector_sopro.c
      inibidor.c
      tela.c
      ble.c
tests/
  dominio/
  maquina/
```

Os pinos ficam no nó `zephyr,user` do devicetree (ver [definições de hardware](../hardware.md)). Um binding próprio só entra se a interface virar componente reutilizável.

## Máquina de estados

Regras do SMF, conferidas no `include/zephyr/smf.h` da 4.4.0:

- estados criados com `SMF_CREATE_STATE(entrada, execucao, saida, NULL, NULL)`;
- a ação de execução retorna `SMF_EVENT_HANDLED`;
- `smf_set_state()` só em entrada ou execução; chamado na saída, registra o erro `SMF_ERR_TRANSITION_IN_EXIT` e não transiciona;
- nenhuma espera dentro dos estados: todo tempo chega como evento do `k_timer`.

| Estado | Na entrada | Eventos e transições |
|---|---|---|
| INICIALIZANDO | Inibidor **ativo**; valida dispositivos; carrega R0 persistido (ou padrão + flag `naoCalibrado`) | ok → PRE_AQUECENDO · falha → ERRO |
| PRE_AQUECENDO | Fase de aquecimento (Q7=0, Q5=1); temporizador de 10 s; contagem na tela (tique de 1 s) | tempo → AGUARDANDO_SOPRO, ou CALIBRANDO_R0 se veio de comando |
| AGUARDANDO_SOPRO | Fase de operação (Q7=1, Q5=0) após acomodação; arma a interrupção do detector; temporizador de 20 s; tela "Sopre"; estado no BLE | sopro → COLETANDO · tempo → ECONOMIA · calibrar → CALIBRANDO_R0 · cancelar → ECONOMIA |
| COLETANDO | Desarma a interrupção; zera coleta e filtro; temporizador periódico de 50 ms | Em cada tique: lê o ADC (mV), alimenta a média móvel e lê o detector. Queda acumulada acima da tolerância antes de 5 s → mensagem de repetição e AGUARDANDO_SOPRO · 5 s completos → PROCESSANDO · falha de ADC → ERRO |
| PROCESSANDO | Para o temporizador; estatística do trecho estável (após o transiente de 2 s); Rs, Rs/R0, concentração, qualidade e decisão de inibição | válido → APRESENTANDO_RESULTADO · qualidade insuficiente → mensagem e AGUARDANDO_SOPRO · erro → ERRO |
| APRESENTANDO_RESULTADO | Tela com mg/L e qualidade; notificação BLE; **atua o inibidor** conforme a decisão; temporizador de apresentação | tempo ou repetir → FINALIZANDO |
| FINALIZANDO | Limpa dados transitórios; registra no histórico opcional; impõe o tempo mínimo entre testes | pronto → AGUARDANDO_SOPRO |
| ECONOMIA | Fase de repouso (combinação pendente no esquemático); display apagado; anúncio BLE lento; detector armado como despertar | sopro ou comando → PRE_AQUECENDO |
| CALIBRANDO_R0 | Só por comando, com sensor quente; 60 amostras a cada 100 ms; valida faixa; grava só após sucesso | fim → AGUARDANDO_SOPRO (sucesso, ou mantém o anterior com aviso) |
| ERRO | Inibidor **ativo**; aquecedor em repouso; código de erro na tela e no BLE | reiniciar → reinicialização |

Proposta: o limite de 20 s de AGUARDANDO_SOPRO funciona também como gatilho de ociosidade; um único temporizador cumpre os dois papéis, e qualquer sopro ou comando o reinicia.

## Eventos

```c
struct EventoBafometro {
    uint8_t  tipo;     /* EVENTO_TEMPO, EVENTO_SOPRO, EVENTO_COMANDO, EVENTO_FALHA */
    uint8_t  codigo;   /* TEMPO: finalidade; COMANDO: iniciar|repetir|cancelar|calibrar|reiniciar; FALHA: causa */
    uint16_t geracao;  /* descarta temporizações obsoletas */
    uint32_t valor;    /* auxiliar, opcional */
};
```

- A cada vez que o temporizador é armado, `geracaoAtual` é incrementada e o valor segue no evento; a máquina descarta `EVENTO_TEMPO` de geração diferente da atual.
- Callbacks de temporizador e de GPIO apenas montam o evento e o inserem com `K_NO_WAIT`.
- Inserção que falha incrementa um contador de perdas e gera `LOG_ERR`; perda de tique de amostragem invalida a medição em curso.
- Laço principal: `k_msgq_get(..., K_FOREVER)`, validação da geração e `smf_run_state()`.

## Uso do kernel

| Recurso | Qtde | Uso |
|---|---:|---|
| Thread de aplicação | 1 | A própria `main` |
| `k_msgq` | 1 | Todos os eventos (8 posições de 8 bytes) |
| `k_timer` | 1 | Um disparo (aquecimento, espera, apresentação) e periódico (tique de 50 ms na coleta) |
| Callback de GPIO | 1 | Detector → `EVENTO_SOPRO`, armado só em AGUARDANDO_SOPRO e ECONOMIA |
| `k_mutex` | 1 | Protege o último pacote BLE, porque as características serão de leitura e notificação |
| Semáforos, filas de trabalho e threads extras | 0 | A fila cobre a sincronização; BLE e LOG usam as threads do sistema |

Watchdog de hardware: opcional, coerente com o *fail-active*.

## Aquisição

- Leitura síncrona por tique (`adc_read_dt`), conversão para mV, validação elétrica (guardas do divisor herdadas do protótipo) e entrada no filtro.
- Sobreamostragem opcional (proposta): 4 leituras consecutivas mediadas por laço. No ESP32-C3 o driver não aceita `extra_samplings` fora do modo DMA.
- A precisão do ADC é item de validação nos dois alvos: offset, ganho e repetibilidade contra fonte e multímetro de referência. No STM32WB55, o canal interno `vref` (alias `volt-sensor0` na placa WeAct) mede a referência real.

## Display

Controlador pendente (SH1107 ou SSD1327). Situação no Zephyr 4.4.0:

- **SSD1327:** binding `solomon,ssd1327` no I²C, com as propriedades obrigatórias `oscillator-freq`, `display-offset`, `start-line`, `multiplex-ratio`, `remap-value`, `phase-length` e `prechargep`. O driver aceita apenas `PIXEL_FORMAT_L_8`, exige origem x par e pitch igual à largura. O CFB trabalha com 1 bit por pixel e não serve a esse driver. Caminho: renderizador próprio mínimo, com fonte fixa 8×8 e buffer por linha de texto (128×16 px em L_8 = 2 KiB), escrito com `display_write`.
- **SH1107:** sem binding. Opções: testar `sinowealth,sh1106` (driver `display_ssd1306.c`, monocromático) com 128×128, sem suporte oficial, ou escrever um driver mínimo sobre I²C. O CFB seria compatível com o formato monocromático, mas aloca o buffer com `k_malloc`, contrariando a alocação estática.

Nos dois casos, `tela.c` expõe uma interface pequena (iniciar, mensagem, contagem regressiva e resultado), e o LVGL fica fora do escopo.

## Inibidor

- Saída mantida, **ativa** em INICIALIZANDO e em ERRO.
- Muda apenas por medição válida: ativa se mg/L ≥ `LIMITE_INIBICAO_MGL`; libera somente se mg/L < `LIMITE_INIBICAO_MGL − HISTERESE_LIBERACAO_MGL`.
- Qualidade insuficiente ou medição inválida não liberam; ECONOMIA preserva o último estado.
- Módulo `inibidor.c` com três funções: `inibidor_iniciar` (ativo), `inibidor_ativar` e `inibidor_liberar`.

Consequência: o aparelho liga inibindo e só libera após um teste válido abaixo do limite.

## Calibração e persistência do R0

- R0 persistido com o subsistema `settings` (backend NVS), com versão do registro e faixa de validade na leitura.
- Ausente, corrompido ou fora de faixa: usa `R0_PADRAO_OHMS` e a flag `naoCalibrado`, visível na tela e no BLE.
- Calibração apenas pelo comando `calibrar` (proposta), com sensor quente e ar limpo; gravação só após sucesso e validação de faixa. Nunca a cada boot.
- As duas placas já definem a partição `storage`: 192 KB no layout padrão de 4 MB do ESP32-C3 e 8 KB na WeAct.
- Deriva do R0 com temperatura e umidade: fora do escopo do TCC, registrada como limitação.

## Modo economia

- Aquecedor em repouso, `display_blanking_on`, intervalo de anúncio BLE maior, temporizador parado e fila em espera.
- A interrupção do detector permanece armada como despertar.
- O despertar sempre passa por PRE_AQUECENDO, porque o sensor esfriou.
- Gerenciamento profundo de energia do SoC (`CONFIG_PM`) fica fora do mínimo funcional; a política térmica (`ECONOMIA_MANTEM_AQUECEDOR`) fica parametrizada.

## Comunicação BLE (proposta)

Pacote de resultado em little-endian, 12 bytes, dentro dos 20 bytes úteis do MTU padrão:

| Campo | Tipo |
|---|---|
| `versao` | `uint8_t` |
| `sequencia` | `uint16_t` |
| `estado` | `uint8_t` |
| `qualidade` | `uint8_t` |
| `flags` (validade, calibrado, inibindo) | `uint8_t` |
| `concentracao` (mg/L × 1000) | `uint16_t` |
| `instante` | `uint32_t` |

- Característica de comando: a escrita chama um callback que publica `EVENTO_COMANDO`, sem bloquear.
- UUIDs do serviço e das características: a definir.
- No STM32WB55, o BLE depende do binário HCI no coprocessador M0+; a API Bluetooth Host do Zephyr é a mesma nos dois alvos.

## Estruturas essenciais

- `ContextoBafometro`: `struct smf_ctx` como primeiro campo, `eventoAtual`, `geracaoAtual`, `ColetaMedicao`, `ResultadoMedicao`, `r0`, `r0Calibrado` e `codigoErro`.
- `ColetaMedicao`: vetor estático de 110 posições (5 s / 50 ms + margem), `qtdValidas`, `qtdDescartadas`, `inicioMs`, mínimo e máximo, contexto do filtro (soma corrente, índice, preenchidas) e contadores de tiques com e sem sopro.
- `ResultadoMedicao`: média em mV, desvio, Rs, R0, Rs/R0, ppm, mg/L, qualidade, flags de validade, calibração e inibição, e sequência.
- Histórico circular de 10 resultados: opcional, para diagnóstico via BLE.
- Tipos do Zephyr (`gpio_dt_spec`, `adc_dt_spec`, `bt_conn`) permanecem privados aos módulos de dispositivo.

## Filtro (proposta)

`dominio/filtro.h` declara o contrato e `filtro_media_movel.c` o implementa:

```c
void filtro_iniciar(ContextoFiltro *ctx);
void filtro_adicionar(ContextoFiltro *ctx, int32_t amostraMv);
int32_t filtro_valor(const ContextoFiltro *ctx);
```

Um filtro de Kalman futuro implementa as mesmas assinaturas em `filtro_kalman.c`, e a troca é feita no CMake, sem indireção em tempo de execução. Ponteiros de função (Strategy completo) só entram se for preciso trocar de filtro durante a execução.

## Padrões de projeto

Aplicados:

- máquina de estados (SMF plano, dirigido a eventos);
- adaptador de hardware (devicetree e módulos de dispositivo finos);
- tabela de consulta (curva Rs/R0 → ppm constante; os limites de saturação viram as extremidades da tabela);
- rotina de interrupção mínima (o callback só publica o evento);
- observer degenerado (a aplicação chama tela e BLE diretamente; `zbus` só se surgir um terceiro consumidor).

Recusados, com registro: Singleton formal, orientação a objetos em C, factory, thread por módulo, ADC assíncrono, LVGL e `zbus`.

## Parâmetros do produto (`config_produto.h`)

| Parâmetro | Valor inicial | Situação |
|---|---|---|
| `TEMPO_PRE_AQUECIMENTO_MS` | 10 000 | definido; origem a citar |
| `TEMPO_ESPERA_SOPRO_MS` | 20 000 | definido; origem a citar |
| `TEMPO_SOPRO_MINIMO_MS` | 5 000 | definido |
| `PERIODO_AMOSTRAGEM_MS` | 50 | proposta; validar |
| `TOLERANCIA_QUEDA_SOPRO_MS` | 150 | proposta; validar |
| `TEMPO_TRANSIENTE_MS` | 2 000 | proposta (herança dos 2 200 ms do protótipo) |
| `JANELA_MEDIA_MOVEL` | 8 | proposta; faixa de 8 a 16 |
| `TEMPO_APRESENTACAO_MS` | 10 000 | proposta |
| `TEMPO_MIN_ENTRE_TESTES_MS` | a levantar | pendente |
| `TEMPO_ACOMODACAO_FASE_MS` | a levantar | pendente |
| `QTD_AMOSTRAS_CALIBRACAO` / `INTERVALO_CALIBRACAO_MS` | 60 / 100 | protótipo |
| `R0_PADRAO_OHMS` | 24 000 | protótipo |
| `FAIXA_R0_MIN_OHMS` / `FAIXA_R0_MAX_OHMS` | a definir | pendente |
| `RESISTENCIA_CARGA_OHMS` / `TENSAO_ALIMENTACAO_MV` | 3 300 / 3 300 | circuito |
| `LIMITE_INIBICAO_MGL` / `HISTERESE_LIBERACAO_MGL` | método de banda de guarda | pendente |
| `QUALIDADE_CV_*` | 2 / 5 / 10 % | revalidar (calibrados com 5 amostras; agora são cerca de 100) |

A curva Rs/R0 → concentração fica como tabela em `conversao.c`, sem constante de conversão solta.

## Testes

1. Domínio puro com ztest no `native_sim`: filtro (janela parcial e cheia), estatística, Rs (incluindo guardas), interpolação e extremos da tabela, qualidade e decisão de inibição com histerese.
2. Máquina de estados com eventos injetados e tempos reduzidos: sequência normal, fim da espera levando à economia, sopro interrompido aos 3 s, evento de geração antiga, fila cheia, cancelamento, falha de ADC e comando de calibração.
3. Checklist de hardware por alvo: polaridades, fases Q5/Q7, ADC contra referência, display, BLE (pacote e comando), estado do inibidor no boot, em erro e em economia, e reinicialização.
