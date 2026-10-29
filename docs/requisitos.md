# Requisitos

Requisitos funcionais, não funcionais, restrições e critérios de aceitação do bafômetro.

Situação de cada item:

- **definido**: decisão registrada do projeto;
- **proposta**: vem do escopo consolidado v2 e aguarda confirmação ou validação em bancada;
- **pendente**: depende de definição de hardware ou de ensaio.

## Escopo

Bafômetro com saída de inibição (interlock), executando a mesma aplicação no ESP32-C3-MINI-1 e no STM32WB55CGU6 sobre o Zephyr RTOS 4.4.0.

## Requisitos funcionais

| Código | Requisito | Situação |
|---|---|---|
| RF01 | Ao ligar, o sistema inicia com o inibidor ativo e verifica os dispositivos. | definido |
| RF02 | O sensor passa por pré-aquecimento de 10 s, com Q7 em nível baixo e Q5 em nível alto. O display mostra a contagem regressiva. | definido; origem dos 10 s a citar |
| RF03 | O sistema aguarda o sopro por no máximo 20 s. | definido; origem dos 20 s a citar |
| RF04 | O sopro só é válido com o detector ativo por 5 s contínuos. Interrupção antes dos 5 s invalida a medição e pede repetição. | definido; tolerância a microquedas de 150 ms proposta |
| RF05 | O sinal do sensor é coletado durante os 5 s do sopro, com Q7 em nível alto e Q5 em nível baixo. | definido |
| RF06 | As amostras passam por filtro de média móvel. O filtro de Kalman fica para trabalhos futuros. | definido |
| RF07 | O resultado é calculado em mg/L a partir da razão Rs/R0. | definido; método de conversão proposto |
| RF08 | O display mostra o resultado e é atualizado em cada estágio do teste. | definido |
| RF09 | O inibidor é ativado com resultado igual ou acima do limite e só é liberado abaixo do limite menos a histerese. Medição inválida ou de qualidade insuficiente não libera. | definido; valores do limite e da histerese pendentes |
| RF10 | Em qualquer falha, o inibidor permanece ativo. A saída é mantida, nunca pulsada. | definido |
| RF11 | Cada estágio do teste é registrado e acompanhado via BLE (estado e resultado). | definido |
| RF12 | O sistema aceita comandos via BLE: iniciar, repetir, cancelar, calibrar e reiniciar. | proposta |
| RF13 | O R0 é persistido entre ligações. A calibração ocorre apenas por comando explícito, com o sensor aquecido, e o valor só é gravado após validação de faixa. | persistência definida; política de calibração proposta |
| RF14 | Sem ação do usuário, o sistema entra em modo economia (aquecedor em repouso, display apagado, anúncio BLE reduzido). Sopro ou comando o despertam, passando de novo pelo pré-aquecimento. | definido; gatilho no fim da espera de 20 s proposto |
| RF15 | Entre dois testes, o sistema impõe um tempo mínimo para recuperação do sensor. | definido; valor a levantar |

## Requisitos não funcionais

| Código | Requisito | Situação |
|---|---|---|
| RNF01 | A mesma aplicação executa nos dois alvos; as diferenças ficam restritas a devicetree (um overlay por placa) e configuração. | definido |
| RNF02 | Versão de referência: Zephyr 4.4.0 e Zephyr SDK 1.0.1 ([ADR 0001](arquitetura/decisoes/0001-versao-do-zephyr.md)). | definido |
| RNF03 | Alocação estática de memória. | definido |
| RNF04 | Uma thread de aplicação, máquina de estados plana (SMF), fila de eventos tipada (`k_msgq`) e um único temporizador (`k_timer`) reutilizado. | definido |
| RNF05 | Sem camada de abstração de hardware própria: o hardware é descrito por devicetree. | definido |
| RNF06 | Nomes em português, camelCase para variáveis e funções, PascalCase para tipos e macros em maiúsculas com a unidade no nome. | definido |
| RNF07 | Regras de domínio sem dependência do Zephyr, testáveis por testes automatizados. | definido |
| RNF08 | Decisões relevantes registradas em [decisões de arquitetura](arquitetura/decisoes/). | definido |

## Restrições

- ESP32-C3: o ADC2 é inutilizável (errata); a aquisição usa apenas o ADC1.
- ESP32-C3: o driver ADC do Zephyr aceita apenas referência interna, 12 bits e tempo de aquisição padrão. Fora do modo DMA, não aceita amostragens extras nem leitura de vários canais na mesma sequência.
- ESP32-C3: os 15 GPIOs do módulo estão comprometidos; a saída de inibição ainda não tem pino ([definições de hardware](hardware.md)).
- STM32WB55: o BLE exige binário HCI no coprocessador M0+, gravado com sonda SWD externa.
- O `native_sim` só executa em Linux; no Windows exige WSL 2 ou máquina virtual.
- O pacote BLE de resultado deve caber nos 20 bytes úteis do MTU padrão (proposta).

## Critérios de aceitação

- CA01: a sequência completa (inicialização, pré-aquecimento, espera, coleta, processamento, apresentação e finalização) executa nos dois alvos com o mesmo código de aplicação;
- CA02: os tempos de pré-aquecimento, espera e coleta são cumpridos dentro da resolução do temporizador e registrados em log;
- CA03: um sopro interrompido antes de 5 s invalida a medição e solicita repetição;
- CA04: o inibidor está ativo na partida, em erro, com medição inválida e com resultado igual ou acima do limite, e só é liberado abaixo do limite menos a histerese;
- CA05: a ausência de sopro leva ao modo economia, e o despertar passa pelo pré-aquecimento;
- CA06: o R0 sobrevive à reinicialização; valor ausente ou fora de faixa usa o padrão e sinaliza "não calibrado" no display e no BLE;
- CA07: o display é atualizado em cada estágio, incluindo a contagem regressiva do pré-aquecimento;
- CA08: estado e resultado são publicados via BLE em cada estágio;
- CA09: os testes de domínio passam;
- CA10: critérios metrológicos ainda em aberto.

## Questões em aberto

- critérios metrológicos de aceitação;
- limite funcional de inibição e histerese, pelo método de banda de guarda proposto;
- origem dos tempos de 10 s e 20 s, a citar (datasheet do MQ303B ou medição);
- tempo mínimo entre testes;
- tempo de acomodação após a troca de fase de Q5 e Q7.
