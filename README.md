# TCC — Bafômetro Embarcado com Zephyr RTOS

Este repositório reúne a documentação para o Trabalho de Conclusão de Curso que consiste no desenvolvimento de um sistema embarcado para um bafômetro utilizando o Zephyr RTOS, com foco em portabilidade, organização do software e reutilização entre diferentes plataformas de microcontroladores.

## Objetivo

Desenvolver uma aplicação embarcada capaz de realizar o processo de medição de álcool utilizando o sensor **MQ303B**, executando a mesma lógica de aplicação em plataformas baseadas em arquiteturas distintas.

As plataformas previstas são:

* **ESP32-C3-MINI-1** — arquitetura RISC-V;
* **STM32WB55CGU6** — arquitetura Arm Cortex-M4.

A utilização do Zephyr RTOS busca reduzir o acoplamento do software aos recursos específicos de cada microcontrolador e facilitar a manutenção, os testes e a portabilidade da aplicação.

## Principais componentes

O sistema prevê a integração dos seguintes elementos:

* Sensor de álcool MQ303B
* Detector digital de sopro
* Circuito de controle do aquecimento do sensor
* Condicionamento e aquisição do sinal analógico
* Display OLED
* Comunicação Bluetooth LE
* Saída digital para atuação do periférico de inibição.

Modelos, interfaces, conexões, pinagem e demais definições físicas são documentados em [definições de hardware](docs/hardware.md).

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

## Organização do projeto

A documentação é dividida por responsabilidade, evitando a duplicação de informações.

| Documento                                                                     | Finalidade                                                                        |
| ----------------------------------------------------------------------------- | --------------------------------------------------------------------------------- |
| [Requisitos](docs/requisitos.md)                                              | Requisitos funcionais, não funcionais, restrições e critérios de aceitação        |
| [Definições de hardware](docs/hardware.md)                                    | Componentes, interfaces, conexões, pinagem e restrições físicas                   |
| [Arquitetura do software embarcado](docs/arquitetura/arquitetura-firmware.md) | Organização do software, componentes, responsabilidades e interação entre módulos |
| [Decisões de arquitetura](docs/arquitetura/decisoes/)                         | Registro das decisões arquiteturais relevantes e suas justificativas              |
| [Ambiente de desenvolvimento](docs/desenvolvimento/ambiente.md)               | Preparação, ferramentas, versões e validação do ambiente                          |
| [Plano de implementação](docs/desenvolvimento/plano-implementacao.md)         | Etapas previstas para desenvolvimento e validação                                 |
| [Visão geral](docs/visao-geral.md)                                            | Contexto condensado e estado atual do projeto                                     |
