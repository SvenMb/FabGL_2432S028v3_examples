// #########################################################################################
// https://lehwalder.wordpress.com/2021/04/07/runcpm-on-the-lilygo-ttgo-vga32-v1-4/
// https://lehwalder.wordpress.com/2021/04/28/getting-runcpm-v5-3-fast-onto-the-ttgo-vga32-v1-4/
// #########################################################################################

// =========================================================================================
#include "globals.h"
#include <SPI.h>
// =========================================================================================

// #define SDFAT_FILE_TYPE 1 // Uncomment for Due and Teensy

// SD library - Greinman SdFat from Library Manager
#include <SdFat.h>           // SD library - Greinman SdFat from Library Manager
// =========================================================================================

/* ATTENTION if you get errors on compile 
SdFat library change
====================
Since the SdFat-Library v2.0.2 you need to edit SdFatConfig.h of the SdFat-Library:
Edit SdFatConfig.h file (around line 78 as of version 2.0.2) changing:
#define SDFAT_FILE_TYPE 3
to

#define SDFAT_FILE_TYPE 1
As file type 1 is required for most of the RunCPM ports.
To find your libraries folder, open the Preferences in Arduino IDE 
and look at the Sketchbook location field.

On Windows systems, SdFatConfig.h will be in Documents\Arduino\libraries\SdFat\src

BTW: This change has to be done also every time you update the SdFat-Library,
because it overwrites this change.

See also that this solved my issue since SdFat-Library v2.0.2:
https://github.com/MockbaTheBorg/RunCPM/issues/143
*/


// =========================================================================================
// Guido Lehwalder's Code-Revision-Number
// =========================================================================================
#define GL_REV "GL20251001.0"


// Board definitions go into the "hardware" folder- Choose/change a file from there

// =========================================================================================
// 2432S028v3 Cheap Yellow Display with ST7789 Hardware Definition File
// =========================================================================================
#include "hardware/esp32/2432S028v3_esp32_nonLED.h"

// =========================================================================================
// TTGO VGA32 Hardware Definition File
// =========================================================================================
// #include "hardware/esp32/ttgo_vga32_esp32_nonLED.h"

// =========================================================================================
// Olimex ESP32-SBC FabGL Hardware Definition File
// =========================================================================================
// #include "hardware/esp32/olimex_esp32_sbc.h"

// =========================================================================================
// Delays for LED blinking (for never used void loop) ;)
// =========================================================================================
#define sDELAY 50
#define DELAY 100

// =========================================================================================
// FabGL Init
// =========================================================================================
// Define this to use the VGA screen/keyboard as a terminal
#define FABGL true;

// =========================================================================================
// Define KeyClick
// =========================================================================================
bool KEYCLICK = true;

// =========================================================================================
// Define this to dont use USB Console as Serial-Mirror
// =========================================================================================
bool SERMIR = false;

// =========================================================================================
// Define this to dont use USB Serial Control
// =========================================================================================
bool SERCTL = false;

// =========================================================================================
// Define this to dont filter USB-Serial-Console
// =========================================================================================
bool SERFLT = false;

// =========================================================================================
// FABGL Init
// =========================================================================================
#ifdef FABGL
#include "fabgl.h"
#include "fabutils.h"

#if defined(CYD2432S028v3)
#include "TFTControllerST7789n.h"
fabgl::ST7789nController  DisplayController;

// =========================================================================================
// Set VGAController in 8 or 16 Color Mode
// VGA8Controller  for ESP32 Core v2.0.0 due to huge memory usage
// VGA16Controller for ESP32 Core v1.0.6 due to less memory usage
// VGA16Controller does work good/better with FabGL-master from 11.10.2021
// =========================================================================================

// #define VGA8  // define if low on memory
// #define VGA16 // no need to define, is else path anyway

// =========================================================================================
#elif defined(VGA8)
bool SETVGA8 = true;
fabgl::VGA8Controller   DisplayController;
// =========================================================================================
#else // VGA16
bool SETVGA8 = false;
fabgl::VGA16Controller  DisplayController;
#endif // defined(CYD2432S028v3)

