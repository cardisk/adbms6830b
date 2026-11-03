# ADBMS6830B Standalone ADI `no-OS` Driver Guide & Architectural Reference

**Target Part:** Analog Devices ADBMS6830B 16-Channel Multicell Battery Stack Monitor  
**isoSPI Transceiver:** ADBMS6822CCSZ (Dual Transceiver, Channel 1 Active)  
**Host MCU:** STM32H723VGT6 (Arm Cortex-M7 @ 550 MHz)  
**Application:** Formula Student (FSG / FSAE) High Voltage Accumulator Management System (AMS)  
**Framework:** Analog Devices `no-OS` Driver Specification (Zero-Heap Static Allocation Variant)

---

## 1. Hardware Architecture & Schematic Ground Truth

The driver is locked to the physical master board schematic (`Firenze Race Team - Università degli Studi di Firenze`). Hardware cannot be modified.

### 1.1 Microcontroller Pin Mapping (STM32H723VGT6)

| Signal Net | MCU Pin | Function | Schematic Route & Description |
| :--- | :--- | :--- | :--- |
| `SPI_SCK` | Pin 29 (`PA5`) | `SPI1_SCK` (AF5) | Clock out through $100\,\Omega$ `R26` to `IC8` Pin 4. |
| `SPI_MISO` | Pin 30 (`PA6`) | `SPI1_MISO` (AF5) | Data in from `IC8` Pin 3 through $100\,\Omega$ `R25`. |
| `SPI_MOSI` | Pin 31 (`PA7`) | `SPI1_MOSI` (AF5) | Data out through $100\,\Omega$ `R24` to `IC8` Pin 2. |
| `SPI_CS` | Pin 92 (`PB6`) | GPIO Output | Active-Low Chip Select through $100\,\Omega$ `R27` to `IC8` Pin 5. |
| `SPI_INTR` | Pin 67 (`PA8`) | GPIO Input / EXTI8 | Interrupt indicator from `IC8` Pin 7. |
| `SPI_WAKE` | Pin 72 (`PB10`) | GPIO Output | Transceiver wake line driving `IC8` Pin 8 & `Q4` LED gate. |

### 1.2 Transceiver Wiring (`IC8` ADBMS6822CCSZ) & Physical Topology
1. **Unidirectional Single-Port Bus:**
   - Only Channel 1 of the dual-channel `ADBMS6822` is connected.
   - Channel 2 pins (`PICO2`, `POCI2`, `SCK2`, `CS2`, `IP2`, `IM2`, Pins 18–32) are unrouted and floating.
   - The master board has a single 2-pin connector: **`J8`** (Molex DuraClik, nets `SPI_IP` / `SPI_IM`).
   - **Verdict:** The physical bus is **strictly single-direction**. Reversible dual-master isoSPI ring topology is physically impossible on this hardware.
2. **Hardwired SPI Mode 0 ($\text{CPOL}=0, \text{CPHA}=0$):**
   - Transceiver Pin 14 (`PHAPOL`) is **connected directly to Ground (`NLVSS`)**.
   - `PHAPOL = 0` locks the ADBMS6822 interface to **SPI Mode 0**.
   - Clock line `SCK` must idle low; data is latched on the rising clock edge and shifted out on the falling edge.
3. **Hardware Resistor Hazard (`R4` on `PB6`):**
   - Resistor `R4` ($10\,\text{k}\Omega$) on Sheet 2 connects `SPI_CS` to `GND`.
   - **Hazard:** Pulls active-low CS low during MCU boot, reset, or tristate mode, asserting the transceiver unintentionally.
   - **Mandatory Firmware Workaround:** Firmware must immediately configure `PB6` as Push-Pull High-Speed Output and force it **HIGH** in early initialization (`main.c` / `SystemInit`) before executing any bus transactions.

---

## 2. isoSPI Daisy Chain Frame Mechanics

### 2.1 1-IC (12 Bytes) vs. 11-IC (92 Bytes) Transaction Sizing

```
WRITE FRAME (WRCFGA, WRCFGB):
Length = 4 Bytes Header + (N * 8 Bytes Payload+PEC10)
  N = 1  (Bench Test):  4 + (1 * 8)  = 12 Bytes
  N = 11 (Full Pack):   4 + (11 * 8) = 92 Bytes

MOSI Data Order:
[CMD0] [CMD1] [PEC0] [PEC1] [Node N-1 Data+PEC] ... [Node 0 Data+PEC]
(Top-of-Stack IC data is sent FIRST; Bottom IC closest to MCU is sent LAST).

READ FRAME (RDCVA..RDCVF, RDCFGA):
Length = 4 Bytes Header + (N * 8 Bytes Dummy Clocking)
  N = 1  (Bench Test):  4 + (1 * 8)  = 12 Bytes
  N = 11 (Full Pack):   4 + (11 * 8) = 92 Bytes

MISO Receipt Order:
[CMD Echo / Dummy (4B)] [Node 0 Data+PEC (8B)] ... [Node N-1 Data+PEC (8B)]
(Bottom IC closest to MCU arrives FIRST; Top-of-Stack IC arrives LAST).
```

