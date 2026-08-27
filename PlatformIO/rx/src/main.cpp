#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <U8g2lib.h>

#define WIFI_CHANNEL 1

#define LED_PIN 2

#define OLED_ADDRESS 0x3C


// ========================================
// OLED
// ========================================

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(
    U8G2_R0,
    U8X8_PIN_NONE
);


// ========================================
// LINE PACKET
// ========================================

struct LinePacket
{
    uint8_t type;

    uint8_t x0;
    uint8_t y0;

    uint8_t x1;
    uint8_t y1;
};


LinePacket receivedPacket;


// ========================================
// FLAGS
// ========================================

volatile bool newLine = false;

volatile bool clearDisplay = false;


// ========================================
// DRAW LINE
// ========================================

void drawLine(
    int x0,
    int y0,
    int x1,
    int y1
)
{
    int dx =
        abs(x1 - x0);

    int dy =
        abs(y1 - y0);


    int sx =
        (x0 < x1)
        ? 1
        : -1;


    int sy =
        (y0 < y1)
        ? 1
        : -1;


    int err =
        dx - dy;


    while (true)
    {
        // Make sure pixel is inside OLED

        if (
            x0 >= 0 &&
            x0 < 128 &&
            y0 >= 0 &&
            y0 < 64
        )
        {
            oled.drawPixel(
                x0,
                y0
            );
        }


        // Finished

        if (
            x0 == x1 &&
            y0 == y1
        )
        {
            break;
        }


        int e2 =
            2 * err;


        if (e2 > -dy)
        {
            err -= dy;

            x0 += sx;
        }


        if (e2 < dx)
        {
            err += dx;

            y0 += sy;
        }
    }
}


// ========================================
// ESP-NOW RECEIVE CALLBACK
// ========================================

void onDataRecv(
    const uint8_t* mac,
    const uint8_t* incomingData,
    int len
)
{
    // ====================================
    // CLEAR COMMAND
    // ====================================

    if (
        len == 4 &&
        incomingData[0] == 255 &&
        incomingData[1] == 255 &&
        incomingData[2] == 255 &&
        incomingData[3] == 255
    )
    {
        clearDisplay = true;

        return;
    }


    // ====================================
    // DRAW COMMAND
    // ====================================

    if (len != 5)
        return;


    if (incomingData[0] != 1)
        return;


    memcpy(
        &receivedPacket,
        incomingData,
        sizeof(LinePacket)
    );


    newLine = true;
}


// ========================================
// SETUP
// ========================================

void setup()
{
    Serial.begin(115200);


    // ====================================
    // LED
    // ====================================

    pinMode(
        LED_PIN,
        OUTPUT
    );

    digitalWrite(
        LED_PIN,
        LOW
    );


    // ====================================
    // OLED
    // ====================================

    oled.setI2CAddress(
        OLED_ADDRESS << 1
    );

    oled.begin();


    oled.clearBuffer();

    oled.sendBuffer();


    // ====================================
    // WIFI
    // ====================================

    WiFi.mode(WIFI_STA);

    WiFi.disconnect();


    esp_wifi_set_channel(
        WIFI_CHANNEL,
        WIFI_SECOND_CHAN_NONE
    );


    Serial.print("RX MAC: ");

    Serial.println(
        WiFi.macAddress()
    );


    Serial.print("Channel: ");

    Serial.println(
        WIFI_CHANNEL
    );


    // ====================================
    // ESP-NOW
    // ====================================

    if (
        esp_now_init() != ESP_OK
    )
    {
        Serial.println(
            "ESP-NOW INIT FAILED"
        );

        return;
    }


    esp_now_register_recv_cb(
        onDataRecv
    );


    Serial.println(
        "LIVE DRAW RX READY"
    );
}


// ========================================
// LOOP
// ========================================

void loop()
{
    // ====================================
    // CLEAR
    // ====================================

    if (clearDisplay)
    {
        clearDisplay = false;


        oled.clearBuffer();

        oled.sendBuffer();
    }


    // ====================================
    // DRAW
    // ====================================

    if (newLine)
    {
        newLine = false;


        drawLine(
            receivedPacket.x0,
            receivedPacket.y0,

            receivedPacket.x1,
            receivedPacket.y1
        );


        oled.sendBuffer();


        // Blink LED

        digitalWrite(
            LED_PIN,
            HIGH
        );

        delay(10);

        digitalWrite(
            LED_PIN,
            LOW
        );
    }


    delay(1);
}