fabgl::PS2Controller     PS2Controller;
fabgl::Terminal          Terminal;
#include "confdialog.h"
#endif // FABGL
// =========================================================================================

#include "abstraction_arduino.h"


// =========================================================================================
// Set Serial port speed
// =========================================================================================
#define SERIALSPD 115200

// =========================================================================================
// PUN: device configuration
// =========================================================================================
#ifdef USE_PUN
File32 pun_dev;
int pun_open = FALSE;
#endif

// =========================================================================================
// LST: device configuration
// =========================================================================================
#ifdef USE_LST
File32 lst_dev;
int lst_open = FALSE;
#endif

// =========================================================================================
// include RunCPM sources
// =========================================================================================
#include "ram.h"
#include "console.h"
#include "cpu.h"
#include "disk.h"
#include "host.h"
#include "cpm.h"

// =========================================================================================
// Do we use the internal RunCPM CCP?
// =========================================================================================
#ifdef CCP_INTERNAL
#include "ccp.h"
#endif

// =========================================================================================
// void setup - START
// =========================================================================================
void setup(void) {

  // disable the WatchDog-TimeOut of the ESP32 Cores
  disableCore0WDT();
  delay(100); // experienced crashes without this delay!
  disableCore1WDT();  

  // more stack is required for the UI (used inside Terminal.onVirtualKey)
  // default value 2048
  Terminal.keyboardReaderTaskStackSize = 2048;
  // Terminal.keyboardReaderTaskStackSize = 3000;
  
  // 2048 good to pass vttest
  // 1024 is default value
  //  512 is good value to save memory
  // Terminal.inputQueueSize = 2048;
  Terminal.inputQueueSize = 512;

  // 1024 is default value
  //  512 is that a good value to save memory?
  // DisplayController.queueSize = 512;


  preferences.begin("RunCPM", false);

  ConfDialogApp::checkVersion();
  
  Serial.begin(SERIALSPD);
  delay(500);  // avoid garbage into the UART

#ifdef CYD2432S028v3
  (new fabgl::Keyboard)->begin(PS2_CLK, PS2_DAT, true, true);
  DisplayController.begin(TFT_SCK, TFT_MOSI, TFT_DC, TFT_RESET, TFT_CS, TFT_SPIBUS, TFT_BL);
  DisplayController.setResolution(TFT_240x320);
  DisplayController.setOrientation(fabgl::TFTOrientation::Rotate90);
#else // CYD2432S028v3
  PS2Controller.begin(PS2Preset::KeyboardPort0);
  DisplayController.begin(); //default
//  DisplayController.setResolution(VGA_640x240_60Hz);
  DisplayController.setResolution(VGA_640x480_60Hz);  
//  DisplayController.shrinkScreen(0, -48);
//  DisplayController.setResolution(VGA_640x480_73Hz);
//  DisplayController.setResolution(VGA_640x480_60HzAlt1);  
//  DisplayController.setResolution(VGA_640x480_60HzD);
#endif // CYD2432S028v3

  Terminal.begin(&DisplayController);
  Terminal.connectLocally();                  // to use Terminal.read(), available(), etc..
  Terminal.activate();
  Terminal.clear();
  Terminal.enableCursor(true);

  // Set Color 16 (from 0 to 15) to AMBER-Orange for Retro-Display
  // https://rgbcolorcode.com/color/amber
  // RGB(255,191,0)

#if defined(VGA8)
  DisplayController.setPaletteItem(4, RGB888(255, 191, 0));
#elif defined(VG16)
  DisplayController.setPaletteItem(9, RGB888(255, 191, 0));
#endif
  ConfDialogApp::loadConfiguration();

// =========================================================================================
// If DEBUGLOG exists - delete it
// =========================================================================================
#ifdef DEBUGLOG
  _sys_deletefile((uint8 *)LogName);
#endif

// =========================================================================================
// Do the Boot-Sound
// =========================================================================================
     for (int i = 0; i <= 2; i++)   {
         Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 800 , 50);
         Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 1600 , 50);
         delay(100);
                                    }
