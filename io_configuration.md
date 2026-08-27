# ESP32 Wireless Drawing System --- I/O Configuration

## System Overview

The project consists of two ESP32 boards:

-   **TX ESP32** --- receives drawing coordinates from the PC over USB
    Serial and transmits them using ESP-NOW.
-   **RX ESP32** --- receives drawing coordinates through ESP-NOW and
    displays them on a 1.3-inch 128×64 I²C OLED.

## TX ESP32 I/O Configuration

  Interface / Pin   Connection   Function
  ----------------- ------------ ------------------------------------------------------
  USB               PC           Serial communication with Python drawing application
  Wi-Fi radio       RX ESP32     ESP-NOW wireless transmission

### TX Serial Configuration

  Parameter   Value
  ----------- ------------------------
  Baud rate   115200
  Data        4-byte drawing command
  Protocol    USB Serial

### TX Drawing Packet

The PC sends four bytes:

`[X0] [Y0] [X1] [Y1]`

-   `X0`, `Y0` = starting coordinate
-   `X1`, `Y1` = ending coordinate
-   Coordinates use the OLED's 128×64 coordinate system.

## RX ESP32 I/O Configuration

### OLED Connections

  OLED Pin     ESP32 GPIO Function
  ---------- ------------ -----------
  VCC                3.3V Power
  GND                 GND Ground
  SDA             GPIO 21 I²C data
  SCL             GPIO 22 I²C clock

### OLED Configuration

  Parameter            Value
  -------------------- -----------------
  Display controller   SH1106
  Resolution           128 × 64 pixels
  Interface            I²C
  I²C address          `0x3C`
  Driver library       U8g2

### Status LED

  Component     ESP32 GPIO Function
  ----------- ------------ ------------------------------------------
  LED               GPIO 2 Blinks when a drawing packet is received

## ESP-NOW Configuration

  Parameter         TX         RX
  ----------------- ---------- ----------
  Wi-Fi mode        Station    Station
  ESP-NOW channel   1          1
  Encryption        Disabled   Disabled

### MAC Addresses

For a public repository, avoid publishing device-specific MAC addresses
unless necessary.

``` text
TX MAC: <TX_MAC>
RX MAC: <RX_MAC>
```

The transmitter is configured with the receiver's MAC address as its
ESP-NOW peer.

## Communication Flow

``` text
PC
 │
 │ USB Serial
 │ 4-byte coordinate packet
 ▼
TX ESP32
 │
 │ ESP-NOW
 │ 5-byte packet
 ▼
RX ESP32
 │
 │ I²C
 ▼
SH1106 OLED
```

## RX Drawing Logic

The RX receives:

`[TYPE] [X0] [Y0] [X1] [Y1]`

where:

``` text
TYPE = 1 → Draw line
```

The RX uses **Bresenham's line algorithm** to calculate the pixels
between the two coordinates. This allows horizontal, vertical, and
diagonal mouse strokes to be reproduced on the 128×64 OLED.

## Clear Command

The PC uses:

``` text
255 255 255 255
```

as a reserved clear-screen command.

The TX recognizes this pattern and sends a clear command to the RX. The
RX then clears the OLED framebuffer and updates the display.

## Software Components

  Component            Technology
  -------------------- ---------------------
  PC GUI               Python + Tkinter
  PC Serial            PySerial
  TX firmware          Arduino framework
  RX firmware          Arduino framework
  Wireless             ESP-NOW
  OLED driver          U8g2
  Display interface    I²C
  Line rasterization   Bresenham algorithm

## Hardware Summary

### TX

-   ESP32 Dev Module
-   USB connection to PC

### RX

-   ESP32 Dev Module
-   1.3-inch SH1106 128×64 OLED
-   LED on GPIO 2

### OLED Wiring

``` text
OLED          ESP32 RX
-----------------------
VCC     →     3.3V
GND     →     GND
SDA     →     GPIO 21
SCL     →     GPIO 22
```
