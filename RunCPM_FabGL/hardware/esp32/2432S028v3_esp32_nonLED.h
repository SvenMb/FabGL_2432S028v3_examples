#ifndef ESP32_H
#define ESP32_H

// SPI_DRIVER_SELECT must be set to 0 in SdFat/SdFatConfig.h (default is 0)

// Cheap Yellow Display PINS
#define CYD2432S028v3
#define TFT_MISO   12
#define TFT_MOSI   13
#define TFT_SCK    14
#define TFT_CS     15
#define TFT_DC     2
#define TFT_RESET  -1
#define TFT_BL     21
#define TFT_SPIBUS HSPI_HOST
#define XPT2046_IRQ  36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK  25
#define XPT2046_CS   33
// #define Red_LED   4 replaced for PS2
// #define Green_LED 16 replaced for PSRAM
// #define Blue_LED  17 replaced for PSRAM
#define SD_MISO 19
#define SD_MOSI 23
#define SD_SCK  18
#define SD_CS   5
#define LDR     34
#define PS2_DAT GPIO_NUM_27
#define PS2_CLK GPIO_NUM_4
#define AUDIO 26
#define BOOT    0
#define UART_CTS 35
#define UART_RTS 22
#define UART_TX 1
#define UART_RX 3

// define keyboard class/init
#define  KEYB (new fabgl::Keyboard)->begin(PS2_CLK, PS2_DAT, true, true);


// definitions for RunCPM
SdFat SD;
#define SS SD_CS. //Bug fix, you have to define this because later it is used by SD.init. This was missed out on the master branch.
#define SPIINIT SD_SCK,SD_MISO,SD_MOSI,SD_CS // sck, miso, mosi, cs
#define SPIINIT_TXT "18,19,23,5" // 2432S028
#define SDINIT SD_CS, SD_SCK_MHZ(SDMHZ)

// #define ENABLE_DEDICATED_SPI 1

#define SDMHZ 19 // TTGO_T1,LOLIN32_Pro=25 ePaper,ESP32_DevKit=20
#define SDMHZ_TXT "19" // TTGO_T1,LOLIN32_Pro=25 ePaper,ESP32_DevKit=20
#define BOARD "CYD 2432S028v3"
#define board_esp32
#define board_digital_io

uint8 esp32bdos(uint16 dmaaddr) {
	return(0x00);
}

#endif