// =========================================================================================
// readout the Config-Menu screen color and create a string to output Escape-sequences
// =========================================================================================
// get colors for Back- and Foreground
int BGCOL = preferences.getInt("BGColor", (int)Color::Black);
int FGCOL = preferences.getInt("FGColor", (int)Color::BrightGreen);

// set Escape-Sequence Color for Background and Foreground
if (BGCOL < 8) {BGCOL = BGCOL +40;} else {BGCOL = BGCOL +92;}
if (FGCOL < 8) {FGCOL = FGCOL +30;} else {FGCOL = FGCOL +82;}                                    

// convert int values to string
String SBGCOL = String(BGCOL);
String SFGCOL = String(FGCOL);




// =========================================================================================
// set KeyClick to the value of the config-menu
// =========================================================================================
if (ConfDialogApp::getKeyClick() == KEYCLICK_ENABLED){KEYCLICK = true;}
      else {KEYCLICK = false;}

// =========================================================================================
// set USBSerialMirror to the value of the config-menu
// =========================================================================================
if (ConfDialogApp::getSerMir() == SERMIR_ENABLED){SERMIR = true;}
      else {SERMIR = false;}

// =========================================================================================
// set USBSerialControl to the value of the config-menu
// =========================================================================================
if (ConfDialogApp::getSerCtl() == SERCTL_ENABLED){SERCTL = true;}
      else {SERCTL = false;}      
 
// =========================================================================================
// set USBSerialFilter to the value of the config-menu
// =========================================================================================
if (ConfDialogApp::getSerFlt() == SERFLT_ENABLED){SERFLT = true;}
      else {SERFLT = false;}

// =========================================================================================
// Show Boot-Screen
// =========================================================================================
_clrscr();

_puts("\e[0m");
// _puts("\r\n");

// if (SETVGA8 == true)
//                     { _puts(" \e[37m\e#6  RunCPM v" VERSION " VGA32 \e[0m \r\n"); } // RunCPM Double Wide Line -Esc 37m white
//                     else
//                     { _puts(" \e[97m\e#6  RunCPM v" VERSION " VGA32 \e[0m \r\n"); } // RunCPM Double Wide Line -Esc 97m bright-white 
                    
 // _puts("[\e[1m\e#6\e#3 RunCPM v" VERSION "  VGA32 \e[0m]\r\n"); // top    half of RunCPM Double Height Line
 // _puts("[\e[1m\e#6\e#4 RunCPM v" VERSION "  VGA32 \e[0m]\r\n"); // bottom half of RunCPM Double Height Line

// _puts("______________________________________________\r\n");

// =========================================================================================
// Print "Z80" at column 48 of row 2
// Terminal.write("\e_F49;4$");
// Terminal.write("######## \e[1m #######    #####   \e[0m");
// Terminal.write("\e_F49;5$");
// Terminal.write("     ##  \e[1m##     ##  ##   ##  \e[0m");
// Terminal.write("\e_F49;6$");
// Terminal.write("    ##   \e[1m##     ## ##     ## \e[0m");
// Terminal.write("\e_F49;7$");
// Terminal.write("   ##    \e[1m #######  ##     ## \e[0m");
// Terminal.write("\e_F49;8$");
// Terminal.write("  ##     \e[1m##     ## ##     ## \e[0m");
// Terminal.write("\e_F49;9$");
// Terminal.write(" ##      \e[1m##     ##  ##   ##  \e[0m");
// Terminal.write("\e_F49;10$");
// Terminal.write("######## \e[1m #######    #####   \e[0m");

// Terminal.write("\e_F1;3$");
// =========================================================================================

   
_puts("______________________________________________\r\n");
_puts("CP/M  Emulator \e[1mv");
_puts(VERSION);
_puts("\e[0m      by   [\e[1mMarcelo Dantas\e[0m]\r\n");
_puts("using FabGL    Terminal  by   [  \e[1m@fdivitto\e[0m   ]\r\n");
_puts("Revision                      [ \e[1m");
_puts(GL_REV);
_puts("\e[0m ]\r\n");