### 2.2 Command Counter (`CCNT`) & Data PEC (`PEC10`)
Each 8-byte received data block contains:
- Bytes 0–5: 6 bytes of register data.
- Byte 6: Bits [7:2] = 6-bit Command Counter (`CCNT[5:0]`), Bits [1:0] = `PEC[9:8]`.
- Byte 7: Bits [7:0] = `PEC[7:0]`.

**Validation Rules:**
1. Hardware increments `CCNT` on valid Write commands only if the received `PEC10` matches.
2. Rollover after 63 is **1, NOT 0** (0 indicates power-on reset or unexpected sleep).
3. The driver calculates `PEC10` over Bytes 0–5 and factors in `CCNT` from Byte 6 before comparing with `rx_pec`.

### 2.3 Wakeup Timing Across the Chain
- $t_{\text{WAKE}} = 500\,\mu\text{s}$ (Regulator start-up time from `SLEEP` to `STANDBY`).
- $t_{\text{READY}} = 10\,\mu\text{s}$ (isoSPI receiver enable time from `IDLE`).
- $t_{\text{IDLE}} = 4.3\text{--}6.7\,\text{ms}$ (isoSPI idle timeout).
- To wake $N$ chained ICs from `SLEEP`, the MCU transmits $N$ successive wake pulses separated by $500\,\mu\text{s}$ ($T_{\text{wake}} \approx N \times 500\,\mu\text{s}$).

---

## 3. Legacy FS Code Bug Fixes & Discarded Elements

The legacy library in `Third_Parties/ADBMS6830` was audited and discarded due to critical flaws:

1. **Elimination of Dynamic Memory Allocation:**
   - Legacy functions (`spiReadData`, `adBmsWriteData`, `adBms6830ParseCell`) called `calloc()`, `free()`, and `exit(0)` on every SPI transfer.
   - **Fix:** Zero dynamic memory allocation. Descriptors, array mirrors, and DMA frame buffers are statically dimensioned (`ADBMS6830B_MAX_ICS 11`).
2. **Rectified Voltage Conversion Mathematics:**
   - Legacy code used `((data + 10000) * 0.000150)`, which misinterpreted negative differential readings and overflowed unsigned integers to $> 11\,\text{V}$.
   - **Fix:** Signed 16-bit integer conversion:
     $$V_{\text{cell}} = \frac{(\text{int16\_t})\text{raw} \times 150}{1000} + 1500\quad [\text{mV}]$$
3. **Latent Fault Injection Defeat:**
   - Legacy `adBms6830_init_config` intentionally set `FLAG_D[0]` and `FLAG_D[1]`, forcing permanent oscillator failures.
   - **Fix:** `FLAG_D` is initialized strictly to `0x00`.
4. **Fast Table-Driven CRC-10 Engine:**
   - Replaced legacy bit-banging modulo-2 division loops with a 256-entry Flash lookup table (`adbms6830b_crc10_table`).
5. **Real-Time Data Integrity Validation:**
   - The legacy code printed PEC status to UART but ignored errors in logic.
   - **Fix:** `adbms6830b_read_reg_group()` returns `-EBADMSG` on any PEC or Command Counter mismatch.

---

## 4. Standalone File Structure (`Third_Parties/adbms6830b/`)

All files are self-contained with flattened `#include` headers:

```
Third_Parties/adbms6830b/
├── adbms6830b.h         # Master Driver Header (API, Register Map, Structs)
├── adbms6830b.c         # Clean Driver Implementation (Zero Runtime Heap)
├── no_os_alloc.h        # Memory allocation interface wrapper
├── no_os_alloc.c        # Weak allocator stubs
├── no_os_delay.h        # Delay HAL interface
├── no_os_error.h        # Standard POSIX error codes
├── no_os_gpio.h         # GPIO HAL interface
├── no_os_gpio.c         # GPIO dispatcher
├── no_os_mutex.h        # Minimal standalone mutex stubs
├── no_os_spi.h          # SPI HAL interface
├── no_os_spi.c          # SPI dispatcher
├── no_os_util.h         # Linux bitfield macros (NO_OS_BIT, NO_OS_GENMASK)
├── stm32_delay.c        # DWT microsecond timer
├── stm32_gpio.h         # STM32 GPIO platform types
├── stm32_gpio.c         # STM32 GPIO register driver (BSRR atomic)
├── stm32_hal.h          # Bridge to main.h / stm32h7xx_hal.h
├── stm32_spi.h          # STM32 SPI platform types
└── stm32_spi.c          # STM32 SPI driver (Mode 0, CS BSRR, TransmitReceive)
```

---

## 5. 1-IC Bench Bring-Up Tutorial (`main.c`)

