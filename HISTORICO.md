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
- lógica de cálculo portada do protótipo (etapa 2), com testes no native_sim.

### Corrigido

- fórmula de Rs do protótipo, que invertia a razão Rs/R0 no divisor com o ADC sobre RL;
- interpolação da curva na versão 1.1 do protótipo, invertida dentro de cada segmento.

### Alterado

- documentação reorganizada na pasta docs;
- visão geral atualizada com as propostas a validar e as questões abertas.

## Versionamento previsto

O projeto utilizará números no formato:

MAIOR.MENOR.CORREÇÃO

Durante o desenvolvimento inicial serão utilizadas versões abaixo de 1.0.0.
