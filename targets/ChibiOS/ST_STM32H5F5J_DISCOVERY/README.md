## Configuration of ChibiOS, HAL and MCU

The board definition files (`board.c` and `board.h`) are copied, unchanged, from ChibiOS `os/hal/boards/ST_STM32H5F5J_DK`.

This target requires ChibiOS `master` (STM32H5F5 is not available in the stable branches).

In _halconf.h_ (in both nanoBooter and nanoCLR folders), when compared with a default ChibiOS file:
- SERIAL_DEFAULT_BITRATE to 921600
- SERIAL_BUFFERS_SIZE to 64

In _mcuconf.h_ (in both nanoBooter and nanoCLR folders), when compared with the default file available from ChibiOS (`demos/STM32/RT-STM32-MULTI/cfg/stm32h5f5lj_discovery`):
- STM32_SERIAL_USE_USART2 to TRUE (ST-LINK virtual COM port, used for the wire protocol)
- STM32_HSIDIV_VALUE set to 1 and STM32_I2C3SEL set to the HSI kernel clock (I2C timings are computed for 64MHz)
- STM32_I2C_USE_DMA set to FALSE
- STM32_UART_USE_UART7 and STM32_UART_USE_UART5 to TRUE (nanoCLR only)
- STM32_LSI_ENABLED to TRUE (required by the watchdog)
- STM32_MAC_IP_CHECKSUM_OFFLOAD set to 3 (lwIP in nanoFramework has the software checksums disabled)

NOTE: this configuration uses the Serial port through the onboard ST Link USB connection.

## Debugging with OpenOCD

Use an STMicroelectronics OpenOCD build with STM32H5E/H5F support (device ID `0x47A`, 4MB flash), together with its matching scripts.
Stock OpenOCD 0.12.0 does not include the STM32H5 flash driver; copying `target/stm32h5x.cfg` into that installation is not sufficient.
The [ST OpenOCD fork](https://github.com/STMicroelectronics/OpenOCD) includes the required driver support.

Both launch profiles use `interface/stlink-dap.cfg` and explicitly select `dapdirect_swd` before loading `target/stm32h5x.cfg`.
The legacy `interface/stlink.cfg` uses HLA, which the STM32H5 target script does not support.
Set `debugServerPath` to the ST OpenOCD executable and the `-s` argument in `debugServerArgs` to its matching scripts directory.

## Flash layout

The STM32H5F5 has 4MB of flash organized in two banks of 2MB, with sectors of 8kB.

| Sectors | Address    | Usage                                           |
|---------|------------|-------------------------------------------------|
| 0-7     | 0x08000000 | nanoBooter (64kB)                               |
| 8-15    | 0x08010000 | configuration block (64kB)                      |
| 16-255  | 0x08020000 | nanoCLR (1920kB, rest of the first bank)        |
| 256-511 | 0x08200000 | deployment (2MB, second bank)                   |

The deployment area is in the second bank so programming it doesn't stall the code that is running from the first bank.

Flash is programmed in flash words of 128 bits with ECC, which means that a flash word can be programmed only once after an erase.
The instruction cache is invalidated after any program or erase operation.

## RAM layout

SRAM1 + SRAM2 + SRAM3 (768kB, contiguous): stacks, data, BSS, C heap, CLR managed heap, Ethernet DMA buffers and GPDMA links.
SRAM4 and SRAM5 are not used.

## ADC configurations

The following ADC channels (and respective GPIO pins) are available to the managed API, in the respective index:
- PA0 ADC1 INP0 (Arduino A0)
- PC0 ADC1 INP10 (Arduino A3)
- PC1 ADC1 INP11 (Arduino A4)
- PC3 ADC1 INP13 (Arduino A5)

Arduino A1 and A2 (PE12, PE13) are ADC3 inputs, which are not supported (yet).

More pins can be added at runtime with `nanoFramework.Hardware.Stm32.Configuration.SetPinFunction`.

## Peripherals

- Wire protocol: USART2, TX PA2, RX PA3 (ST-LINK virtual COM port)
- COM1: UART7, TX PE8 (Arduino D1), RX PE7 (Arduino D0)
- I2C3: SCL PH7 (Arduino D15), SDA PH8 (Arduino D14)
- SPI1: SCK PG11 (Arduino D13), MISO PG9 (Arduino D12), MOSI PB5 (Arduino D11)
- DAC1 channel 1: PA4 (Arduino D10)
- PWM: TIM1, TIM3, TIM4, TIM5, TIM8
- 1-Wire: UART5, TX PC12 (Arduino D5)
- Ethernet: RMII
- User button: PI8 (reads low when pressed). In nanoBooter, keep it pressed during reset to stay in the booter.
- LEDs: PH10 (red), PA5 (green), PB9 (blue), PD15 (orange). All are active low.

## Not supported (yet)

- SD card and file system: the SDMMC1 pins are shared with the OCTOSPI memories
- USB: the device has OTG controllers (not USB DRD), which are not supported by ChibiOS for this series yet
- Graphics (LTDC parallel RGB display)
- CAN (FDCAN)
- ADC3, internal ADC channels (temperature sensor, VREFINT, VBAT)
- External OCTOSPI NOR flash and PSRAM

## Floating point

The Cortex-M33 FPU in STM32H5 has single-precision only. The build adds support for double-precision floating point which is
done in software, meaning that `System.Math` API supports both the `float` and the `double` overloads.
