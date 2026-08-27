import tkinter as tk
import serial
import time


# ========================================
# SETTINGS
# ========================================

SERIAL_PORT = "COM5"       # CHANGE THIS
BAUD_RATE = 115200

WIDTH = 128
HEIGHT = 64

SCALE = 8


# ========================================
# SERIAL
# ========================================

print("Connecting to TX...")

ser = serial.Serial(
    SERIAL_PORT,
    BAUD_RATE,
    timeout=1
)

time.sleep(2)

print("Connected!")


# ========================================
# WINDOW
# ========================================

root = tk.Tk()

root.title(
    "ESP32 Wireless Drawing"
)


canvas = tk.Canvas(
    root,

    width=WIDTH * SCALE,

    height=HEIGHT * SCALE,

    bg="white",

    highlightthickness=1
)

canvas.pack(
    padx=10,
    pady=10
)


# ========================================
# DRAWING STATE
# ========================================

drawing = False

last_x = None
last_y = None


# ========================================
# SEND LINE
# ========================================

def send_line(
    x0,
    y0,
    x1,
    y1
):
    packet = bytes([
        x0,
        y0,
        x1,
        y1
    ])

    ser.write(packet)


# ========================================
# CLEAR
# ========================================

def clear():
    canvas.delete("all")

    # 255 255 255 255
    # = CLEAR command

    ser.write(
        bytes([
            255,
            255,
            255,
            255
        ])
    )


# ========================================
# MOUSE DOWN
# ========================================

def mouse_down(event):

    global drawing
    global last_x
    global last_y


    drawing = True


    x = int(
        event.x / SCALE
    )

    y = int(
        event.y / SCALE
    )


    # Keep inside OLED

    x = max(
        0,
        min(127, x)
    )

    y = max(
        0,
        min(63, y)
    )


    last_x = x
    last_y = y


    # Draw point on PC

    canvas.create_oval(
        x * SCALE + 1,

        y * SCALE + 1,

        x * SCALE + SCALE - 1,

        y * SCALE + SCALE - 1,

        fill="black",

        outline="black"
    )


    # Send point

    send_line(
        x,
        y,
        x,
        y
    )


# ========================================
# MOUSE MOVE
# ========================================

def mouse_move(event):

    global last_x
    global last_y


    if not drawing:
        return


    x = int(
        event.x / SCALE
    )

    y = int(
        event.y / SCALE
    )


    x = max(
        0,
        min(127, x)
    )

    y = max(
        0,
        min(63, y)
    )


    if (
        x == last_x and
        y == last_y
    ):
        return


    # ====================================
    # DRAW ON PC
    # ====================================

    canvas.create_line(

        last_x * SCALE +
        SCALE / 2,

        last_y * SCALE +
        SCALE / 2,

        x * SCALE +
        SCALE / 2,

        y * SCALE +
        SCALE / 2,

        fill="black",

        width=SCALE,

        capstyle=tk.ROUND
    )


    # ====================================
    # SEND TO TX
    # ====================================

    send_line(
        last_x,
        last_y,
        x,
        y
    )


    last_x = x
    last_y = y


# ========================================
# MOUSE UP
# ========================================

def mouse_up(event):

    global drawing

    drawing = False


# ========================================
# CLEAR BUTTON
# ========================================

clear_button = tk.Button(
    root,

    text="CLEAR",

    command=clear,

    width=15
)

clear_button.pack(
    pady=5
)


# ========================================
# MOUSE EVENTS
# ========================================

canvas.bind(
    "<Button-1>",
    mouse_down
)

canvas.bind(
    "<B1-Motion>",
    mouse_move
)

canvas.bind(
    "<ButtonRelease-1>",
    mouse_up
)


# ========================================
# CLOSE
# ========================================

def close():

    try:
        ser.close()
    except:
        pass

    root.destroy()


root.protocol(
    "WM_DELETE_WINDOW",
    close
)


# ========================================
# START
# ========================================

root.mainloop()