cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(teste_dominio)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../app/fontes.cmake)

target_include_directories(app PRIVATE ${BAFOMETRO_INCLUDES})
target_sources(app PRIVATE
  src/teste_calibracao.c
  src/teste_conversao.c
  src/teste_decisao.c
  src/teste_estatistica.c
  src/teste_filtro.c
  src/teste_medicao.c
  src/teste_qualidade.c
  ${BAFOMETRO_FONTES_DOMINIO}
)
