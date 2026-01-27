/*
  Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com) - <http://www.fabgl.com>
  Copyright (c) 2019-2022 Fabrizio Di Vittorio.
  All rights reserved.


* Please contact fdivitto2013@gmail.com if you need a commercial license.


* This library and related software is available under GPL v3.

  FabGL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  FabGL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FabGL.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "fabgl.h"
#include "TFTControllerST7789n.h"

fabgl::ST7789nController DisplayController;
fabgl::Terminal          Terminal;
fabgl::PS2Controller     PS2Controller;

// Cheap Yellow Display PINS
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
// #define UART_CTS 35
// #define UART_RTS 22
// #define UART_TX 1
// #define UART_RX 3

void xprintf(const char * format, ...)
{
  va_list ap;
  va_start(ap, format);
  int size = vsnprintf(nullptr, 0, format, ap) + 1;
  if (size > 0) {
    va_end(ap);
    va_start(ap, format);
    char buf[size + 1];
    vsnprintf(buf, size, format, ap);
    Serial.write(buf);
    Terminal.write(buf);
  }
  va_end(ap);
}


void printHelp()
{
  xprintf("\e[93m\n\nPS/2 Keyboard Scancodes\r\n");
  xprintf("Chip Revision: %d   Chip Frequency: %d MHz\r\n", ESP.getChipRevision(), ESP.getCpuFreqMHz());

  printInfo();

  xprintf("Commands:\r\n");
  xprintf("  q = Scancode set 1  w = Scancode set 2\r\n");
  xprintf("  l = Test LEDs       r = Reset keyboard\r\n");
  xprintf("Various:\r\n");
  xprintf("  h = Print This help\r\n\n");
  xprintf("Use Serial Monitor to issue commands\r\n\n");
}


void printInfo()
{
  auto keyboard = PS2Controller.keyboard();

  if (keyboard->isKeyboardAvailable()) {
    xprintf("Device Id = ");
    switch (keyboard->identify()) {
      case PS2DeviceType::OldATKeyboard:
        xprintf("\"Old AT Keyboard\"");
        break;
      case PS2DeviceType::MouseStandard:
        xprintf("\"Standard Mouse\"");
        break;
      case PS2DeviceType::MouseWithScrollWheel:
        xprintf("\"Mouse with scroll wheel\"");
        break;
      case PS2DeviceType::Mouse5Buttons:
        xprintf("\"5 Buttons Mouse\"");
        break;
      case PS2DeviceType::MF2KeyboardWithTranslation:
        xprintf("\"MF2 Keyboard with translation\"");
        break;
      case PS2DeviceType::M2Keyboard:
        xprintf("\"MF2 keyboard\"");
        break;
      default:
        xprintf("\"Unknown\"");
        break;
    }
    xprintf("\r\n", keyboard->getLayout()->name);
  } else
    xprintf("Keyboard Error!\r\n");
}



void setup()
{
  Serial.begin(115200);
  delay(500);  // avoid garbage into the UART
  Serial.write("\r\n\nReset\r\n");

  DisplayController.begin(TFT_SCK, TFT_MOSI, TFT_DC, TFT_RESET, TFT_CS, TFT_SPIBUS, TFT_BL);
  DisplayController.setResolution(TFT_240x320);
  DisplayController.setOrientation(fabgl::TFTOrientation::Rotate90);

  Terminal.begin(&DisplayController);
  Terminal.loadFont(&fabgl::FONT_6x10);
  Terminal.enableCursor(true);

  // PS2Controller.begin(PS2Preset::KeyboardPort0);
  (new fabgl::Keyboard)->begin(PS2_CLK, PS2_DAT, false, false);
  printHelp();
}




void loop()
{
  static int clen = 1;
  auto keyboard = PS2Controller.keyboard();

  if (Serial.available() > 0) {
    char c = Serial.read();
    switch (c) {
      case 'h':
        printHelp();
        break;
      case 'r':
        keyboard->reset();
        printInfo();
        break;
      case 'l':
        xprintf("Teste LEDs.\r\n");
        for (int i = 0; i < 8; ++i) {
          keyboard->setLEDs(i & 1, i & 2, i & 4);
          delay(1000);
        }
        delay(2000);
        if (keyboard->setLEDs(0, 0, 0))
          xprintf("OK\r\n");
        break;
      case 'q':
        keyboard->setScancodeSet(1);
        xprintf("Scancode Set = %d\r\n", keyboard->scancodeSet());
        break;
      case 'w':
        keyboard->setScancodeSet(2);
        xprintf("Scancode Set = %d\r\n", keyboard->scancodeSet());
        break;
    }
  }

  if (keyboard->scancodeAvailable()) {
    int scode = keyboard->getNextScancode();
    xprintf("%02X ", scode);
    if (scode == 0xF0 || scode == 0xE0) ++clen;
    --clen;
    if (clen == 0) {
      clen = 1;
      xprintf("\r\n");
    }
    switch (scode) {
      case 0x33:
        printHelp();
        break;
      case 0x2d:
        keyboard->reset();
        printInfo();
        break;
      case 0x4b:
        xprintf("Teste LEDs.\r\n");
        for (int i = 0; i < 8; ++i) {
          keyboard->setLEDs(i & 1, i & 2, i & 4);
          delay(1000);
        }
        delay(2000);
        if (keyboard->setLEDs(0, 0, 0))
          xprintf("OK\r\n");
        break;
      case 0x15:
        keyboard->setScancodeSet(1);
        xprintf("Scancode Set = %d\r\n", keyboard->scancodeSet());
        break;
      case 0x1d:
        keyboard->setScancodeSet(2);
        xprintf("Scancode Set = %d\r\n", keyboard->scancodeSet());
        break;
    }
  }

}