// =========================================================================================
#ifdef VGA8
// VGA8 Colored Text
              { 
              _puts("\e[37m\e[41m V \e[30m\e[102m G \e[92m\e[104m A \e[30m\e[107m 8 \e[0m-\e[37m\e[44m Controller   \e[0m   [\e[47m \e[101m \e[41m \e[102m \e[42m \e[104m \e[44m \e[101m \e[41m \e[102m \e[42m \e[104m \e[44m \e[107m \e[0m]\r\n");
              }
#else // VGA8
// VGA16 Colored Text
              {
              _puts("\e[96m\e[101m V \e[30m\e[102m G \e[96m\e[104m A \e[30m\e[103m 16 \e[96m\e[45m Controller   \e[0m   [\e[101m \e[41m \e[102m \e[42m \e[104m \e[44m \e[103m \e[43m \e[105m \e[45m \e[106m \e[46m \e[107m \e[47m \e[0m]\r\n");
              }
#endif // VGA8

// =========================================================================================
// If Boot-Info is enabled show Boot-Info
// =========================================================================================
  if (ConfDialogApp::getBootInfo() == BOOTINFO_ENABLED) {
    Terminal.printf("Screen Size                ->  \e[1m%d x %d\e[0m\r\n", DisplayController.getScreenWidth(), DisplayController.getScreenHeight());
    Terminal.printf("Terminal Size              ->   \e[1m%d x %d\e[0m\r\n", Terminal.getColumns(), Terminal.getRows());
    Terminal.printf("Keyboard                   ->   \e[1m%s\e[0m\r\n", PS2Controller.keyboard()->isKeyboardAvailable() ? "OK" : "Error");    
    Terminal.printf("Keyboard Layout            ->   \e[1m%s\e[0m\r\n", PS2Controller.keyboard()->isKeyboardAvailable() ? SupportedLayouts::names()[ConfDialogApp::getKbdLayoutIndex()] : "No Keyboard");
    Terminal.printf("Terminal Type              ->   \e[1m%s\e[0m\r\n", SupportedTerminals::names()[(int)ConfDialogApp::getTermType()]);
    Terminal.printf("Free DMA Memory            ->   \e[1m%d bytes\e[0m\r\n", heap_caps_get_free_size(MALLOC_CAP_DMA));
    Terminal.printf("Free Memory                ->   \e[1m%d bytes\e[0m\r\n", heap_caps_get_free_size(MALLOC_CAP_32BIT));
    Terminal.printf("Terminal-Version           ->   \e[1m%d.%d\e[0m\r\n", TERMVERSION_MAJ, TERMVERSION_MIN);
    // Terminal.write("____________________________________________\r\n");
    // Terminal.write("Press BREAK  [Ctrl+PAUSE]    to reboot\r\n");
    // Terminal.write("Press F12 to change terminal configuration\r\n");
    //    Terminal.write("    and CTRL-ALT-F12 to reset settings\r\n");

  } else if (ConfDialogApp::getBootInfo() == BOOTINFO_TEMPDISABLED) {
    preferences.putInt("BootInfo", BOOTINFO_ENABLED);
  }      

  if (ConfDialogApp::getBootInfo() != BOOTINFO_ENABLED) {
                                                        // _puts("\r\n");
                                                           _puts("______________________________________________\r\n");
                                                        }
#ifndef CYD2432S028v3
// =========================================================================================
// # KeyClick-Output
// =========================================================================================
   _puts("SND   : KeyClick       [F5]   ");
if (KEYCLICK == true)
   {_puts("[    enabled   ]\r\n");}
      else
      {_puts("[   \e[91mdisabled\e[0m   ]\r\n");}

// =========================================================================================
// # USBSerialMirror-Output
// =========================================================================================
   _puts("USB   : SerialMirror   [F6]   ");
   if (SERMIR == true)
   {_puts("[    enabled   ]\r\n");}
      else
      {_puts("[   \e[91mdisabled\e[0m   ]\r\n");}

// =========================================================================================
// # USBSerialControl-Output
// =========================================================================================
   _puts("USB   : SerialControl  [F7]   ");
   if (SERCTL == true)
   {_puts("[    enabled   ]\r\n");}
      else
      {_puts("[   \e[91mdisabled\e[0m   ]\r\n");}

