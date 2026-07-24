# Decisão 0001 — Versão de referência do Zephyr

- Estado: aceita
- Data: 24 de julho de 2026

## Contexto

O desenvolvimento precisa ser reproduzível nas duas plataformas previstas.

O uso de uma versão em desenvolvimento do Zephyr poderia introduzir alterações não controladas durante a realização do trabalho.

## Decisão

Adotar o Zephyr RTOS 4.4.0 como versão de referência inicial.

O ambiente correspondente utiliza:

- Zephyr RTOS 4.4.0;
- Zephyr SDK 1.0.1;
- Python 3.12.10;
- West 1.5.0;
- CMake 4.4.0.

## Consequências positivas

- maior reprodutibilidade;
- redução de mudanças inesperadas;
- ambiente comum para ESP32-C3 e STM32WB55;
- documentação baseada em uma versão estável;
- maior facilidade para repetir os testes.

## Restrições

- atualizações deverão ser planejadas;
- mudanças de versão deverão ocorrer separadamente;
- as duas plataformas deverão ser novamente validadas após uma atualização;
- incompatibilidades deverão ser documentadas.