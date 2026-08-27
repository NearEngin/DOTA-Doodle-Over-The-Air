# ESP32 Wireless Drawing System

A real-time wireless drawing system that captures mouse input on a PC,
transmits drawing commands through an ESP32 transmitter using ESP-NOW,
and reproduces the drawing on a 1.3-inch 128×64 SH1106 OLED connected to
a second ESP32.

## Features

-   🖱️ Real-time mouse drawing from a PC
-   📡 Wireless communication using ESP-NOW
-   🖥️ Python/Tkinter drawing interface
-   🔌 USB Serial communication between PC and TX ESP32
-   📺 128×64 SH1106 OLED display
-   📐 Continuous horizontal, vertical, and diagonal lines
-   🧮 Bresenham line rasterization on the receiver
-   🧹 Clear button for both PC canvas and OLED
-   💡 GPIO 2 reception indicator
-   🚫 No Wi-Fi router required

------------------------------------------------------------------------

## System Architecture

``` text
┌──────────────────────┐
│         PC           │
│                      │
│ Python + Tkinter     │
│ Mouse Drawing        │
└──────────┬───────────┘
           │
           │ USB Serial
           │
           ▼
┌──────────────────────┐
│      ESP32 TX        │
│                      │
│ Serial → ESP-NOW     │
└──────────┬───────────┘
           │
           │ ESP-NOW
           │ Channel 1
           ▼
┌──────────────────────┐
│      ESP32 RX        │
│                      │
│ ESP-NOW → Bresenham  │
└──────────┬───────────┘
           │
           │ I²C
           ▼
┌──────────────────────┐
│   SH1106 OLED        │
│      128 × 64        │
└──────────────────────┘
```

------------------------------------------------------------------------

### Directories

  Directory   Purpose
  ----------- --------------------------------------
  `tx/`       ESP32 transmitter PlatformIO project
  
  `rx/`       ESP32 receiver PlatformIO project
  
  `Python_Code/`       Python mouse-drawing application

------------------------------------------------------------------------

## Hardware

### Transmitter

-   ESP32 Dev Module
-   USB connection to PC

### Receiver

-   ESP32 Dev Module
-   1.3-inch SH1106 128×64 OLED
-   Built-in/external LED on GPIO 2

------------------------------------------------------------------------

## OLED Wiring

The OLED is connected to the RX ESP32 using I²C.

  OLED     ESP32 RX Function
  ------ ---------- -----------
  VCC          3.3V Power
  GND           GND Ground
  SDA       GPIO 21 I²C data
  SCL       GPIO 22 I²C clock

### OLED Configuration

``` text
Controller: SH1106
Resolution: 128 × 64
Interface: I²C
Address:    0x3C
```

------------------------------------------------------------------------

## ESP-NOW Configuration

Both ESP32 boards operate in Wi-Fi Station mode and communicate directly
using ESP-NOW.

``` text
Channel: 1
Encryption: Disabled
```

The TX is configured with the RX MAC address as its ESP-NOW peer.

For a public repository, device-specific MAC addresses should preferably
be kept out of the README.

------------------------------------------------------------------------

## Software Requirements

### ESP32

-   VS Code
-   PlatformIO
-   Arduino framework
-   ESP32 platform
-   U8g2 library

The RX project requires:

``` text
olikraus/U8g2
```

### PC

-   Python 3
-   PySerial
-   Tkinter

Install PySerial with:

``` bash
python -m pip install pyserial
```

Tkinter is normally included with standard Python installations on
Windows.

------------------------------------------------------------------------

## Setup

### 1. Clone the repository

### 2. Upload the RX firmware

Open the `rx` folder in VS Code with PlatformIO and upload the firmware
to the receiver ESP32.

The RX connects to the OLED using:

``` text
SDA → GPIO 21
SCL → GPIO 22
```

### 3. Upload the TX firmware

Open the `tx` folder in PlatformIO and upload the firmware to the
transmitter ESP32.

The TX is connected to the PC through USB.

### 4. Configure the Python program

Open:

``` text
Python_Code/live_draw.py
```

Find:

``` python
SERIAL_PORT = "COM5"
```

Change it to the COM port assigned to the TX ESP32.

For example:

