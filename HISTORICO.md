# Histórico de alterações

Este arquivo registra as mudanças relevantes realizadas no projeto.

## Ainda não publicado

### Adicionado

- criação do diretório oficial do projeto;
- descrição inicial do trabalho;
- documentação do ambiente de desenvolvimento;
- visão geral funcional;
- registro da versão de referência do Zephyr;
- regras iniciais de contribuição;
- configuração de arquivos ignorados pelo Git;
- padronização de codificação e quebras de linha;
- requisitos funcionais, não funcionais, restrições e critérios de aceitação;
- definições de hardware do ESP32-C3-MINI-1 e da placa do STM32WB55;
- arquitetura do software embarcado;
- plano de implementação;
- esqueleto da máquina de estados (etapa 1), com roteiro de simulação e testes no native_sim;
- lógica de cálculo portada do protótipo (etapa 2), com testes no native_sim;
- leitura do sensor, fases do aquecedor e saída de inibição pelo devicetree (etapa 3), com ADC e GPIO emulados nos testes;
- cálculo da medição em PROCESSANDO e do R0 em CALIBRANDO_R0, ainda sem persistência;
- testes de falhas com dublês dos dispositivos (`tests/falhas`);
- decisão 0002: classificação das falhas e estado do inibidor;
- faixa de validade do R0 e guarda mínima de tensão do divisor, provisórias;
- regra de estado seguro elétrico da saída de inibição e dados do datasheet do MQ303B nas definições de hardware;
- relatório RT-01/2026 de auditoria, correção e retomada da etapa 3 (versão 2.2, final).

### Corrigido

- fórmula de Rs do protótipo, que invertia a razão Rs/R0 no divisor com o ADC sobre RL;
- interpolação da curva na versão 1.1 do protótipo, invertida dentro de cada segmento;
- sensor desconectado ou frio era medido como ar limpo e liberava o inibidor;
- calibração feita com álcool no ar era aceita, e sopros com álcool passavam a ser lidos abaixo da curva;
- falhas de escrita nos GPIOs do inibidor e do aquecedor eram ignoradas;
- falha publicada com a fila de eventos cheia se perdia;
- a posição de RL no divisor era atribuída ao datasheet; passou a ser premissa do projeto a confirmar na placa.

### Alterado

- documentação reorganizada na pasta docs;
- visão geral atualizada com as propostas a validar e as questões abertas.

## Versionamento previsto

O projeto utilizará números no formato:

MAIOR.MENOR.CORREÇÃO

Durante o desenvolvimento inicial serão utilizadas versões abaixo de 1.0.0.
