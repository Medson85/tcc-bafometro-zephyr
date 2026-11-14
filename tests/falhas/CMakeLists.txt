cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(teste_falhas)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../app/fontes.cmake)

# Máquina e domínio reais; dispositivos substituídos por dublês que injetam falhas.
target_include_directories(app PRIVATE ${BAFOMETRO_INCLUDES} src)
target_sources(app PRIVATE
  src/teste_falhas.c
  src/dispositivos_falsos.c
  ${BAFOMETRO_FONTES_DOMINIO}
  ${BAFOMETRO_FONTES_APLICACAO}
)
