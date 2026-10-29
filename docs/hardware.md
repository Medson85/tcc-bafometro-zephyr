# Definições de hardware

Componentes, interfaces, conexões, pinagem e restrições físicas do bafômetro. A seção do ESP32-C3-MINI-1 foi auditada em 25/07/2026 contra as fontes da Espressif; as notas datadas de 28/10/2026 foram conferidas no código do Zephyr 4.4.0.

## Componentes do sistema

| Componente | Interface | Situação |
|---|---|---|
| Sensor de álcool Winsen MQ303B | Entrada analógica (divisor com resistor de carga) | Definido |
| Aquecedor do sensor (fases Q5/Q7) | Duas saídas digitais comandando drivers | Aquecimento e operação definidos; repouso pendente |
| Detector digital de sopro | Entrada digital com interrupção | Definido |
| Display OLED 1,5" 128×128 (GME128128-01-I2C) | I²C | Controlador pendente |
| Bluetooth LE | Rádio integrado (ESP32-C3) / coprocessador M0+ (STM32WB55) | Definido |
| Saída de inibição | Saída digital mantida, *fail-active* | Pino pendente |
| Botões de energia lógica e de interface | Entradas digitais | Definido |

## Pendências

1. **Controlador do display.** Confirmar no módulo físico (marcação ou datasheet do fornecedor) se é SH1107 ou SSD1327. Situação no Zephyr 4.4.0:
   - SSD1327: driver `solomon,ssd1327` (`CONFIG_SSD1327_5`). Aceita apenas `PIXEL_FORMAT_L_8` (1 byte por pixel, convertido internamente para 4 bits). O CFB trabalha com 1 bit por pixel e não serve a esse driver, então a tela exige renderização própria em L_8.
   - SH1107: não há binding. O mais próximo é `sinowealth,sh1106` (128×64, atendido pelo driver `display_ssd1306.c`); o uso com 128×128 não é suportado oficialmente e precisa de teste de bancada.
2. **Pino da saída de inibição.** Ver "Orçamento de pinos esgotado" e a nota sobre a DevKitM-1.
3. **Combinação de Q5 e Q7 no repouso do aquecedor.** O protótipo termina com os dois em nível alto; definir no esquemático.
4. **Datasheet do MQ303B.** Tensões e corrente do aquecedor, tempos (incluindo a origem dos 10 s de pré-aquecimento) e circuito de medição.
5. **Checklist antes do esquemático.** Ver a seção do ESP32-C3.

---

## Alvo ESP32-C3-MINI-1

**Módulo:** ESP32-C3-MINI-1-N4X · 4 MB flash · −40 a 85 °C · chip revision v1.1 · antena PCB integrada
**SoC:** ESP32-C3 (RISC-V RV32IMC, núcleo único, até 160 MHz) · 384 KB ROM · 400 KB SRAM (16 KB cache) · 8 KB SRAM RTC
**Rádio:** Wi-Fi 802.11 b/g/n 2,4 GHz (1T1R) · Bluetooth LE 5 + mesh · antena compartilhada por coexistência interna

> Sempre especificar o *part number* completo. `ESP32-C3-MINI-1` sozinho não define memória, faixa térmica nem revisão de silício. As variantes `-N4`, `-H4` e `-H4-AZ` estão marcadas como **NRND**.

### Mapa de pinos

| Sinal do projeto | GPIO | Direção | Função no silício | Observação |
|---|---:|---|---|---|
| `MQ303B_ADC` | 3 | Entrada analógica | `ADC1_CH3` | Único GPIO exposto sem função JTAG/FSPI alternativa |
| `HEAT_2V_EN` | 4 | Saída digital | `ADC1_CH4`, `FSPIHD`, `MTMS` | Comanda driver, nunca a carga |
| `HEAT_0V9_EN` | 5 | Saída digital | `ADC2_CH0`, `FSPIWP`, `MTDI` | `ADC2_CH0` é inutilizável (errata) |
| `OLED_SDA` | 6 | I²C0 SDA | `FSPICLK`, `MTCK` | Pull-up externo obrigatório |
| `OLED_SCL` | 7 | I²C0 SCL | `FSPID`, `MTDO` | Pull-up externo obrigatório |
| `AIRFLOW_DIG` | 10 | Entrada digital + IRQ | `FSPICS0` | Interrupção por borda ou nível |
| `BTN_POWER_LOGIC` | 0 | Entrada, pull-up | `ADC1_CH0`, `XTAL_32K_P` | Comando lógico, **não** corta energia |
| `BTN_UI` | 1 | Entrada, pull-up | `ADC1_CH1`, `XTAL_32K_N` | — |

