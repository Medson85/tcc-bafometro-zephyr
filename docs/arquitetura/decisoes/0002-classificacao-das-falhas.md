# Decisão 0002 — Classificação das falhas e estado do inibidor

- Estado: proposta
- Data: 14 de novembro de 2026
- Origem: revisão da etapa 3 ([relatório RT-01/2026](../../desenvolvimento/relatorios/RT-01-2026-retomada-etapa3.md))

## Contexto

Os requisitos tratam três situações diferentes:

- RF09: medição inválida ou de qualidade insuficiente **não libera**;
- RF10 e CA04: em qualquer falha e com medição inválida, o inibidor **fica ativo**;
- RF13: o R0 só vale depois da **validação de faixa**.

A revisão do commit `995c111` mostrou que o código não detectava parte das falhas, e por isso não cumpria o RF10:

- com o sensor desconectado, o nó do ADC fica preso a RL; a leitura de 4 mV passava pela guarda de 1 mV e era medida como ar limpo, liberando o inibidor;
- a calibração feita com álcool no ar era aceita (R0 = 4,3 kΩ); depois dela, um sopro de 200 ppm era lido como 50 ppm e liberava o inibidor;
- os retornos das escritas nos GPIOs do inibidor e do aquecedor eram ignorados;
- uma falha publicada com a fila cheia se perdia.

## Decisão

Toda situação do fluxo cai em uma de quatro classes:

| Classe | Situações | Inibidor | Requisito |
|---|---|---|---|
| Falha técnica | Falha ao configurar ou comandar o inibidor, o aquecedor ou o detector; falha de leitura do ADC; medição fora da faixa medida (tensão abaixo de `TENSAO_MINIMA_VALIDA_MV` ou junto de VCC) | ERRO: ativo e aquecedor em repouso; só o comando de reinício sai do estado | RF10, CA04 |
| Leitura rejeitada | Qualidade insuficiente (coeficiente de variação) | Mantém o estado: não libera nem reativa | RF09 |
| Ausência de medição | Sopro interrompido, evento perdido durante a coleta, espera esgotada, cancelamento, economia | Mantém o estado | RF04, RF14 |
| Medição aceita | Medição válida com qualidade diferente de descartar | Ativa se ≥ limite; libera se < limite − histerese; entre os dois, mantém | RF09 |

Regras de implementação:

1. As funções de dispositivo que comandam saídas retornam erro (`inibidor_ativar`, `inibidor_liberar`, `sensor_alcool_definirFase`); a máquina trata o erro como falha técnica. Em erro, o inibidor fica logicamente ativo e o aquecedor tenta ir para o repouso.
2. Em APRESENTANDO_RESULTADO, o inibidor é comandado antes de a tela mostrar o resultado.
3. `bafometro_publicarFalha` registra a falha antes de enfileirar o evento. A falha registrada substitui o próximo evento processado, mesmo que o seu próprio evento tenha se perdido por fila cheia. Em ERRO, a falha pendente é descartada.
4. Guarda do divisor: abaixo de `TENSAO_MINIMA_VALIDA_MV` a leitura é falha técnica, porque o circuito não distingue sensor e circuito aberto. Com o erro de ±35 mV do ADC do ESP32-C3 a 11 dB, o valor provisório é 100 mV, que corresponde a Rs ≈ 105 kΩ com RL = 3,3 kΩ.
5. O R0 calibrado só é aceito entre `FAIXA_R0_MIN_OHMS` e `FAIXA_R0_MAX_OHMS`, provisoriamente de 10 kΩ a 100 kΩ:
   - o mínimo rejeita a calibração com álcool no ar (com 50 ppm, Rs/R0 = 0,18 daria 4,3 kΩ);
   - o máximo coincide com o maior Rs que a guarda aceita.

## Consequências positivas

- O RF10 passa a valer também para falhas que antes não eram detectadas.
- O comportamento de cada situação fica explícito e testado: testes de falhas com dublês dos dispositivos (`tests/falhas`), testes da máquina com ADC e GPIO emulados e testes de domínio para a guarda e a faixa do R0.
- A falha deixa de depender da entrega pela fila.

## Restrições e riscos

- Os valores de 100 mV e de 10 kΩ a 100 kΩ são provisórios. O datasheet do MQ303B indica, no circuito de teste dele, saída de 0,01 V a 0,45 V em ar limpo; supondo RL = 4,7 kΩ, isso dá R0 de ~27 kΩ a ~1,4 MΩ (inferência, não faixa garantida). Um sensor com R0 acima de ~100 kΩ exige RL maior para ser medido; com RL = 3,3 kΩ, ele vai para ERRO.
- A guarda de 100 mV, a faixa do R0 e a fórmula de Rs valem para RL do lado do terra, que é premissa ([definições de hardware](../../hardware.md)). Com RL do lado de Vc, o sensor desconectado leva o nó a Vc, e a guarda precisa ir para o lado alto: uma troca de topologia revê os três juntos.
- Sem R0 calibrado, o firmware mede com `R0_PADRAO_OHMS` e pode liberar, como o CA06 permite. Se o R0 real for o dobro do padrão, 100 ppm (0,200 mg/L, acima do limite) são lidos como ~54 ppm (0,109 mg/L) e liberam. A política de liberação sem calibração é decisão pendente.
- Uma leitura rejeitada mantém a liberação já concedida, conforme o RF09 e a regra da arquitetura de que o inibidor só muda por medição aceita. Se o projeto preferir que um novo sopro rejeitado volte a inibir, o RF09 precisa mudar antes do código.
- O firmware não cobre reset, perda de alimentação nem travamento. Esses casos dependem do circuito da saída de inibição (ver [definições de hardware](../../hardware.md)).
- Falha de leitura do detector de sopro conta como ausência de sopro (pede repetição), não como falha técnica.

## Alternativas consideradas

- **Limite relativo de Rs/R0 para detectar o sensor ausente.** Recusado: depende de um R0 que pode estar errado, justamente o caso da calibração defeituosa. A guarda de tensão não depende do R0.
- **Tratar a leitura rejeitada como falha técnica.** Recusado por ora: contraria a redação do RF09, e um sopro instável bloquearia o veículo já liberado até o reinício.
- **Garantir a entrega da falha aumentando a fila.** Recusado: reduz a chance de perda, mas não a elimina.