// =========================================================================================
// # USBSerialFilter-Output
// =========================================================================================
   _puts("USB   : SerialFilter   [F8]   ");
   if (SERFLT == true)
   {_puts("[    enabled   ]\r\n");}
      else
      {_puts("[   \e[91mdisabled\e[0m   ]\r\n");}

// =========================================================================================
#endif
//  _puts("\r\n");

//  _puts("BIOS  :                       [   \e[1m0x");
//  _puthex16(BIOSjmppage);
//  _puts(" - ");
//  _puts("\e[0m     ]\r\n");

#ifdef ABDOS
	_puts("ABDOS.SYS \e[1menabled\e[0m at [\e[1m0x");
	_puthex16(BDOSjmppage);
	_puts("\e[0m]\r\n");
  #else
	_puts("BDOS  :         at [\e[1m0x");
	_puthex16(BDOSjmppage);
	_puts("\e[0m]\r\n");
  #endif

  _puts("CCP   :         at [\e[1m0x");
  _puthex16(CCPaddr);
        _puts("\e[0m]   [ \e[1m");
        _puts(CCPname);
  _puts("\e[0m]\r\n");

// =========================================================================================
//  _puts("\r\n");  


//  _puts("DEBUG : ESP32 CPU Clock       [   \e[1m240Mhz \e[0m    ]\r\n");
//  _puts("DEBUG : ESP32 Core            [   \e[1mv2.0.17\e[0m    ]\r\n");
//  _puts("DEBUG : SdFat Library         [   \e[1mv2.3.1 \e[0m    ]\r\n");   

//  _puts("\r\n");  

// =========================================================================================

  #if BANKS > 1
  _puts("Banked      RAM               [\e[1m");
  _puthex8(BANKS);
    _puts("\e[0m]banks\r\n");
  #else
  _puts("Banked      RAM               [\e[1m");
  _puthex8(BANKS);
  _puts("\e[0m]bank\r\n");
  #endif

// =========================================================================================

// Allocate MEMSIZE-Patch of Ian Schofield / https://github.com/Isysxp )
// Allocate MEMSIZE bytes of memory for RAM
// see https://github.com/MockbaTheBorg/RunCPM/issues/181
// see also globals.h

RAM = (uint8*)malloc((size_t)MEMSIZE);
if (!RAM) {
  _puts("PS-RAM in FlashModer not enabled. MemAlloc has failed.");
   while (1);
 }

// =========================================================================================
   Terminal.printf("Free    PS  RAM Ian-Mem-Alloc [\e[1m%d Bytes\e[0m]\r\n", heap_caps_get_free_size(MALLOC_CAP_32BIT));
   Terminal.printf("Free    DMA RAM               [\e[1m%d Bytes\e[0m]\r\n", heap_caps_get_free_size(MALLOC_CAP_DMA));

   if (SERMIR == true)
    {
     Serial.printf("Free    PS  RAM Ian-Mem-Alloc [\e[1m%d Bytes\e[0m]\r\n", heap_caps_get_free_size(MALLOC_CAP_32BIT));
     Serial.printf("Free    DMA RAM               [\e[1m%d Bytes\e[0m]\r\n", heap_caps_get_free_size(MALLOC_CAP_DMA));
    }