Válido apenas se o projeto **não** usar cristal externo de 32,768 kHz (ocuparia GPIO0/GPIO1).

### Pinos reservados

| GPIO | Reservado para | Ação no esquemático |
|---:|---|---|
| 2 | Strapping | **Pull-up externo** (recomendação da Espressif contra glitches) |
| 8 | Strapping / controle de log da ROM | Sem carga; flutuante ou pull-up |
| 9 | Strapping / BOOT (pull-up interno, padrão `1`) | Sem carga; opcional botão para GND |
| 18, 19 | USB Serial/JTAG (`D−`, `D+`) | Gravação, log e depuração por USB |
| 20, 21 | UART0 (`U0RXD`, `U0TXD`) | Gravação e log por serial |

`GPIO2=1, GPIO8=1, GPIO9=0` no reset seleciona *joint download boot*.

### ⚠ Orçamento de pinos esgotado

O módulo expõe **15** GPIOs. A tabela acima usa 8 e a reserva protege 7. **Não há pino livre para a saída de inibição.** Decidir antes de fechar o esquemático:

| Opção | Ganho | Custo |
|---|---|---|
| Liberar UART0 (GPIO20/21) | +2 pinos | Perde a rota alternativa de gravação; o USB Serial/JTAG assume tudo |
| Usar GPIO2 | +1 pino | Requer pull-up no reset; com driver ativo-alto, o estado de boot coincide com "inibindo", coerente com `fail-active` |
| Expansor I²C | +N pinos | Função de segurança passa a depender da integridade do barramento — desaconselhado |
| Trocar de módulo | +N pinos | Refaz a fundamentação de hardware |

**Nota sobre a ESP32-C3-DevKitM-1 (28/10/2026).** A recomendação de liberar a UART0 vale para o módulo montado em placa própria. Na DevKitM-1, placa de referência do projeto:

- o console padrão da placa no Zephyr é a UART0 (`zephyr,console = &uart0`, TX em GPIO21 e RX em GPIO20), acessada pelo micro-USB por meio do conversor USB-UART da placa;
- o USB Serial/JTAG (GPIO18/19) só aparece no conector de pinos, e o arquivo da placa no Zephyr 4.4.0 registra que usá-lo exige ressoldar resistores;
- o log da ROM na partida vem habilitado de fábrica (eFuse `UART_PRINT_CONTROL` = 0) e sai pela UART0, então o GPIO21 alterna durante o boot — inadequado para uma saída *fail-active*.

Na DevKitM-1, liberar a UART0 exige mover o console para o USB Serial/JTAG, com a modificação da placa, e a opção GPIO2 ganha peso. Decidir conforme a placa em que o protótipo será montado e conferir no esquemático da DevKitM-1 como o conversor se liga a GPIO20/21.

### Restrições assumidas

- **JTAG externo indisponível.** GPIO4–GPIO7 carregam `MTMS`/`MTDI`/`MTCK`/`MTDO`. A depuração é feita pelo USB Serial/JTAG integrado.
- **ADC2 inutilizável.** A errata do SoC registra que o controlador digital do SAR ADC2 não funciona; no ESP32-C3 tanto o modo one-shot quanto o DMA dependem dele. Só o ADC1 (GPIO0–GPIO4) é utilizável.
- **`BTN_POWER_LOGIC` não desliga o módulo.** Quem liga/desliga o chip é o pino `EN`. Corte físico exige latch, load switch ou PMIC.

### Devicetree

Overlay da placa `esp32c3_devkitm` (arquivo `boards/esp32c3_devkitm.overlay` da aplicação):

