# Plano de implementação

Etapas de desenvolvimento e validação, na ordem do escopo consolidado v2. A arquitetura de cada etapa está em [arquitetura do software embarcado](../arquitetura/arquitetura-firmware.md).

## Pré-requisitos

- **Ambiente de simulação:** o `native_sim` só executa em Linux. No Windows, exige WSL 2 ou máquina virtual; o WSL 1 não executa o binário de 32 bits padrão. Decidir entre montar o Zephyr 4.4.0 no WSL 2 ou rodar as etapas 1 e 2 direto no ESP32-C3, e registrar a decisão em `arquitetura/decisoes/`. A mudança de ambiente segue separada das funcionalidades.
- **Métricas de portabilidade:** definidas antes da etapa 8 (ver seção própria).
- **Sonda SWD** (ST-LINK/V2 ou J-Link): obrigatória para gravar o binário HCI no coprocessador do STM32WB55 (etapa 8).
- **Definições de hardware:** controlador do display (etapa 4), pino do inibidor (etapa 3) e repouso de Q5/Q7 (etapa 7). Ver [definições de hardware](../hardware.md).

## Etapas

| Etapa | Entrega | Depende de | Concluída quando |
|---:|---|---|---|
| 1 | Esqueleto: SMF, fila, temporizador e geração, com estados só com LOG e tempos reduzidos | Ambiente de simulação | O fluxo completo, incluindo sopro interrompido e economia, roda sem hardware |
| 2 | Domínio puro portado do protótipo, com ztest | 1 | Os testes de domínio passam |
| 3 | Dispositivos no ESP32-C3: `sensor_alcool` (fases e ADC em mV), `detector_sopro` e `inibidor` *fail-active* | 2; pino do inibidor; polaridades de Q5/Q7 | Primeiro sopro real medido |
| 4 | Tela: renderização mínima e `tela.c` | Controlador do display confirmado | Todas as mensagens do fluxo exibidas |
| 5 | BLE: pacote, estado e comando | 3 | Pacote e comando verificados com um cliente BLE |
| 6 | Persistência do R0 e estado CALIBRANDO_R0 | 3, 5 | O R0 sobrevive à reinicialização |
| 7 | Modo economia básico | Repouso de Q5/Q7 definido | Entrada e saída conforme os requisitos |
| 8 | Porte para o STM32WB55: overlay da WeAct, binário HCI no M0+ e checklist | 1 a 7; sonda SWD; métricas definidas | Mesmo código de aplicação nos dois alvos, com o checklist completo |
| 9 | Campanha metrológica | 3 a 8 | Protocolos executados e resultados registrados |

Situação:

- **Etapa 1 concluída.** O esqueleto está em `app/` e os testes da máquina em `tests/maquina`. Ele roda nos dois caminhos do pré-requisito de simulação: no `native_sim`, com o roteiro de simulação, e na DevKitM-1, ligando o GPIO10 ao 3V3 para simular o sopro.
- **Etapa 2 concluída.** A lógica de cálculo está em `app/src/dominio` e os testes em `tests/dominio`. A máquina já usa a decisão de inibição; na etapa 3, a coleta passa a registrar as amostras do ADC e PROCESSANDO passa a chamar `medicao_calcular()`.

## Métricas de portabilidade

A pergunta central do trabalho é o esforço de migração entre as plataformas. Para que o porte gere dado comparável, as métricas precisam estar definidas antes da etapa 8. Candidatas:

- arquivos e linhas alterados no porte, por categoria (overlay, `prj.conf`, aplicação, dispositivos);
- testes de domínio e checklist de hardware aprovados nos dois alvos sem alteração de código;
- tempo gasto no porte;
- consistência metrológica entre os alvos, com o mesmo protocolo e resultados comparáveis dentro da incerteza.

Para medir as alterações, marcar o início e o fim do porte com etiquetas do Git e comparar com `git diff --stat`.

## Campanha metrológica (etapa 9)

- **Taxa de amostragem:** capturar sopros reais a 200 Hz, confirmar que a energia do sinal fica abaixo de 1–2 Hz e fixar a taxa em 10 a 20 vezes essa banda (proposta: 20 Hz).
- **Janela da média móvel:** aplicar N ∈ {4, 8, 16, 32} aos sopros gravados e escolher o menor N a partir do qual o desvio residual para de melhorar (proposta: 8).
- **Limiares de qualidade:** revalidar os limites de CV de 2, 5 e 10 %, calibrados com 5 amostras.
- **Conversão para mg/L:** calibração ponta a ponta de Rs/R0 para mg/L com concentrações de referência (simulador de gás úmido com solução-padrão de etanol, ou comparação pareada com etilômetro de referência). Enquadramento: OIML R 126 e regulamento do Inmetro para etilômetros; o dispositivo é didático, não evidencial.
- **Limite de inibição:** limite funcional = limiar de decisão − incerteza expandida U (k = 2), com histerese na liberação.
- **ADC:** offset, ganho e repetibilidade contra fonte e multímetro de referência, nos dois alvos.
- **Tempo entre testes:** levantar o tempo de recuperação do sensor após um sopro com álcool.
- **Ergonomia:** validar a duração de 5 s com usuários.