// =========================================================================================
// Press F12 for Config-Menue & BREAK - START
// or press F2 for Config-Menue for Vestel VES-541 Mini keyboard without F12-key
// =========================================================================================

  // onVirtualKey is triggered whenever a key is pressed or released
  Terminal.onVirtualKeyItem = [&](VirtualKeyItem * vkItem) 
  {
    if (vkItem->vk == VirtualKey::VK_F2 || vkItem->vk == VirtualKey::VK_F12) {
// =========================================================================================
// -----------------------------------------------------------------------------------------      
      if (vkItem->CTRL && (vkItem->LALT || vkItem->RALT)) {
        Terminal.deactivate();
        preferences.clear();
        delay(1000);
        Terminal.write("\r\n[REBOOT]");
        delay(1000);
        ESP.restart();
                                                          } 
// -----------------------------------------------------------------------------------------                                                          
        else if (!vkItem->CTRL && !vkItem->LALT && !vkItem->RALT && !vkItem->down) {        
        // releasing F12 key to open configuration dialog
        Terminal.deactivate();
        auto dlgApp = new ConfDialogApp;
        dlgApp->run(&DisplayController);
        delete dlgApp;
        Terminal.keyboard()->emptyVirtualKeyQueue();
        Terminal.activate();

                                                                                    }
        vkItem->vk = VirtualKey::VK_NONE;                                                                                    
// -----------------------------------------------------------------------------------------
// =========================================================================================                                                                                    
                                          }

// =========================================================================================
// Press BREAK for reboot - or F9 on the Vestel VES-541 Mini Keyboard without BREAK-Key
// =========================================================================================
    if (vkItem->vk == VirtualKey::VK_F9 || vkItem->vk == VirtualKey::VK_BREAK) {
      if (!vkItem->down) {
                          // releasing BREAK key to reboot
        
                          // _reboot this is only available in the "internal" CCP :(
                          _ccp_reboot();
        
                          // for any other CCP (like DR or Z80) we have to use the 
                          // "fast" reboot :)
                          // ESP.restart();
                         } 
                                            }

// =========================================================================================
// F5 Toggle KeyClick
// =========================================================================================
    if (vkItem->vk == VirtualKey::VK_F5) {
      if (!vkItem->CTRL && !vkItem->LALT && !vkItem->RALT && !vkItem->down) {
          if (KEYCLICK == true)
             {KEYCLICK = false; 
             // Terminal.write("\r\nDEBUG: KEYCLICK disabled\r\n");
             Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 800 , 50);
             Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 1600 , 50);
             }
             else
             {KEYCLICK = true; 
             // Terminal.write("\r\nDEBUG: KEYCLICK enabled\r\n");
             Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 800 , 50);
             Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 1600 , 50);
             }
                                                                            }
             // always convert to NONE, on both keydown an dup
             vkItem->vk = VirtualKey::VK_NONE;
                                         }

// =========================================================================================
// F6 Toggle SERMIR
// =========================================================================================
    if (vkItem->vk == VirtualKey::VK_F6) {
      if (!vkItem->CTRL && !vkItem->LALT && !vkItem->RALT && !vkItem->down) {
          if (SERMIR == true)
             {SERMIR = false; 
             // Terminal.write("\r\nDEBUG: SERMIR disabled\r\n");
             Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 800 , 50);
             Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 1600 , 50);
             }
             else
             {SERMIR = true; 
             // Terminal.write("\r\nDEBUG: SERMIR enabled\r\n");
             Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 800 , 50);
             Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 1600 , 50);
             }
                                                                            }
             // always convert to NONE, on both keydown an dup
             vkItem->vk = VirtualKey::VK_NONE;
                                         }

// =========================================================================================
// F7 Toggle SERCTL
// =========================================================================================
    if (vkItem->vk == VirtualKey::VK_F7) {
      if (!vkItem->CTRL && !vkItem->LALT && !vkItem->RALT && !vkItem->down) {
          if (SERCTL == true)
             {SERCTL = false; 
             // Terminal.write("\r\nDEBUG: SERCTL disabled\r\n");
             Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 800 , 50);
             Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 1600 , 50);
             }
             else
             {SERCTL = true; 
             // Terminal.write("\r\nDEBUG: SERCTL enabled\r\n");
             Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 800 , 50);
             Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 1600 , 50);
             }
                                                                            }
             // always convert to NONE, on both keydown an dup
             vkItem->vk = VirtualKey::VK_NONE;
                                         }

// =========================================================================================
// F8 Toggle SERFLT
// =========================================================================================
    if (vkItem->vk == VirtualKey::VK_F8) {
      if (!vkItem->CTRL && !vkItem->LALT && !vkItem->RALT && !vkItem->down) {
          if (SERFLT == true)
             {SERFLT = false; 
             // Terminal.write("\r\nDEBUG: SERFLT disabled\r\n");
             Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 800 , 50);
             Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 1600 , 50);
             }
             else
             {SERFLT = true; 
             // Terminal.write("\r\nDEBUG: SERFLT enabled\r\n");
             Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 800 , 50);
             Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), 1600 , 50);
             }
                                                                            }
             // always convert to NONE, on both keydown an dup
             vkItem->vk = VirtualKey::VK_NONE;
                                         }