```dts
#include <zephyr/dt-bindings/pinctrl/esp32c3-pinctrl.h>
#include <zephyr/dt-bindings/adc/adc.h>
#include <zephyr/dt-bindings/gpio/gpio.h>
#include <zephyr/dt-bindings/i2c/i2c.h>

&pinctrl {
	i2c0_bafometro: i2c0_bafometro {
		group1 {
			pinmux = <I2C0_SDA_GPIO6>, <I2C0_SCL_GPIO7>;
			bias-pull-up;
			drive-open-drain;
			output-high;
		};
	};
};

/* A placa habilita o SPI2 em GPIO2/6/7/10, que conflita com o I2C e o detector. */
&spi2 {
	status = "disabled";
};

&i2c0 {
	status = "okay";
	pinctrl-0 = <&i2c0_bafometro>;          /* a placa usa GPIO1/GPIO3 por padrão; GPIO3 é o ADC */
	pinctrl-names = "default";
	clock-frequency = <I2C_BITRATE_FAST>;   /* 400 kbit/s */
};

&adc0 {
	status = "okay";
	#address-cells = <1>;
	#size-cells = <0>;

	channel@3 {
		reg = <3>;                                    /* ADC1_CH3 = GPIO3 */
		zephyr,gain = "ADC_GAIN_1_4";                 /* atenuação 11 dB -> 0..2500 mV */
		zephyr,reference = "ADC_REF_INTERNAL";        /* única referência aceita */
		zephyr,acquisition-time = <ADC_ACQ_TIME_DEFAULT>;  /* não parametrizável */
		zephyr,resolution = <12>;                     /* única resolução no C3 */
	};
};

/ {
	zephyr,user {
		io-channels = <&adc0 3>;
		aquecimento2v-gpios = <&gpio0 4 GPIO_ACTIVE_HIGH>;    /* confirmar polaridade no esquemático */
		aquecimento0v9-gpios = <&gpio0 5 GPIO_ACTIVE_HIGH>;   /* confirmar polaridade no esquemático */
		detector-sopro-gpios = <&gpio0 10 (GPIO_ACTIVE_HIGH | GPIO_PULL_DOWN)>;
		btn-energia-gpios = <&gpio0 0 (GPIO_ACTIVE_LOW | GPIO_PULL_UP)>;
		btn-interface-gpios = <&gpio0 1 (GPIO_ACTIVE_LOW | GPIO_PULL_UP)>;
		/* inibidor-gpios: pino pendente (ver orçamento de pinos) */
	};
};
```

No código, os sinais são obtidos com `GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), aquecimento2v_gpios)` e `ADC_DT_SPEC_GET(DT_PATH(zephyr_user))`. O nó do ADC é `adc0` (`dts/riscv/espressif/esp32c3/esp32c3_common.dtsi`, Zephyr 4.4.0).

#### Limites do driver ADC (`espressif,esp32-adc`)

| Propriedade | Valor aceito no ESP32-C3 |
|---|---|
| `zephyr,resolution` | `12` apenas |
| `zephyr,reference` | `ADC_REF_INTERNAL` apenas — qualquer outra retorna `-ENOTSUP` |
| `zephyr,acquisition-time` | `ADC_ACQ_TIME_DEFAULT` apenas — **não** é parametrizável |
| Modo diferencial | Não suportado |
| Vários canais na mesma sequência | Não suportado fora do modo DMA |
| `extra_samplings` e `interval_us` | Não suportados fora do modo DMA — sobreamostragem só por laço de leituras |
| `calibrate = true` na sequência | Rejeitado (`-ENOTSUP`); a calibração de fábrica já é aplicada pelo driver |

Mapeamento ganho ↔ atenuação e faixa de entrada:

| `zephyr,gain` | Atenuação | Faixa efetiva | Erro total após calibração |
|---|---|---|---|
| `ADC_GAIN_1` | 0 dB | 0 – 750 mV | ±10 mV |
| `ADC_GAIN_4_5` | 2,5 dB | 0 – 1050 mV | ±10 mV |
| `ADC_GAIN_1_2` | 6 dB | 0 – 1300 mV | ±10 mV |
| `ADC_GAIN_1_4` | 11 dB | 0 – 2500 mV | ±35 mV |

