 /*
  Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com) - www.fabgl.com
  Copyright (c) 2019-2020 Fabrizio Di Vittorio.
  All rights reserved.

  This file is part of FabGL Library.

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


#pragma once

#include <Preferences.h>

#include "fabui.h"
#include "uistyle.h"
#include "restartdialog.h"

Preferences preferences;


#define TERMVERSION_MAJ 1
#define TERMVERSION_MIN 4

static const char * FONTS_STR[]            = { "Auto", "VGA 4x6", "VGA 5x7", "VGA 5x8", "VGA 6x8", "VGA 6x9", "VGA 6x10", "VGA 6x12", "VGA 6x13",
                                                "VGA 7x13", "VGA 7x14", "VGA 8x8", "VGA 8x9", "VGA 8x13", "VGA 8x14", "VGA 8x16", "VGA 8x19", "VGA 9x15",
                                                "VGA 9x18", "VGA 10x20", "BigSerif 8x14", "BigSerif 8x16", "Block 8x14", "Broadway 8x14",
                                                "Computer 8x14", "Courier 8x14", "LCD 8x14", "Old English 8x16", "Sans Serif 8x14", "Sans Serif 8x16",
                                                "Slant 8x14", "Wiggly 8x16" };
static const fabgl::FontInfo * FONTS_INFO[] = { nullptr, &fabgl::FONT_4x6, &fabgl::FONT_5x7, &fabgl::FONT_5x8, &fabgl::FONT_6x8, &fabgl::FONT_6x9,
                                               &fabgl::FONT_6x10, &fabgl::FONT_6x12, &fabgl::FONT_6x13, &fabgl::FONT_7x13, &fabgl::FONT_7x14, &fabgl::FONT_8x8,
                                               &fabgl::FONT_8x9, &fabgl::FONT_8x13, &fabgl::FONT_8x14, &fabgl::FONT_8x16, &fabgl::FONT_8x19, &fabgl::FONT_9x15,
                                               &fabgl::FONT_9x18, &fabgl::FONT_10x20, &fabgl::FONT_BIGSERIF_8x14, &fabgl::FONT_BIGSERIF_8x16, &fabgl::FONT_BLOCK_8x14,
                                               &fabgl::FONT_BROADWAY_8x14, &fabgl::FONT_COMPUTER_8x14, &fabgl::FONT_COURIER_8x14,
                                               &fabgl::FONT_LCD_8x14, &fabgl::FONT_OLDENGL_8x16, &fabgl::FONT_SANSERIF_8x14, &fabgl::FONT_SANSERIF_8x16,
                                               &fabgl::FONT_SLANT_8x14, &fabgl::FONT_WIGGLY_8x16 };
constexpr int       FONTS_COUNT             = sizeof(FONTS_STR) / sizeof(char const *);

constexpr int       BOOTINFO_DISABLED     = 0;
constexpr int       BOOTINFO_ENABLED      = 1;
constexpr int       BOOTINFO_TEMPDISABLED = 2;

constexpr int       KEYCLICK_DISABLED     = 0;
constexpr int       KEYCLICK_ENABLED      = 1;

constexpr int       SERCTL_DISABLED     = 0;
constexpr int       SERCTL_ENABLED      = 1;

constexpr int       SERMIR_DISABLED     = 0;
constexpr int       SERMIR_ENABLED      = 1;

constexpr int       SERFLT_DISABLED     = 0;
constexpr int       SERFLT_ENABLED      = 1;

struct ConfDialogApp : public uiApp {

  Rect              frameRect;
  int               progToInstall;

  uiFrame *         frame;
  uiComboBox *      termComboBox;
  uiComboBox *      kbdComboBox;
  uiColorComboBox * bgColorComboBox;
  uiColorComboBox * fgColorComboBox;
  uiColorComboBox * bdColorComboBox;
  uiComboBox *      fontComboBox;
  uiCheckBox *      infoCheckBox;
  uiCheckBox *      clickCheckBox;
  uiCheckBox *      serctlCheckBox;
  uiCheckBox *      sermirCheckBox;
  uiCheckBox *      serfltCheckBox;

  void init() {

    setStyle(&dialogStyle);

    rootWindow()->frameProps().fillBackground = false;

#ifdef CYD2432S028v3
    frame = new uiFrame(rootWindow(), "", UIWINDOW_PARENTCENTER, Size(320, 240), true, STYLE_FRAME);
    frame->windowStyle().borderSize     = 0;
    int y = 4;
#else
    frame = new uiFrame(rootWindow(), "Terminal Configuration", UIWINDOW_PARENTCENTER, Size(380, 275), true, STYLE_FRAME);
    int y = 24;
#endif

    frameRect = frame->rect(fabgl::uiOrigin::Screen);

    frame->frameProps().resizeable        = false;
    frame->frameProps().moveable          = false;
    frame->frameProps().hasCloseButton    = false;
    frame->frameProps().hasMaximizeButton = false;
    frame->frameProps().hasMinimizeButton = false;

    // ESC : exit without save
    // F10 : save and exit
    frame->onKeyUp = [&](uiKeyEventInfo key) {
      if (key.VK == VirtualKey::VK_ESCAPE)
        quit(0);
      if (key.VK == VirtualKey::VK_F10) {
        saveProps();
        quit(0);
      }
    };

    // little help
    new uiLabel(frame, "RunCPM for VGA32 by Guido Lehwalder & coopzone-dc", Point(28, y), Size(0, 0), true, STYLE_LABELHELP);
    new uiLabel(frame, "Press TAB key to move between fields", Point(68, y +18), Size(0, 0), true, STYLE_LABELHELP);
    new uiLabel(frame, "Outside this dialog press CTRL-ALT-F12 to reset settings", Point(28, y + 30), Size(0, 0), true, STYLE_LABELHELP);


    y += 50;

    // select terminal emulation combobox
    new uiLabel(frame, "Terminal Type", Point(10,  y), Size(0, 0), true, STYLE_LABEL);
    termComboBox = new uiComboBox(frame, Point(10, y + 12), Size(75, 20), 80, true, STYLE_COMBOBOX);
    termComboBox->items().append(SupportedTerminals::names(), SupportedTerminals::count());
    termComboBox->selectItem((int)getTermType());

    // select keyboard layout
    new uiLabel(frame, "Keyboard", Point(100, y), Size(0, 0), true, STYLE_LABEL);
    kbdComboBox = new uiComboBox(frame, Point(100, y + 12), Size(70, 20), 70, true, STYLE_COMBOBOX);
    kbdComboBox->items().append(SupportedLayouts::names(), SupportedLayouts::count());
    kbdComboBox->selectItem(getKbdLayoutIndex());

    // background color
    new uiLabel(frame, "bg Color", Point(185,  y), Size(0, 0), true, STYLE_LABEL);
    bgColorComboBox = new uiColorComboBox(frame, Point(185, y + 12), Size(55, 20), 70, true, STYLE_COMBOBOX);
    bgColorComboBox->selectColor(getBGColor());

    // foreground color
    new uiLabel(frame, "fg Color", Point(255,  y), Size(0, 0), true, STYLE_LABEL);
    fgColorComboBox = new uiColorComboBox(frame, Point(255, y + 12), Size(55, 20), 70, true, STYLE_COMBOBOX);
    fgColorComboBox->selectColor(getFGColor());

    y += 44;

    // font
    new uiLabel(frame, "Font", Point(10,  y), Size(0, 0), true, STYLE_LABEL);
    fontComboBox = new uiComboBox(frame, Point(10, y + 12), Size(75, 20), 70, true, STYLE_COMBOBOX);
    fontComboBox->items().append(FONTS_STR, FONTS_COUNT);
    fontComboBox->selectItem(getFontIndex());

    // bold attribute color
    new uiLabel(frame, "Bold Color", Point(255,  y), Size(0, 0), true, STYLE_LABEL);
    bdColorComboBox = new uiColorComboBox(frame, Point(255, y + 12), Size(55, 20), 70, true, STYLE_COMBOBOX);
    bdColorComboBox->selectColor(getBDColor());

    y += 44;

    // show keyclick select
    new uiLabel(frame, "KeyClick", Point(10, y), Size(0, 0), true, STYLE_LABEL);
    clickCheckBox = new uiCheckBox(frame, Point(80, y - 2), Size(16, 16), uiCheckBoxKind::CheckBox, true, STYLE_CHECKBOX);
    clickCheckBox->setChecked(getKeyClick() == KEYCLICK_ENABLED);


    y += 24;

    // set control to usb-serial 115.200
    new uiLabel(frame, "USBSerControl", Point(10, y), Size(0, 0), true, STYLE_LABEL);
    serctlCheckBox = new uiCheckBox(frame, Point(80, y - 2), Size(16, 16), uiCheckBoxKind::CheckBox, true, STYLE_CHECKBOX);
    serctlCheckBox->setChecked(getSerCtl() == SERCTL_ENABLED);

    // set mirroring to usb-serial
    new uiLabel(frame, "USBSerMirror", Point(110, y), Size(0, 0), true, STYLE_LABEL);
    sermirCheckBox = new uiCheckBox(frame, Point(180, y - 2), Size(16, 16), uiCheckBoxKind::CheckBox, true, STYLE_CHECKBOX);
    sermirCheckBox->setChecked(getSerMir() == SERMIR_ENABLED);

    y += 24;

    // set filter for usb-serial
    new uiLabel(frame, "USBSerFilter", Point(10, y), Size(0, 0), true, STYLE_LABEL);
    serfltCheckBox = new uiCheckBox(frame, Point(80, y - 2), Size(16, 16), uiCheckBoxKind::CheckBox, true, STYLE_CHECKBOX);
    serfltCheckBox->setChecked(getSerFlt() == SERFLT_ENABLED);


    // show boot info
    new uiLabel(frame, "BootInfo", Point(110, y), Size(0, 0), true, STYLE_LABEL);
    infoCheckBox = new uiCheckBox(frame, Point(180, y - 2), Size(16, 16), uiCheckBoxKind::CheckBox, true, STYLE_CHECKBOX);
    infoCheckBox->setChecked(getBootInfo() == BOOTINFO_ENABLED);

    y += 24;


    // exit without save button
    auto exitNoSaveButton = new uiButton(frame, "Quit [ESC]", Point(10, y), Size(90, 20), uiButtonKind::Button, true, STYLE_BUTTON);
    exitNoSaveButton->onClick = [&]() {
      quit(0);
    };

    // exit with save button
    auto exitSaveButton = new uiButton(frame, "Save & Quit [F10]", Point(110, y), Size(90, 20), uiButtonKind::Button, true, STYLE_BUTTON);
    exitSaveButton->onClick = [&]() {
      saveProps();
      quit(0);
    };



    setActiveWindow(frame);
    setFocusedWindow(exitNoSaveButton);

  }


  void saveProps() {

    // need reboot?
    bool reboot =    infoCheckBox->checked()  != getBootInfo() ||
                    clickCheckBox->checked()  != getKeyClick() ||
                    serctlCheckBox->checked() != getSerCtl()   ||
                    serfltCheckBox->checked() != getSerFlt()   ||                    
                    sermirCheckBox->checked() != getSerMir()   ||
                    fontComboBox->selectedItem() != getFontIndex();
                  
    preferences.putInt("TermType", termComboBox->selectedItem());
    preferences.putInt("KbdLayout", kbdComboBox->selectedItem());
    preferences.putInt("BGColor", (int)bgColorComboBox->selectedColor());
    preferences.putInt("FGColor", (int)fgColorComboBox->selectedColor());
    preferences.putInt("BDColor", (int)bdColorComboBox->selectedColor());
    preferences.putInt("BootInfo", infoCheckBox->checked() ? BOOTINFO_ENABLED : BOOTINFO_DISABLED);
    preferences.putInt("KeyClick", clickCheckBox->checked() ? KEYCLICK_ENABLED : KEYCLICK_DISABLED);
    preferences.putInt("SerCtl", serctlCheckBox->checked() ? SERCTL_ENABLED : SERCTL_DISABLED);
    preferences.putInt("SerMir", sermirCheckBox->checked() ? SERMIR_ENABLED : SERMIR_DISABLED);
    preferences.putInt("SerFlt", serfltCheckBox->checked() ? SERFLT_ENABLED : SERFLT_DISABLED);
    preferences.putInt("FONT", fontComboBox->selectedItem());

    if (reboot) {
      auto rebootDialog = new RebootDialog(frame);
      showModalWindow(rebootDialog);  // no return!
    }

    loadConfiguration();
  }


  ~ConfDialogApp() {
    // this is required, becasue the terminal may not cover the entire screen
    canvas()->reset();
    canvas()->setBrushColor(getBGColor());
    canvas()->fillRectangle(frameRect);
  }


  static TermType getTermType() {
    return (TermType) preferences.getInt("TermType", 7);    // default 7 = ANSILegacy
  }

  static int getKbdLayoutIndex() {
    return preferences.getInt("KbdLayout", 3);              // default 3 = "US"
  }

  static Color getBGColor() {
    return (Color) preferences.getInt("BGColor", (int)Color::Black);
  }

  static Color getFGColor() {
    return (Color) preferences.getInt("FGColor", (int)Color::BrightGreen);
  }

  static Color getBDColor() {
    return (Color) preferences.getInt("BDColor", (int)Color::BrightYellow);
  }

  static int getBootInfo() {
    return preferences.getInt("BootInfo", BOOTINFO_ENABLED);
  }

  static int getKeyClick() {
    return preferences.getInt("KeyClick", KEYCLICK_ENABLED);
  }

static int getSerCtl() {
    return preferences.getInt("SerCtl", SERCTL_ENABLED);
  }

static int getSerMir() {
    return preferences.getInt("SerMir", SERMIR_ENABLED);
  }

static int getSerFlt() {
    return preferences.getInt("SerFlt", SERFLT_DISABLED);
  }

  // if version in preferences doesn't match, reset preferences
  static void checkVersion() {
    if (preferences.getInt("VerMaj", 0) != TERMVERSION_MAJ || preferences.getInt("VerMin", 0) != TERMVERSION_MIN) {
      preferences.clear();
      preferences.putInt("VerMaj", TERMVERSION_MAJ);
      preferences.putInt("VerMin", TERMVERSION_MIN);
    }
  }

static int getFontIndex() {
    return preferences.getInt("FONT", 0);                    // default 0 = auto
  }


  static void loadConfiguration() {
    
    Terminal.setTerminalType(getTermType());
    Terminal.keyboard()->setLayout(SupportedLayouts::layouts()[getKbdLayoutIndex()]);
    // Terminal.connectLocally();                  // to use Terminal.read(), available(), etc..
    Terminal.setBackgroundColor(getBGColor());
    Terminal.setForegroundColor(getFGColor());
    // change the bold color but dont (false) apply the blurry/unsharp maintainStyle
    Terminal.setColorForAttribute(CharStyle::Bold, getBDColor(), false);
    if (getFontIndex() == 0) {
      // auto select a font from specified Columns and Rows or from 80x25
      Terminal.loadFont(fabgl::getPresetFontInfo(Terminal.canvas()->getWidth(), Terminal.canvas()->getHeight(), 80, 25));
    } else {
      // load specified font
      Terminal.loadFont(FONTS_INFO[getFontIndex()]);
    }
  }

};
