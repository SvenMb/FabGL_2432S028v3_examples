
#include "fabgl.h"
#include "TFTControllerST7789n.h"


fabgl::ST7789nController DisplayController;
// fabgl::ILI9341Controller DisplayController;
fabgl::PS2Controller                PS2Controller;
fabgl::Terminal                     Terminal;
fabgl::SerialPort                   SerialPort;

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
// #define BOOT    0
// #define UART_CTS 35
// #define UART_RTS 22
// #define UART_TX 1
// #define UART_RX 3

#define BAUDRATE  115200
#define SLOWPRINTDELAY 4

void slowPrintf(const char * format, ...);
void slowPrintLine(const char c);

void setup()
{
  unsigned int cols,rows,width,height;
  Serial.begin(BAUDRATE); Serial.write("\n\n\n");

  // PS2Controller.begin(PS2Preset::KeyboardPort0);
  (new fabgl::Keyboard)->begin(PS2_CLK, PS2_DAT, true, true);

  DisplayController.begin(TFT_SCK, TFT_MOSI, TFT_DC, TFT_RESET, TFT_CS, TFT_SPIBUS, TFT_BL);
  DisplayController.setResolution(TFT_240x320);
  DisplayController.setOrientation(fabgl::TFTOrientation::Rotate90);

  Terminal.begin(&DisplayController);
  Terminal.loadFont(&fabgl::FONT_5x8);
  Terminal.write("\e[40;33m"); // background: black, foreground: Yellow
  Terminal.write("\e[2J");     // clear screen
  Terminal.write("\e[11;20H"); // move cursor to middle of screen
  slowPrintf("\e[1m<< STARTUP >>");
  delay(2500);                 // REQUIRED to settle FabGL display controller

  Terminal.write("\e[40;32m"); // background: black, foreground: green
  Terminal.write("\e[2J");     // clear screen
  Terminal.write("\e[1;1H");   // move cursor to 1,1

  cols = Terminal.getColumns();
  rows = Terminal.getRows();
  height = DisplayController.getScreenWidth(); // rotated screen
  width = DisplayController.getScreenHeight(); // rotated screen
  slowPrintf("* * * \e[31mA\e[32mN\e[33mS\e[34mI\e[32m-VT100 serial terminal * * *\r\n");
  slowPrintf("Serial speed  : %d 8 N1\r\n", BAUDRATE);
  slowPrintf("Screen size   : %d x %d\r\n", width,height);
  slowPrintf("Terminal size : %d x %d\r\n", cols,rows);
  slowPrintLine('=');
  Terminal.write("\r\n");
  Terminal.enableCursor(true);
  Terminal.connectLocally(); 

  pinMode(0, INPUT);
}

void slowPrintf(const char * format, ...)
{
  va_list ap;
  va_start(ap, format);
  int size = vsnprintf(nullptr, 0, format, ap) + 1;
  if (size > 0) {
    va_end(ap);
    va_start(ap, format);
    char buf[size + 1];
    vsnprintf(buf, size, format, ap);
    for (int i = 0; i < size; ++i) {
      Terminal.write(buf[i]);
      delay(SLOWPRINTDELAY);
    }
  }
  va_end(ap);
}

void slowPrintLine(const char c) {
  unsigned int maxcols = Terminal.getColumns();
  for(int i = 1; i < maxcols; i++) {
    Terminal.write(c);
    delay(SLOWPRINTDELAY);
  }
  Terminal.write("\r\n");
}

const char *colorCycle[] = {
  "\e[32m", // 0 GREEN
  "\e[37m", // 1 WHITE
  "\e[34m", // 2 BLUE
  "\e[33m", // 3 YELLOW
  "\e[31m", // 4 RED
  "\e[35m", // 5 MAGENTA
  "\e[36m", // 6 CYAN
};
unsigned int colorindex; // starts on GREEN

void loop()
{
  if(Serial.available() > 0) {
    Terminal.write(Serial.read());
  }
  if (Terminal.available()) {
    char c = Terminal.read();
    switch (c) {
     case 0x7F:       // DEL -> backspace + ESC[K
       Serial.write("\b\e[K");
       break;
     case 0x0D:       // CR  -> CR + LF
       Serial.write("\r\n");
       break;
    default:
       Serial.write(c);
       break;
    }
  }
  if(digitalRead(0) == LOW) {
    if(++colorindex > (sizeof(colorCycle)/sizeof(colorCycle[0]))-1) colorindex = 0;
    Terminal.write(colorCycle[colorindex]);
    while(digitalRead(0) == LOW);
  }
}