O driver do Zephyr instancia e aplica o esquema de calibração de fábrica do ESP-IDF (*curve fitting* no C3) por canal e atenuação. A aplicação **não** chama `esp_adc_cal`; usa a API ADC do Zephyr e recebe o valor já calibrado.

> A calibração corrige ganho e offset **do conversor**. Ela não calibra o sensor: a relação tensão → concentração continua exigindo caracterização experimental própria.

### Alimentação

| Parâmetro | Valor |
|---|---|
| Tensão de operação | 3,0 / **3,3** / 3,6 V (mín. / típ. / máx.) |
| Máximo absoluto | 3,6 V |
| Corrente mínima da fonte | **0,5 A** — só para o módulo |
| Pico Wi-Fi TX | 350 mA (802.11b @20,5 dBm) |
| Pico BLE TX | 340 mA (@20,0 dBm) |
| Light-sleep / Deep-sleep / Power-off | 130 µA / 5 µA / 1 µA |
| `V_IH` / `V_IL` | ≥ 0,75·VDD / ≤ 0,25·VDD |
| Pull-up/pull-down internos | ~45 kΩ típico |

Somar display, condicionamento analógico, aquecedor e inibidor ao orçamento — os 0,5 A não os incluem.

**Pino EN:** alto habilita, baixo desliga, nunca flutuante. RC recomendado: **10 kΩ + 1 µF**. `t_STBL ≥ 50 µs` (estabilização antes de EN subir), `t_RST ≥ 50 µs` (EN baixo para reset), `t_H ≥ 3 ms` (leitura dos strapping pins).

**Layout:** respeitar a zona de keepout da antena; posicionar o módulo minimizando a interferência da placa-base. Soldar o EPAD ao GND é opcional, mas melhora o desempenho térmico. Desacoplamento de referência do módulo: 10 µF + 0,1 µF no barramento de 3,3 V — **não** na entrada do ADC.

### Gravação e log

```bash
west build -b esp32c3_devkitm/esp32c3 app
west flash
west espressif monitor
```

Duas rotas independentes preservadas: USB Serial/JTAG (GPIO18/19) e UART0 (GPIO20/21). Se a UART0 for liberada para a saída de inibição, resta apenas o USB.

> **Compatibilidade de silício:** as variantes recomendadas usam chip revision **v1.1**, que exige versões recentes do ESP-IDF. Como o Zephyr o consome via `hal_espressif`, a versão fixada na ADR 0001 (Zephyr 4.4.0) precisa reconhecer a v1.1 — confirmar na primeira gravação. Um módulo v1.1 com SDK antigo pode não inicializar.

### Checklist antes do esquemático

- [ ] Decidir o pino da saída de inibição (ver "orçamento de pinos")
- [ ] Pull-up externo em GPIO2
- [ ] Pull-ups do I²C dimensionados; verificar se o módulo do display já os traz
- [ ] Confirmar tensão de operação e endereço I²C do display
- [ ] Estado seguro externo (pull-up/pull-down) nas entradas dos drivers do aquecedor
- [ ] Polaridade ativa de `HEAT_2V_EN` e `HEAT_0V9_EN` derivada do esquemático, não presumida
- [ ] Intertravamento *break-before-make* entre as duas fases de aquecimento
- [ ] Faixa de saída condicionada do MQ303B compatível com a atenuação escolhida
- [ ] Níveis do detector de sopro compatíveis com `V_IL`/`V_IH`
- [ ] Rede RC no pino EN
- [ ] Zona de keepout da antena livre na placa-base
- [ ] Part number completo na lista de materiais
- [ ] Ensaio de ruído no ADC: rádio off, BLE ativo, Wi-Fi ativo, aquecedor comutando

### Alterações em relação à versão de 25/07/2026

- GPIOs do projeto movidos de um nó `gpio-keys` para o nó `zephyr,user`: `gpio-keys` é o binding de teclas de entrada e não serve para as saídas do aquecedor; `zephyr,user` é o que a arquitetura prevê.
- `&spi2` desabilitado no overlay: a placa `esp32c3_devkitm` o habilita em GPIO2/6/7/10.
- Comentário sobre o pinctrl padrão do `&i2c0` na placa (GPIO1/GPIO3).
- Nó do ADC confirmado como `adc0`.
- Limites adicionais do driver ADC (sequências, sobreamostragem e calibração).
- Nota sobre a DevKitM-1 no orçamento de pinos.
- Referência ao `west.yml` trocada pela ADR 0001.

