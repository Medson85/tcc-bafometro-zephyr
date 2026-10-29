# Fontes compartilhadas entre a aplicação e os testes.

set(BAFOMETRO_DIR ${CMAKE_CURRENT_LIST_DIR})

set(BAFOMETRO_INCLUDES ${BAFOMETRO_DIR}/src)

set(BAFOMETRO_FONTES
  ${BAFOMETRO_DIR}/src/aplicacao/bafometro.c
  ${BAFOMETRO_DIR}/src/dispositivos/ble.c
  ${BAFOMETRO_DIR}/src/dispositivos/detector_sopro.c
  ${BAFOMETRO_DIR}/src/dispositivos/inibidor.c
  ${BAFOMETRO_DIR}/src/dispositivos/sensor_alcool.c
  ${BAFOMETRO_DIR}/src/dispositivos/tela.c
)
