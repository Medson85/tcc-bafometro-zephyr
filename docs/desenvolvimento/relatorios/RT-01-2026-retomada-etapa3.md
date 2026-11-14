# Relatório técnico RT-01/2026 — Auditoria, correção e retomada da etapa 3

| | |
|---|---|
| **Projeto** | TCC — Bafômetro embarcado com Zephyr RTOS (ESP32-C3-MINI-1 ⇄ STM32WB55CGU6) |
| **Repositório** | [Medson85/tcc-bafometro-zephyr](https://github.com/Medson85/tcc-bafometro-zephyr) |
| **Documento** | RT-01/2026 — versão 2.2 (final) |
| **Data** | 14 de novembro de 2026 |
| **Substitui** | Versões 1.0, 1.1, 2.0 e 2.1, todas de 14/11/2026 |
| **Revisões auditadas** | `main` em `b31228c`; `funcionalidade/leitura-do-sensor` em `995c111` |
| **Revisão proposta** | Patch `etapa3-correcoes-rt01.patch` sobre `995c111`, não integrado; nada foi enviado ao GitHub |
| **Identificação do conteúdo** | Árvores Git de `app/` (`46aa9c9`) e `tests/` (`ff5f136`) depois do `git am`, as mesmas dos testes desta versão; não dependem do autor nem da data do commit |
| **Natureza** | Confronto das revisões anteriores; revisão do código contra os requisitos; correções; plano de retomada |

## Resumo executivo

O estado do projeto descrito desde a versão 1.0 se confirma: as etapas 1 e 2 estão integradas à `main`, e a etapa 3 está só na branch `funcionalidade/leitura-do-sensor`, sem PR e sem validação física.

A revisão do código encontrou dois caminhos de liberação indevida do inibidor: o sensor desconectado era lido como ar limpo, e a calibração feita com álcool no ar era aceita. Os dois foram confirmados por testes que reprovam no `995c111` e foram corrigidos no patch, junto com os pontos R2, R4, R5 e R7 da versão 1.0. Nesta versão 2.2, o patch entregue foi aplicado com `git am` num clone limpo e todos os 77 testes passaram de novo; as duas placas compilam sem avisos.

A revisão 2.1 acertou em três pontos, incorporados aqui:
- a documentação atribuía ao datasheet a posição de RL;
- o consumo de 140 mW não fecha com a resistência do aquecedor a frio;
- o R0 padrão pode autorizar uma liberação.

Ela também tinha erros, corrigidos aqui: o bloco de comandos para na primeira linha quando a árvore está limpa, e a descrição dos testes de aplicação estava imprecisa.

Esta versão acrescenta um dado físico que muda a prioridade da bancada. O MQ303B tem 3 pinos, e o elemento sensor usa um eletrodo do aquecedor como retorno. Com o aquecedor ligado ao terra, o natural é RL ficar do lado de Vc, e nesse caso a fórmula de Rs adotada na etapa 2 é a errada e a do protótipo é a certa. Uma medição de continuidade com a placa desligada resolve a dúvida e deve ser o primeiro ensaio.

A etapa 3 continua aberta. Falta uma decisão do autor: a política de liberação sem R0 calibrado.

## 1. Método e grau de evidência

**Fontes:**
- versões 1.0, 2.0 e 2.1 deste relatório;
- histórico de commits e PRs no GitHub;
- código e documentação de `b31228c`, `995c111` e do patch;
- texto do manual do MQ303B v1.4.

**Executado nesta versão (Linux x86-64, Zephyr v4.4.0, Zephyr SDK 1.0.1):**
- `git am` do patch entregue num clone limpo da branch e `git diff --check`;
- testes de domínio, máquina e falhas em `native_sim/native/64` e a simulação;
- compilação de `esp32c3_devkitm/esp32c3` e `weact_stm32wb55_core/stm32wb55xx`;
- o bloco de comandos da seção 12, testado no PowerShell 7.4.6 em quatro cenários.

**Não verificado:**
- a Figura 2 do manual. O download direto do PDF é bloqueado neste ambiente, e o texto extraído não contém o desenho;
- as partes do bloco de comandos exclusivas do Windows: a pasta Downloads, a ativação do ambiente Python e o `west build` real.

| Grau | Significado |
|---|---|
| **Confirmado no GitHub** | Observado no repositório remoto |
| **Reproduzido** | Executado nesta versão, com log |
| **Corrigido** | Defeito demonstrado por teste que reprova antes da correção e passa depois |
| **Não verificado** | Afirmação de outra revisão que esta não pôde conferir |
| **Pendente de bancada** | Depende de circuito, instrumentos ou placa |

**Limites:** sem acesso ao diretório local do autor, ao circuito montado nem a instrumentos.

## 2. Histórico das revisões

| Versão | Contribuição |
|---|---|
| 1.0 | Estado do repositório e riscos R1 a R8 |
| 2.0 | Reprodução dos testes; dois caminhos de liberação indevida; patch com as correções e 77 testes |
| 2.1 | Ressalvas sobre a topologia, a potência do aquecedor e o R0 padrão; comandos mais cautelosos |
| 2.2 | Confronto da 2.1; reprodução sobre o patch aplicado; estrutura de 3 pinos; teste de continuidade; risco do R0 padrão quantificado; comandos testados |

## 3. Confronto com a revisão 2.1

| Afirmação da 2.1 | Verificação nesta versão | Situação |
|---|---|---|
| Testes não reexecutados; só registros anteriores | Reexecutados sobre o patch aplicado com `git am`: 42 + 23 + 12 = 77, todos aprovados | Superada |
| Patch com 33 arquivos (25 modificados e 8 novos), feito sobre `995c111` | Confere; `git diff --check` sem problemas | Confirmada |
| Teste de aplicação "no Windows", com registros "fornecidos pelo autor" | O teste foi em Linux com `core.autocrlf=true`, simulando o Git do Windows, e os registros são desta auditoria | Corrigida |
| A Figura 2 desenha RL do lado de Vc | O texto do manual liga a saída à tensão sobre a resistência de carga, mas não diz de que lado ela fica; a figura não pôde ser inspecionada aqui | Não verificada; adotada como cautela |
| A documentação do patch atribuía ao manual a posição de RL no terra | Correto: comentário de `config_produto.h`, arquitetura e hardware | Confirmada e corrigida |
| Não alterar a fórmula antes de inspecionar a placa | De acordo; esta versão acrescenta o teste de continuidade (seção 6) | Confirmada |
| 140 mW não fecham com 0,9 V sobre 4 Ω (~203 mW) | Correto; 140 mW a 0,9 V implicam ~5,8 Ω, aquecedor quente | Confirmada e documentada |
| R0 de ~27 kΩ a ~1,4 MΩ é inferência | Correto: a conta supõe RL = 4,7 kΩ | Confirmada; texto corrigido |
| O R0 padrão pode autorizar liberação | Correto e quantificado na seção 7 | Confirmada; decisão pendente |
| As guardas não cobrem todas as falhas | Correto; além disso, a guarda depende da topologia (seção 6) | Confirmada e ampliada |
| RF10 parcialmente atendido no software | Mais preciso que "atendido" | Aceita |
| `AGENTS.md` opcional | De acordo | Aceita |
| Bloco de comandos com interrupção segura | Com a árvore limpa, o caso normal, `(git status --porcelain).Trim()` chama um método sobre `$null` e o bloco para na primeira verificação (testado no PowerShell 7.4.6). As demais verificações são boas | Corrigida na seção 12 |
| Não apagar a branch local automaticamente | Correto; a 2.0 sugeria apagá-la | Aceita |
| Sem push nesta etapa | Aceito como padrão; o envio fica num bloco separado | Aceita |

## 4. Confronto com a versão 1.0

| Afirmação da 1.0 | Situação |
|---|---|
| Estado da `main`, da branch, dos PRs, dos arquivos (70) e dos testes (50 e 55); sem CI | Confirmada |
| Integração "em 29/10/2026" | Corrigida: PR #1 em 28/10 às 23:52 (UTC−3), PR #2 em 29/10 às 00:33, PR #3 em 29/10 às 08:12 |
| Testes da etapa 3 relatados e não reproduzidos | Superada: reproduzidos na 2.0 e de novo na 2.2 |
| R1 estado seguro do inibidor depende do circuito | Confirmada e complementada (seção 8) |
| R2 retornos de GPIO ignorados | Confirmada e corrigida (C1) |
| R3 o overlay "assume" nível alto | Corrigida em parte: os níveis por fase são requisito (RF02 e RF05); o risco restante é de circuito |
| R4 falha perdida com a fila cheia | Confirmada em parte (hoje só os testes publicam falha pela fila) e corrigida preventivamente (C2) |
| R5 leitura rejeitada mantém a liberação | Confirmada, formalizada (decisão 0002) e coberta por teste (C5) |
| R6 topologia, faixa do ADC e calibração | Confirmada e agravada (seção 6) |
| R7 documentação desatualizada | Confirmada e corrigida |
| R8 display, BLE e porte | Confirmada |

## 5. Achados

| Código | Achado | Tratamento |
|---|---|---|
| N1 | Sensor desconectado lido como ar limpo: 4 mV → Rs de 2,7 MΩ → 50 ppm → inibidor **liberado** (RF10) | Corrigido (C3) |
| N2 | Calibração com álcool no ar aceita: R0 = 4,3 kΩ; depois, 200 ppm lidos como 50 ppm → **liberado** (RF13) | Corrigido (C4) |
| N3 | Topologia do divisor: premissa, não dado do datasheet. Com a fórmula do lado errado, o firmware inibe em ar limpo e libera com álcool | Seção 6; primeiro ensaio de bancada |
| N4 | Requisitos desatualizados (inibidor "sem pino") | Corrigido |
| N5 | Datasheet: 3 pinos; saída em ar limpo de 0,01 V a 0,45 V e em 125 ppm de 1,0 V a 2,5 V; aquecedor de 4,0 Ω ± 0,5 Ω; até 140 mW; pré-aquecimento inicial de 48 h | Documentado; o R0 derivado é inferência |
| N6 | Com RL = 3,3 kΩ e a guarda de 100 mV, o firmware só mede Rs até ~105 kΩ | Documentado |
| N7 | Potência do aquecedor: 140 mW a 0,9 V implicam ~5,8 Ω a quente; a frio, ~0,2 W | Documentado; medir |
| N8 | Sem R0 calibrado, o firmware mede com o R0 padrão e pode liberar com álcool acima do limite | Seção 7; decisão pendente |

## 6. Topologia do divisor

O firmware supõe RL do lado do terra. A escolha da etapa 2 se apoiou no comentário de `sensor.h` da V1.1 do protótipo. O datasheet e o texto do TCC usam VRL, a tensão sobre RL, como saída, mas não mostram de que lado RL fica nem onde o ADC está ligado.

| | RL no terra (premissa do firmware) | RL em Vc (desenho da Figura 2, segundo a 2.1) |
|---|---|---|
| Circuito | Vc – Rs – nó – RL – GND | Vc – RL – nó – Rs – GND |
| Tensão no nó do ADC | `Vc × RL / (Rs + RL)` | `Vc × Rs / (Rs + RL)` |
| Fórmula de Rs | `RL × (Vc − Vadc) / Vadc` (firmware atual) | `RL × Vadc / (Vc − Vadc)` (protótipo) |
| Com mais álcool | Sobe | Cai |
| Ar limpo, R0 = 24 kΩ, RL = 3,3 kΩ, Vc = 3,3 V | 0,40 V | 2,90 V, acima da faixa de 2,5 V do ESP32-C3 a 11 dB |
| Sensor desconectado | Nó vai a 0 V (guarda baixa, já implementada) | Nó vai a Vc (exigiria guarda alta) |

**Estrutura de 3 pinos.** Pelo manual, os pinos 1 e 3 são o aquecedor, e a tensão de teste vai entre o pino 2 e o pino 1 ou 3. O elemento sensor usa, portanto, o eletrodo do aquecedor como retorno. Com o aquecedor alimentado pelo lado alto (Q5 e Q7 escolhendo 2,0 V ou 0,9 V) e retorno no terra, o sensor fica do lado do terra e RL do lado de Vc. Pôr RL no terra exigiria o aquecedor flutuando perto de Vc.

O R0 padrão de 24 kΩ do protótipo não decide a questão: é coerente com qualquer das duas topologias, conforme a fórmula que o produziu. Conclusão: a correção da fórmula feita na etapa 2 pode estar errada, e a fórmula do protótipo pode ser a certa.

**Ensaio, antes de qualquer sopro com álcool:**
1. Com a placa desligada, localizar RL e testar continuidade entre o terminal de RL que não vai ao pino 2 e o GND (RL no terra) ou o 3V3 (RL em Vc). Ou conferir no esquemático.
2. Com o sensor aquecido e em ar limpo, medir o nó do ADC com multímetro:
   - abaixo de ~1 V indica RL no terra;
   - acima de ~2,4 V indica RL em Vc.

   Os limites valem para R0 entre 10 kΩ e 100 kΩ.
3. Com vapor de álcool, a tensão do nó deve se mover no sentido da tabela.
4. Medir o nó em ar limpo nas duas fases do aquecedor. O retorno é compartilhado, então a tensão do aquecedor e a corrente na trilha de terra (até ~0,6 A) podem deslocar a leitura.

**Se RL estiver em Vc,** antes de qualquer outro ensaio:
- a fórmula passa a ser a do protótipo;
- a guarda de tensão vai para o lado alto;
- a faixa do R0 é recalculada;
- RL é revisto, porque com 3,3 kΩ o ar limpo fica acima da faixa do ADC;
- os testes acompanham as mudanças.

**Mesmo que a medição confirme RL no terra,** com R0 perto de 24 kΩ o ar limpo fica em ~0,4 V e a leitura satura acima de ~220 ppm. A escolha de RL deve ser revista na etapa 9 com os valores medidos.

## 7. R0 padrão e liberação sem calibração

Hoje não existe caminho de calibração no hardware: o comando de calibrar só chega pela fila, e o BLE é a etapa 5. A persistência é a etapa 6. Por isso, toda operação na placa usa `R0_PADRAO_OHMS` (24 kΩ), com o aviso "não calibrado", e pode liberar, como o CA06 permite.

**Exemplo calculado:** com o R0 real de 48 kΩ, um sopro de 100 ppm (0,200 mg/L, acima do limite de 0,150) tem Rs = 4,13 kΩ. Com o padrão, a razão sai 0,172 em vez de 0,086, o que dá ~54 ppm (0,109 mg/L), e o inibidor libera. Com o R0 real menor que o padrão, o erro vai para o lado de inibir.

A regra "sem R0 calibrado e validado, não libera" fecharia dois caminhos:
- o do R0 padrão;
- o da topologia errada, porque com a fórmula trocada a calibração em ar limpo dá ~455 Ω, fica fora da faixa de 10 kΩ a 100 kΩ e é rejeitada.

Em troca, a liberação na bancada passaria a exigir um caminho de calibração no hardware (BLE, ou um botão antes disso). A regra muda o RF09 e o CA06, então é **decisão do autor (D1)**.

## 8. Saída de inibição: estado seguro

Inibir deve ser o estado do atuador **sem energia**. Com o GPIO2, que fica alto pelo pull-up no reset, a solução é um estágio inversor: GPIO2 alto desliga o atuador (inibindo) e GPIO2 baixo, comandado pelo firmware, libera. O arranjo em que o nível alto energiza o atuador para inibir deve ser evitado, porque libera quando a energia cai. A validação vai primeiro com carga simulada, nunca no sistema de partida de um veículo.

## 9. Correções aplicadas

| Item | Mudança | Verificação |
|---|---|---|
| C1 (R2) | Falhas de escrita no inibidor e no aquecedor levam ao ERRO (`FALHA_INIBIDOR`, `FALHA_AQUECEDOR`); o inibidor é comandado antes da tela | 12 testes de falhas com dublês; mutações M1 a M4, M7 e M8 |
| C2 (R4) | Falha registrada antes da fila; chega ao ERRO mesmo com a fila cheia | Teste da máquina; M5 e M6 |
| C3 (N1) | Abaixo de `TENSAO_MINIMA_VALIDA_MV` (100 mV, provisório), falha técnica e ERRO | 4 testes de domínio e 1 da máquina; M9 e M12 |
| C4 (N2) | R0 calibrado aceito só entre 10 kΩ e 100 kΩ (provisório) | 3 testes de domínio e 1 da máquina; M10 e M11 |
| C5 (R5) | Leitura instável testada nos dois sentidos do inibidor | Testes da máquina |
| C6 | Decisão 0002 (classes de falha); documentação de arquitetura, hardware, requisitos, plano, README e histórico | 0 links relativos quebrados |
| C7 (2.2) | Posição de RL como premissa (comentário de `config_produto.h`, arquitetura, hardware, decisão 0002); 3 pinos e retorno compartilhado; teste de continuidade; potência a frio e a quente; R0 como inferência; risco do R0 padrão e topologia como questões abertas nos requisitos | Só comentários e documentação: os binários não mudam |

As 12 mutações (cada correção revertida isoladamente; algum teste reprovou em todas) foram feitas na versão 2.0. Desde então, o código C só mudou num comentário.

## 10. Evidências

| Conjunto | `995c111` (versão 2.0) | Patch aplicado com `git am` (versão 2.2) |
|---|---|---|
| Testes de domínio | 36/36 | 42/42 |
| Testes da máquina | 19/19 | 23/23 |
| Testes de falhas | — | 12/12 |
| Testes novos contra `995c111` (fase vermelha) | 3 de 4 reprovaram | — |
| Simulação | 6 roteiros | 7 roteiros; o 7º (sensor desconectado) vai ao ERRO com o inibidor ativo |
| ESP32-C3 | FLASH 139 296 B; DRAM 58 528 B | FLASH 139 376 B; DRAM 58 544 B |
| WeAct | FLASH 40 876 B; RAM 9 024 B | FLASH 41 040 B; RAM 9 024 B |
| Avisos de compilação | 0 | 0 |
| `git diff --check` | — | sem problemas |
| Bloco de comandos (PowerShell 7.4.6) | — | 4 cenários: aplica, confere as árvores e "compila" (west simulado); para com alteração local; para fora da base sem apagar o commit local; para sem o patch |

Os valores 199 ppm e 24 062 Ω da simulação (em vez de 200 ppm e 24 000 Ω) vêm da quantização do ADC emulado de 12 bits e do truncamento para mV inteiro.

## 11. Coerência entre requisitos e implementação

| Requisito | Situação | Observação |
|---|---|---|
| RF01 | Atendido no fluxo lógico | Falha na inicialização leva ao ERRO; o pino no reset depende do circuito |
| RF02, RF05 | Atendido no firmware | Níveis e duração testados; tensões do aquecedor a medir |
| RF03, RF04, RF06, RF15 | Atendido | Valores propostos ou provisórios |
| RF07 | Atendido no cálculo, **sob premissa** | Depende da topologia (seção 6) |
| RF08, RF11 | Pendente | Etapas 4 e 5 |
| RF09 | Atendido | Testado nos dois sentidos; valores provisórios |
| RF10 | Parcialmente atendido no software | Falhas detectadas levam ao ERRO; reset, falta de energia e falhas não detectáveis dependem do circuito |
| RF12, RF14 | Parcial | Transporte BLE, repouso de Q5/Q7, display e BLE pendentes |
| RF13 | Parcial | Faixa provisória implementada; persistência na etapa 6; política sem calibração em D1 |
| RNF01 | Parcial | Compila nos dois alvos; execução na WeAct pendente |
| RNF02 a RNF07 | Atendidos | |
| RNF08 | Parcial | ADR 0001 e 0002; outras decisões vinculantes sem ADR |
| CA01 | Parcial | Placa pendente |
| CA02 a CA05 | Atendidos na simulação | |
| CA06 a CA08 | Pendentes | Etapas 4 a 6 |
| CA09 | Atendido | |
| CA10 | Em aberto | Etapa 9 |

## 12. Próximo passo operacional

Bloco 1, para aplicar e compilar sem enviar nada. Cole o bloco inteiro num terminal PowerShell (não no ISE). Ele roda como uma unidade e para no primeiro erro, sem alterar nada:

```powershell
& {
    $ErrorActionPreference = 'Stop'
    $repo = 'C:\Dev\zephyr-workspace\tcc-bafometro-zephyr'
    $ativar = 'C:\Dev\zephyr-workspace\.venv\Scripts\Activate.ps1'
    $base = '995c1115ce9a4afdda9861697d23e4ff82979d7b'
    $arvoreApp = '46aa9c944ca9f1adde8f8fe42c294ee8ce45d269'
    $arvoreTestes = 'ff5f136072b736aea4cb4d5009fb835d58182410'

    Set-Location $repo
    & $ativar

    if (git status --porcelain) { throw 'Há alterações locais no repositório: revise antes de continuar.' }
    git fetch origin
    if ($LASTEXITCODE -ne 0) { throw 'Falha no git fetch.' }
    git switch funcionalidade/leitura-do-sensor
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao selecionar a branch.' }
    git merge --ff-only origin/funcionalidade/leitura-do-sensor
    if ($LASTEXITCODE -ne 0) { throw 'A branch local diverge da remota: revise antes de continuar.' }
    if ((git rev-parse HEAD) -ne $base) { throw 'A branch não está em 995c111: não aplique o patch.' }

    $downloads = (New-Object -ComObject Shell.Application).NameSpace('shell:Downloads').Self.Path
    $patch = Get-ChildItem -LiteralPath $downloads -Filter 'etapa3-correcoes-rt01*.patch' |
        Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if (-not $patch) { throw "Patch não encontrado em $downloads." }

    git am --3way $patch.FullName
    if ($LASTEXITCODE -ne 0) { git am --abort; throw 'O patch não aplicou; nada foi alterado.' }
    if (((git rev-parse 'HEAD:app') -ne $arvoreApp) -or ((git rev-parse 'HEAD:tests') -ne $arvoreTestes)) {
        throw 'O código aplicado difere do testado.'
    }

    west build -p always -b esp32c3_devkitm/esp32c3 app -d build/esp32c3
    if ($LASTEXITCODE -ne 0) { throw 'Falha na compilação do ESP32-C3.' }
    west build -p always -b weact_stm32wb55_core/stm32wb55xx app -d build/stm32wb55
    if ($LASTEXITCODE -ne 0) { throw 'Falha na compilação do STM32WB55.' }

    git log --oneline -2
    Write-Host 'Patch aplicado e compilado. Nada foi enviado ao GitHub.'
}
```

Tamanhos esperados: 139 376 B de flash no ESP32-C3; 41 040 B de flash e 9 024 B de RAM na WeAct. Se o bloco parar em "A branch não está em 995c111", inspecione `git log --oneline -3` antes de decidir; o bloco não apaga nada.

Bloco 2, opcional, para enviar a branch e abrir o PR como rascunho:

```powershell
git push origin funcionalidade/leitura-do-sensor
gh pr create --base main --head funcionalidade/leitura-do-sensor --fill --draft
```

Bloco 3, só depois dos ensaios da seção 13 e da revisão do PR:

```powershell
gh pr ready
gh pr merge --squash --delete-branch
git switch main
git pull --ff-only
```

## 13. Critérios para concluir a etapa 3

A etapa 3 se conclui com o primeiro sopro real medido, depois destes ensaios, nesta ordem:

1. topologia do divisor (seção 6) e, se RL estiver em Vc, a revisão do firmware antes de seguir;
2. tensões do aquecedor por fase e corrente no pré-aquecimento;
3. saída de inibição com carga simulada: inibindo no reset, sem alimentação e em ERRO;
4. sensor desconectado levando ao ERRO;
5. sopro real de 5 s com as amostras registradas (série temporal, resultado e estado da saída);
6. sopro interrompido, sopro instável e reinicialização na placa.

Evidências mínimas por ensaio:
- revisão do Git e hash da árvore;
- hardware efetivo;
- tensões medidas;
- log do firmware;
- ficha de ensaio;
- decisão registrada sobre os valores provisórios que mudarem.

## 14. Riscos remanescentes

| # | Risco | Prioridade |
|---|---|---|
| N3/R6 | Topologia do divisor e direção da medição (seção 6) | Crítica |
| R1 | Estado seguro da saída de inibição em reset, sem energia e com o MCU parado (seção 8) | Crítica |
| N8 | Liberação com o R0 padrão (seção 7) | Alta |
| R3 | Tensões do aquecedor, intertravamento físico de Q5/Q7 e repouso indefinido | Alta |
| P1 | Valores provisórios: 100 mV; faixa do R0; limite de 0,150 mg/L com histerese de 0,020 mg/L; limiares de CV; piso da curva em 50 ppm | Alta |
| P2 | Saturação do ADC acima de ~2,5 V lida como concentração alta, do lado de inibir | Média |
| P3 | O ramo de erro interno de `inibidor.c` foi revisado, mas não tem teste automatizado, porque o GPIO emulado não injeta falha | Média |
| P4 | Decisões vinculantes sem ADR (RNF08) | Média |
| P5 | Sem integração contínua; os testes do `native_sim` exigem Linux ou WSL 2 | Média |
| R8 | Display, BLE e porte | Média |

## 15. Plano de ação

| Fase | Ação | Situação |
|---|---|---|
| A | Reproduzir testes e compilações | Feito (Linux); compilar no Windows com o bloco 1 |
| B | Corrigir os defeitos de segurança | Feito (patch) |
| C | Topologia, estado seguro do inibidor e aquecedor | Pendente de bancada |
| D | Ensaios da seção 13 | Pendente de bancada |
| E | PR em rascunho; squash após D | Pendente |
| F a J | Etapas 4 a 9: display, BLE, persistência e economia, porte, metrologia e texto | Pendentes |

## 16. Parecer conclusivo

O projeto tem base consistente: documentação, máquina de estados e domínio estão integrados e testados, e a etapa 3 tem código completo, com os dois caminhos de liberação indevida encontrados nas revisões corrigidos e cobertos por teste. O que falta é de circuito e de bancada. O ponto mais importante é a topologia do divisor: a estrutura de 3 pinos do MQ303B sugere que a fórmula adotada na etapa 2 pode estar invertida, e um teste de continuidade com a placa desligada resolve a questão antes de qualquer sopro com álcool. Até a validação elétrica e metrológica, o equipamento é um protótipo acadêmico e não está apto a controlar a partida de um veículo.

## 17. Referências

- [Commit da etapa 2 (PR #3)](https://github.com/Medson85/tcc-bafometro-zephyr/commit/b31228cdb11b7f8bf1a1024612ab3ec0ac095fe6)
- [Commit da etapa 3 na branch](https://github.com/Medson85/tcc-bafometro-zephyr/commit/995c1115ce9a4afdda9861697d23e4ff82979d7b)
- [Requisitos](../../requisitos.md) · [Arquitetura](../../arquitetura/arquitetura-firmware.md) · [Definições de hardware](../../hardware.md) · [Plano de implementação](../plano-implementacao.md)
- [Decisão 0002 — Classificação das falhas e estado do inibidor](../../arquitetura/decisoes/0002-classificacao-das-falhas.md)
- [Winsen MQ303B — manual v1.4](https://www.winsen-sensor.com/d/files/manual/mq303b.pdf)

## Anexo A — Trechos de log

Fase vermelha (versão 2.0), testes novos contra `995c111`:

```text
test_sensor_desconectado_leva_ao_erro_e_ativa_o_inibidor
  Medição 2: 4 mV, Rs 2719200 ohms, Rs/R0 113300/1000, 50 ppm, CV 0/10 %, qualidade 3
  Inibidor LIBERADO
  FAIL
test_calibracao_com_alcool_presente_e_rejeitada
  R0 calibrado: 4326 ohms (60 amostras válidas)
  Medição 2: 2458 mV, Rs 1130 ohms, Rs/R0 261/1000, 50 ppm, CV 0/10 %, qualidade 3
  Inibidor LIBERADO
  FAIL
SUITE FAIL - 86.96% [maquina]: pass = 20, fail = 3, skip = 0, total = 23
```

Versão 2.2, patch aplicado com `git am`:

```text
clone limpo de 995c111 + git am · Zephyr v4.4.0 · SDK 1.0.1
árvores: app 46aa9c9 · tests ff5f136
SUITE PASS - 100.00% [calibracao]: pass = 8, fail = 0, skip = 0, total = 8
SUITE PASS - 100.00% [conversao]: pass = 10, fail = 0, skip = 0, total = 10
SUITE PASS - 100.00% [decisao]: pass = 4, fail = 0, skip = 0, total = 4
SUITE PASS - 100.00% [estatistica]: pass = 4, fail = 0, skip = 0, total = 4
SUITE PASS - 100.00% [filtro]: pass = 6, fail = 0, skip = 0, total = 6
SUITE PASS - 100.00% [medicao]: pass = 8, fail = 0, skip = 0, total = 8
SUITE PASS - 100.00% [qualidade]: pass = 2, fail = 0, skip = 0, total = 2
SUITE PASS - 100.00% [maquina]: pass = 23, fail = 0, skip = 0, total = 23
SUITE PASS - 100.00% [falhas]: pass = 12, fail = 0, skip = 0, total = 12
--- Roteiro 7: sensor desconectado: falha, ERRO e inibidor ativo ---
<err> bafometro: Medição inválida: tensão média 4 mV
<inf> inibidor: Inibidor ATIVO
esp32c3_devkitm/esp32c3        FLASH: 139376 B   dram0_0_seg: 58544 B
weact_stm32wb55_core/stm32wb55xx FLASH: 41040 B    RAM: 9024 B
```
