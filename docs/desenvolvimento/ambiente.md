# Ambiente de desenvolvimento

## Workspace oficial

O ambiente oficial do projeto está localizado em:

C:\Dev\zephyr-workspace

O projeto está localizado em:

C:\Dev\zephyr-workspace\tcc-bafometro-zephyr

## Componentes instalados

| Componente | Versão |
|---|---:|
| Python | 3.12.10 |
| West | 1.5.0 |
| Zephyr RTOS | 4.4.0 |
| Zephyr SDK | 1.0.1 |
| CMake | 4.4.0 |
| Compilador GNU | 14.3.0 |
| Ligador GNU | 2.43.1 |

## Ambiente virtual

O ambiente virtual está localizado em:

C:\Dev\zephyr-workspace\.venv

Ele deve ser ativado antes do uso das ferramentas do Zephyr.

## Verificações realizadas

Foram confirmados:

- Python 3.12.10;
- West 1.5.0;
- Zephyr RTOS 4.4.0;
- Zephyr SDK 1.0.1;
- dependências Python sem inconsistências;
- compilação para ESP32-C3;
- compilação para STM32WB55.

## Alvos validados

### ESP32-C3

Alvo Zephyr: esp32c3_devkitm/esp32c3

### STM32WB55

Alvo Zephyr: weact_stm32wb55_core/stm32wb55xx

## Regras de manutenção

- O código do projeto não deve ser criado dentro da pasta zephyr.
- As pastas modules, bootloader e tools não devem ser alteradas diretamente.
- Diretórios de compilação não devem ser registrados no Git.
- Atualizações do Zephyr e do SDK devem ser documentadas.
- Alterações de ambiente devem ser separadas das alterações funcionais.
- As duas plataformas devem ser validadas após mudanças relevantes.