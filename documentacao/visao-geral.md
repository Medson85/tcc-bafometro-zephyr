# Visão geral do projeto

## Objetivo

Desenvolver um programa embarcado para um bafômetro utilizando o Zephyr RTOS, com capacidade de execução em microcontroladores de arquiteturas diferentes.

O projeto prioriza:

- portabilidade entre plataformas;
- separação entre regras da aplicação e detalhes do equipamento;
- possibilidade de testes automatizados;
- rastreabilidade das decisões;
- comportamento seguro em condições de falha;
- documentação contínua.

## Plataformas previstas

- ESP32-C3-MINI-1, com arquitetura RISC-V;
- STM32WB55CGU6, com arquitetura Arm Cortex-M4.

## Elementos previstos

- sensor MQ303B;
- detector digital de sopro;
- circuito de aquecimento do sensor;
- circuito de condicionamento do sinal;
- conversor analógico-digital;
- mostrador OLED;
- comunicação Bluetooth de baixa energia;
- saída digital para o periférico de inibição.

## Sequência funcional prevista

1. inicialização;
2. verificação dos componentes;
3. pré-aquecimento do sensor;
4. espera pelo início do sopro;
5. validação da duração do sopro;
6. aquisição das amostras;
7. filtragem dos dados;
8. cálculo do resultado;
9. apresentação do resultado;
10. atuação do periférico de inibição;
11. finalização ou repetição do ciclo.

## Definições iniciais

- O pré-aquecimento terá duração inicial de 10 segundos.
- A espera pelo sopro terá limite inicial de 20 segundos.
- Um sopro válido deverá durar pelo menos 5 segundos.
- A aquisição será realizada durante 5 segundos.
- Durante o aquecimento, Q7 ficará em nível baixo e Q5 em nível alto.
- Durante a operação, Q7 ficará em nível alto e Q5 em nível baixo.
- Em uma falha crítica, o inibidor deverá permanecer ativo.
- A saída de inibição deverá ser mantida.

## Questões ainda abertas

- taxa de amostragem;
- tamanho da janela da média móvel;
- método de conversão entre ppm e mg/L;
- limite funcional de inibição;
- procedimento de calibração;
- critérios metrológicos de aceitação;
- detalhamento da arquitetura do programa embarcado.