``` python
SERIAL_PORT = "COM7"
```

### 5. Run the drawing application

Close the PlatformIO Serial Monitor so that Python can access the COM
port.

Then run:

``` bash
python Python_Code/live_draw.py
```

------------------------------------------------------------------------

## Communication Protocol

The PC does not transmit the complete OLED framebuffer.

Instead, it transmits compact line coordinates.

### PC → TX

A drawing operation consists of four bytes:

``` text
[X0] [Y0] [X1] [Y1]
```

Where:

``` text
X0,Y0 = starting coordinate
X1,Y1 = ending coordinate
```

The coordinate system is:

``` text
X: 0–127
Y: 0–63
```

### TX → RX

The TX adds a command byte:

``` text
[TYPE] [X0] [Y0] [X1] [Y1]
```

For drawing:

``` text
TYPE = 1
```

Therefore a drawing packet is:

``` text
[01] [X0] [Y0] [X1] [Y1]
```

------------------------------------------------------------------------

## Clear Command

The PC uses the reserved sequence:

``` text
255 255 255 255
```

as a clear-screen command.

The TX recognizes this sequence and sends the corresponding clear
command to the RX.

The RX then clears its OLED framebuffer and updates the display.

------------------------------------------------------------------------

## Drawing Algorithm

The RX uses **Bresenham's line algorithm** to rasterize the received
line.

For example, if the PC sends:

``` text
(20, 10) → (30, 20)
```

the RX calculates the intermediate pixels rather than displaying only
the endpoints.

``` text
●
  ●
    ●
      ●
        ●
          ●
```

This allows the system to reproduce:

-   Horizontal lines
-   Vertical lines
-   Diagonal lines
-   Arbitrary mouse strokes

Bresenham's algorithm is particularly suitable for embedded systems
because it uses integer-based calculations and avoids unnecessary
floating-point operations.

------------------------------------------------------------------------

## Why Line Commands Instead of Images?

A 128×64 monochrome OLED contains:

``` text
128 × 64 = 8192 pixels
```

At one bit per pixel, a complete framebuffer requires:

``` text
8192 / 8 = 1024 bytes
```

Sending complete frames repeatedly would consume significantly more
bandwidth.

Instead, this project transmits only the geometry of each mouse stroke:

``` text
X0 Y0 X1 Y1
```

This reduces each drawing operation to just a few bytes.

------------------------------------------------------------------------

## Data Flow Example

Suppose the mouse moves from:

``` text
(50, 20)
```

to:

``` text
(55, 25)
```

### PC

Sends:

``` text
50 20 55 25
```

### TX

Creates:

``` text
01 50 20 55 25
```

and transmits it using ESP-NOW.

### RX

Receives:

``` text
TYPE = 1
X0 = 50
Y0 = 20
X1 = 55
Y1 = 25
```

Bresenham's algorithm calculates the pixels along the line and U8g2
updates the OLED.

------------------------------------------------------------------------

## Status LED

GPIO 2 on the RX is used as a reception indicator.

When a drawing packet is processed:

``` text
GPIO 2 → HIGH
        ↓
     ~10 ms
        ↓
GPIO 2 → LOW
```

This provides a simple visual indication that the RX is receiving
drawing data.

------------------------------------------------------------------------

## Technologies Used

  Layer                     Technology
  ------------------------- ---------------------
  GUI                       Python Tkinter
  PC Serial                 PySerial
  TX Firmware               Arduino C++
  RX Firmware               Arduino C++
  Wireless                  ESP-NOW
  Display Driver            U8g2
  Display Interface         I²C
  Display Controller        SH1106
  Line Rasterization        Bresenham Algorithm
  Development Environment   PlatformIO

------------------------------------------------------------------------

## Future Improvements

Possible extensions include:

-   Wireless image transmission
-   Multiple drawing tools
-   Eraser mode
-   Adjustable brush size
-   Undo/redo
-   Shapes such as circles and rectangles
-   Touchscreen input
-   Bidirectional communication
-   Packet acknowledgement and retransmission
-   Drawing synchronization
-   Lower-latency packet scheduling
-   Saving drawings to an SD card

------------------------------------------------------------------------

## License

This project can be released under an open-source license of your
choice.
