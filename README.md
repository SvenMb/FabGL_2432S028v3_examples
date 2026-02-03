# Some examples to use the FabGL with the Cheap Yellow Display 2432s028
these examples are all made for the version 3 with a ST7789 display, but it should be easy
to move to the ILI9344 display on older CYD boards. 

## optional Hardware mods
### PS2 interface for keyboard
FabGL uses the ULP for implementing the PS2 protocol. Unfortunately there aren't 2 ULP
able ports avaiable on the connectors, so I had to modify the board to archive a PS2 port.

I modifyed the CN1 port to contain the GPIO04 instead of the GPIO22
(which can not be used with the ULP). GPIO22 will stay avaiable on P3.

- remove RGB-LED
- remove R17
- cut copper trace to GPIO22
- add wire from LED/GPIO04 to pin on CN1

I also modifyed the PS2 connect to include some resistors. (120Ohm in line to both ports,
2kOhm pull-up from PS2-DAT and PS2-CLK to VCC, see FabGL PS2 ports schema)

Most newer keyboards will work with the 3,3V already avaiable on the port, but I recommend
to put 5V (4,5V after that diode) on that port instead.

### PSRAM for more advanced programs
see old

### shortcut resistor for RX on P1
If input is feeded into the RX port on P1 nothing will be received on the ESP32, this is due
the 100Ohm R6 and the CH340 feeding on the same GPIO03.
So easy bad dirty trick is to short cut the R6, then serial IO on P1 will work. (I don't think this will destroy the CH340 over time.)