```c
#include <stdio.h>
#include "main.h"
#include "adbms6830b.h"
#include "stm32_spi.h"
#include "stm32_gpio.h"

/* Static allocation for safety-critical determinism */
static struct adbms6830b_dev bms_dev;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_SPI1_Init();

    /* Force PB6 CS HIGH immediately to override R4 pull-down */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

    /* Bind STM32 SPI1 platform parameters */
    struct stm32_spi_init_param stm32_spi_extra = {
        .chip_select_port = 1, /* GPIOB (0=A, 1=B, 2=C) */
        .get_input_clock = HAL_RCC_GetPCLK2Freq,
    };

    struct no_os_spi_init_param spi_init_param = {
        .device_id = 1,                 /* SPI1 */
        .max_speed_hz = 2000000,        /* 2.0 MHz isoSPI safe limit */
        .chip_select = 6,               /* PB6 */
        .mode = NO_OS_SPI_MODE_0,       /* HARDWIRED MODE 0 (PHAPOL = GND) */
        .bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
        .platform_ops = &stm32_spi_ops,
        .extra = &stm32_spi_extra,
    };

    struct adbms6830b_init_param bms_init = {
        .spi_init = &spi_init_param,
        .num_ics = 1,                   /* Bench test with 1 IC */
        .filter_corner = ADBMS6830B_FC_45HZ,
        .comp_threshold = ADBMS6830B_CTH_8_1MV,
    };

    /* Initialize Driver */
    if (adbms6830b_init(&bms_dev, &bms_init) != 0) {
        printf("Init Failed!\r\n");
        while (1);
    }

    /* 1. Wake up the IC */
    adbms6830b_wakeup_chain(&bms_dev);

    /* 2. Reset Command Counter */
    adbms6830b_send_cmd(&bms_dev, ADBMS6830B_CMD_RSTCC, false);
    bms_dev.expected_ccnt = 0;

    /* 3. Write Configuration A */
    uint8_t tx_cfga[1][ADBMS6830B_REG_DATA_SIZE];
    tx_cfga[0][0] = ADBMS6830B_CFGAR0_REFON | ADBMS6830B_CFGAR0_CTH_SET(ADBMS6830B_CTH_8_1MV);
    tx_cfga[0][1] = 0x00;
    tx_cfga[0][2] = 0x00;
    tx_cfga[0][3] = 0xFF;
    tx_cfga[0][4] = 0x03;
    tx_cfga[0][5] = ADBMS6830B_CFGAR5_FC_SET(ADBMS6830B_FC_45HZ);
    adbms6830b_write_reg_group(&bms_dev, ADBMS6830B_CMD_WRCFGA, tx_cfga);

    /* 4. Read back Configuration A */
    uint8_t rx_cfga[1][ADBMS6830B_REG_DATA_SIZE];
    int32_t ret = adbms6830b_read_reg_group(&bms_dev, ADBMS6830B_CMD_RDCFGA, rx_cfga);
    if (ret == 0) {
        printf("Readback Valid! CFGAR0: 0x%02X, CCNT: %d\r\n",
               rx_cfga[0][0], bms_dev.nodes[0].last_ccnt);
    } else {
        printf("Readback Fault! Ret: %ld\r\n", ret);
    }

    while (1) {
        /* Start continuous cell conversion */
        adbms6830b_send_cmd(&bms_dev, ADBMS6830B_CMD_ADCV_BASE, true);
        no_os_mdelay(10);

        /* Read cell voltages */
        if (adbms6830b_read_cell_voltages(&bms_dev) == 0) {
            printf("Cell 1: %ld mV, Cell 2: %ld mV, Cell 3: %ld mV\r\n",
                   bms_dev.nodes[0].cell_mv[0],
                   bms_dev.nodes[0].cell_mv[1],
                   bms_dev.nodes[0].cell_mv[2]);
        }
        no_os_mdelay(100);
    }
}
```

---

## 6. Scaling from 1 IC to 11 ICs

Because the driver was architected around static max dimensioning (`ADBMS6830B_MAX_ICS 11`), scaling up from the single bench unit to the full 11-node accumulator pack requires changing **only one configuration parameter**:

```c
struct adbms6830b_init_param bms_init = {
    .spi_init = &spi_init_param,
    .num_ics = 11,   /* Scaled from 1 to 11 */
    ...
};
```

**What automatically happens:**
1. `adbms6830b_wakeup_chain()` generates 11 wake pulses across the chain ($5.5\,\text{ms}$).
2. `adbms6830b_write_reg_group()` formats 92-byte frames, packing Node 10 data first down to Node 0.
3. `adbms6830b_read_reg_group()` clocks out 92 bytes, validating `PEC10` and `CCNT` for all 11 nodes.
4. `adbms6830b_read_cell_voltages()` populates all 176 cell readings in `bms_dev.nodes[0..10].cell_mv[0..15]`.
