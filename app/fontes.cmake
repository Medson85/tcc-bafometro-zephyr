# Fontes compartilhadas entre a aplicação e os testes.

set(BAFOMETRO_DIR ${CMAKE_CURRENT_LIST_DIR})

set(BAFOMETRO_INCLUDES ${BAFOMETRO_DIR}/src)

# Domínio: sem dependência do Zephyr; também compilado pelos testes de domínio.
set(BAFOMETRO_FONTES_DOMINIO
  ${BAFOMETRO_DIR}/src/dominio/calibracao.c
  ${BAFOMETRO_DIR}/src/dominio/conversao.c
  ${BAFOMETRO_DIR}/src/dominio/decisao.c
  ${BAFOMETRO_DIR}/src/dominio/estatistica.c
  ${BAFOMETRO_DIR}/src/dominio/filtro_media_movel.c
  ${BAFOMETRO_DIR}/src/dominio/medicao.c
  ${BAFOMETRO_DIR}/src/dominio/qualidade.c
)

set(BAFOMETRO_FONTES
  ${BAFOMETRO_FONTES_DOMINIO}
  ${BAFOMETRO_DIR}/src/aplicacao/bafometro.c
  ${BAFOMETRO_DIR}/src/dispositivos/ble.c
  ${BAFOMETRO_DIR}/src/dispositivos/detector_sopro.c
  ${BAFOMETRO_DIR}/src/dispositivos/inibidor.c
  ${BAFOMETRO_DIR}/src/dispositivos/sensor_alcool.c
  ${BAFOMETRO_DIR}/src/dispositivos/tela.c
)
