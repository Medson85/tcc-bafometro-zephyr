# TCC — Bafômetro Embarcado com Zephyr RTOS

Este repositório reúne a documentação e, posteriormente, o código-fonte do programa embarcado desenvolvido para o Trabalho de Conclusão de Curso.

O projeto consiste no desenvolvimento de um bafômetro embarcado portável entre duas plataformas de microcontroladores:

- ESP32-C3-MINI-1, baseado na arquitetura RISC-V;
- STM32WB55CGU6, baseado na arquitetura Arm Cortex-M4.

O programa será desenvolvido com o Zephyr RTOS, priorizando portabilidade, reutilização, capacidade de teste, rastreabilidade e separação entre a lógica da aplicação e os detalhes específicos do equipamento.

## Estado atual

O projeto encontra-se na fase de preparação do ambiente, documentação inicial e definição da arquitetura.

O desenvolvimento do programa embarcado ainda não foi iniciado.

## Ambiente de referência

- Zephyr RTOS 4.4.0;
- Zephyr SDK 1.0.1;
- Python 3.12.10;
- West 1.5.0;
- CMake 4.4.0;
- Windows de 64 bits.

A configuração completa será registrada no arquivo `documentacao/ambiente.md`.

## Plataformas previstas

### ESP32-C3

- placa de referência: ESP32-C3 DevKitM;
- arquitetura: RISC-V;
- alvo Zephyr: `esp32c3_devkitm/esp32c3`.

### STM32WB55

- placa de referência: WeAct STM32WB55 Core;
- arquitetura: Arm Cortex-M4;
- alvo Zephyr: `weact_stm32wb55_core/stm32wb55xx`.

## Estrutura inicial

- `documentacao/ambiente.md`;
- `documentacao/visao-geral.md`;
- `documentacao/decisoes/`;
- `.editorconfig`;
- `.gitattributes`;
- `.gitignore`;
- `HISTORICO.md`;
- `CONTRIBUICAO.md`;
- `README.md`.

## Referência anterior

Existe um projeto anterior utilizado somente para testes e consultas pontuais. Ele não pertence a este repositório e não representa a versão inicial do programa atual.

## Licenciamento

A licença do projeto ainda será definida. O repositório deverá permanecer privado até que sejam avaliadas as regras institucionais e a titularidade do material produzido.