// =========================================================================================
// KeyClick-Sound
// =========================================================================================

        Terminal.onVirtualKey = [&](VirtualKey * vk, bool keyDown) 
          { if (keyDown && KEYCLICK == true)  
                Terminal.soundGenerator()->playSound(SquareWaveformGenerator(), (*vk == VirtualKey::VK_RETURN ? 500 : 1000), 4);;
          };


// =========================================================================================
// Press F12 for Config-Menue & BREAK & KeyClick - END
// =========================================================================================
  };


// =========================================================================================
// Activate SPI & SDCard and load the ex- or internal CCP
// =========================================================================================
#if defined board_esp32
//  _puts("INIT  : SPI-Bus [");
//  _puts(SPIINIT_TXT);
//  _puts("]  ");
    SPI.begin(SPIINIT);
//  _puts("  ->  [ Done ]\r\n");
#endif

// ----------------------------------------------------------------------------------------- 
if (SD.begin(SDINIT)) {
//   _puts("        MicroSD Card at ");
//   _puts(SDMHZ_TXT);
//   _puts("Mhz ");
//   _puts("  ->  [ Done ]\r\n");
   _puts("______________________________________________\r\n");
   
    if (VersionCCP >= 0x10 || SD.exists(CCPname)) {
#ifdef ABDOS
      _PatchBIOS();
#endif
      while (true) {
        _puts(CCPHEAD);      
        _PatchCPM();
  Status = STATUS_RUNNING;

// ----------------------------------------------------------------------------------------- 

#ifdef CCP_INTERNAL
        _ccp();
#else
        if (!_RamLoad((uint8 *)CCPname, CCPaddr ,0)) {
          _puts("Unable to load the CCP.\r\nCPU halted.\r\n");
          break;
        }
     		// Loads an autoexec file if it exists and this is the first boot
		    // The file contents are loaded at ccpAddr+8 up to 126 bytes then the size loaded is stored at ccpAddr+7
		    if (firstBoot) {
			    if (_sys_exists((uint8*)AUTOEXEC)) {
				    uint16 cmd = CCPaddr + 8;
				    uint8 bytesread = (uint8)_RamLoad((uint8*)AUTOEXEC, cmd, 125);
				    uint8 blen = 0;
				    while (blen < bytesread && _RamRead(cmd + blen) > 31)
				    	blen++;
				    _RamWrite(cmd + blen, 0x00);
				    _RamWrite(--cmd, blen);
			    }
			    if (BOOTONLY)
				    firstBoot = FALSE;
		    }
        Z80reset();
        SET_LOW_REGISTER(BC, _RamRead(DSKByte));
        PC = CCPaddr;
        Z80run();
#endif
        if (Status == STATUS_EXIT)
#ifdef DEBUG
	#ifdef DEBUGONHALT
    			Debug = 1;
		    	Z80debug();
	#endif
#endif
          break;

// ----------------------------------------------------------------------------------------- 
#ifdef USE_PUN
        if (pun_dev)
          _sys_fflush(pun_dev);
#endif

#ifdef USE_LST
        if (lst_dev)
          _sys_fflush(lst_dev);
#endif

// ----------------------------------------------------------------------------------------- 
      }
    } else {
      _puts("Unable to load CP/M CCP.\r\nCPU halted.\r\n");
    }
  } else {
    _puts("Unable to initialize SD card.\r\nCPU halted.\r\n");
  }
                                                  
// =========================================================================================
// void setup - ENDE
// =========================================================================================

}

// =========================================================================================
// void loop - START - would be never reached because CCP take over the control above
// =========================================================================================

void loop(void) {

                delay(DELAY * 4);
}

// =========================================================================================
// void loop - END of communication
// =========================================================================================
