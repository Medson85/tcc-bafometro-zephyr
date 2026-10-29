cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(teste_maquina)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../app/fontes.cmake)

target_include_directories(app PRIVATE ${BAFOMETRO_INCLUDES})
target_sources(app PRIVATE src/teste_maquina.c ${BAFOMETRO_FONTES})