---

## Alvo STM32WB55CGU6

Pinagem do bafômetro a definir na etapa 8 do [plano de implementação](desenvolvimento/plano-implementacao.md). Fatos da placa de referência, conferidos no Zephyr 4.4.0:

- **Placa:** WeAct Studio STM32WB55 Core Board, alvo `weact_stm32wb55_core/stm32wb55xx`, sem depurador embarcado.
- **Cortex-M4 (aplicação):** gravação por DFU (BOOT pressionado durante o RESET, com `dfu-util`; no Windows pode exigir driver instalado pelo Zadig) ou por sonda SWD com STM32CubeProgrammer.
- **Cortex-M0+ (rádio):** a placa sai de fábrica com o binário "Full Stack", **incompatível** com o Zephyr. É preciso gravar um binário HCI; a documentação da placa testou `stm32wb5x_BLE_HCILayer_extended_fw.bin` (v1.23.0, endereço 0x080DB000 na variante de 1 MB). Essa gravação **exige sonda SWD externa** (ST-LINK/V2 ou J-Link) no conector P3 e STM32CubeProgrammer; o DFU não grava o M0+.
- **Console:** USART1 (PA9/PA10), 115200 8N1.
- **Persistência:** partição `storage` de 8 KB já definida na placa.
- **Referência do ADC:** canal interno `vref` habilitado (alias `volt-sensor0`), útil para medir a referência real.
- **Bluetooth:** a API Bluetooth Host do Zephyr é a mesma nos dois alvos; só o transporte HCI muda.

---

## Fontes

| Documento | Uso |
|---|---|
| [ESP32-C3-MINI-1 & MINI-1U Datasheet v2.2](https://www.espressif.com/documentation/esp32-c3-mini-1_datasheet_en.pdf) | Pinagem, boot, alimentação, dimensões, esquemáticos |
| [ESP32-C3 Series Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf) | ISA RV32IMC, arquitetura, periféricos |
| [ESP32-C3 Hardware Design Guidelines](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32c3/) | Faixas do ADC por atenuação, layout |
| [ESP32-C3 Series SoC Errata](https://docs.espressif.com/projects/esp-chip-errata/en/latest/esp32c3/) | Restrição do SAR ADC2 |
| [ESP32-C3 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-c3_technical_reference_manual_en.pdf) | GPIO Matrix, IO MUX, registradores |
| [ESP32-C3-DevKitM-1 — User Guide](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32c3/esp32-c3-devkitm-1/user_guide.html) | USB e conversor USB-UART da placa |
| [eFuse `UART_PRINT_CONTROL` — ESP32-C3](https://docs.espressif.com/projects/rust/esp-hal/1.0.0-beta.0/esp32c3/esp_hal/efuse/constant.UART_PRINT_CONTROL.html) | Log da ROM habilitado por padrão |
| [Binding `espressif,esp32-adc`](https://docs.zephyrproject.org/latest/build/dts/api/bindings/adc/espressif%2Cesp32-adc.html) | Limites do driver ADC no Zephyr |
| Zephyr 4.4.0: `include/zephyr/dt-bindings/pinctrl/esp32c3-pinctrl.h` | Macros de pinmux I²C/UART |
| Zephyr 4.4.0: `drivers/adc/adc_esp32.c` · `drivers/gpio/gpio_esp32.c` | Comportamento efetivo dos drivers |
| Zephyr 4.4.0: `boards/espressif/esp32c3_devkitm/` | Periféricos e pinos habilitados por padrão na placa |
| Zephyr 4.4.0: `boards/weact/stm32wb55_core/` | Placa do STM32WB55, coprocessador e gravação |
| Zephyr 4.4.0: `drivers/display/display_ssd1327_5.c` · `subsys/fb/cfb.c` | Formatos de pixel do SSD1327 e do CFB |

Seção do ESP32-C3 verificada em 25/07/2026 contra o datasheet v2.2 (publicado em 06/05/2026). Notas de 28/10/2026 conferidas no Zephyr 4.4.0 (tag `v4.4.0`).
