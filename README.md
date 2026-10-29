# TCC — Bafômetro Embarcado com Zephyr RTOS

Este repositório reúne a documentação para o Trabalho de Conclusão de Curso que consiste no desenvolvimento de um sistema embarcado para um bafômetro utilizando o Zephyr RTOS, com foco em portabilidade, organização do software e reutilização entre diferentes plataformas de microcontroladores.

## Objetivo

Desenvolver uma aplicação embarcada capaz de realizar o processo de medição de álcool utilizando o sensor **MQ303B**, executando a mesma lógica de aplicação em plataformas baseadas em arquiteturas distintas.

As plataformas previstas são:

* **ESP32-C3-MINI-1** — arquitetura RISC-V.
* **STM32WB55CGU6** — arquitetura Arm Cortex-M4.

A utilização do Zephyr RTOS busca reduzir o acoplamento do software aos recursos específicos de cada microcontrolador e facilitar a manutenção, os testes e a portabilidade da aplicação.

## Principais componentes

O sistema prevê a integração dos seguintes elementos:

* Sensor de álcool MQ303B.
* Detector digital de sopro.
* Circuito de controle do aquecimento do sensor.
* Condicionamento e aquisição do sinal analógico.
* Display OLED.
* Comunicação Bluetooth LE.
* Saída digital para atuação do periférico de inibição.

Modelos, interfaces, conexões, pinagem e demais definições físicas são documentados em [definições de hardware](docs/hardware.md).

## Ambiente de referência

- Zephyr RTOS 4.4.0
- Zephyr SDK 1.0.1
- Python 3.12.10
- West 1.5.0
- CMake 4.4.0
- Windows de 64 bits

A configuração completa está registrada em [ambiente de desenvolvimento](docs/desenvolvimento/ambiente.md).

## Plataformas previstas

### ESP32-C3

- Placa de referência: ESP32-C3 DevKitM
- Arquitetura: RISC-V
- Alvo Zephyr: `esp32c3_devkitm/esp32c3`

### STM32WB55

- Placa de referência: WeAct STM32WB55 Core
- Arquitetura: Arm Cortex-M4
- Alvo Zephyr: `weact_stm32wb55_core/stm32wb55xx`

## Compilação e testes

Na raiz do repositório, dentro do workspace do Zephyr:

| Alvo | Comando |
| --- | --- |
| ESP32-C3 | `west build -p always -b esp32c3_devkitm/esp32c3 app -d build/esp32c3` |
| STM32WB55 | `west build -p always -b weact_stm32wb55_core/stm32wb55xx app -d build/stm32wb55` |
| Simulação (Linux ou WSL 2) | `west build -p always -b native_sim app -d build/native_sim -t run` |
| Testes da máquina (Linux ou WSL 2) | `west build -p always -b native_sim tests/maquina -d build/testes-maquina -t run` |
| Testes do domínio (Linux ou WSL 2) | `west build -p always -b native_sim tests/dominio -d build/testes-dominio -t run` |

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
