#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#define WIFI_CHANNEL 1

// RX MAC
uint8_t receiverMAC[] = {
    0xC0,
    0xCD,
    0xD6,
    0xCE,
    0x31,
    0xC4
};


// ========================================
// SEND DRAWING LINE
// ========================================

void sendLine(
    uint8_t x0,
    uint8_t y0,
    uint8_t x1,
    uint8_t y1
)
{
    uint8_t packet[5];

    packet[0] = 1;     // DRAW command
    packet[1] = x0;
    packet[2] = y0;
    packet[3] = x1;
    packet[4] = y1;


    esp_now_send(
        receiverMAC,
        packet,
        sizeof(packet)
    );


    // Small delay prevents flooding
    delay(5);
}


// ========================================
// CLEAR SCREEN
// ========================================

void clearScreen()
{
    uint8_t packet[4] = {
        255,
        255,
        255,
        255
    };


    esp_now_send(
        receiverMAC,
        packet,
        sizeof(packet)
    );


    delay(10);
}


// ========================================
// SETUP
// ========================================

void setup()
{
    Serial.begin(115200);


    // WiFi Station mode
    WiFi.mode(WIFI_STA);

    WiFi.disconnect();


    // Force channel 1
    esp_wifi_set_channel(
        WIFI_CHANNEL,
        WIFI_SECOND_CHAN_NONE
    );


    Serial.print("TX MAC: ");

    Serial.println(
        WiFi.macAddress()
    );


    Serial.print("Channel: ");

    Serial.println(
        WIFI_CHANNEL
    );


    // Initialize ESP-NOW
    if (
        esp_now_init() != ESP_OK
    )
    {
        Serial.println(
            "ESP-NOW INIT FAILED"
        );

        return;
    }


    // Add RX peer
    esp_now_peer_info_t peerInfo = {};


    memcpy(
        peerInfo.peer_addr,
        receiverMAC,
        6
    );


    peerInfo.channel =
        WIFI_CHANNEL;

    peerInfo.encrypt = false;


    if (
        esp_now_add_peer(
            &peerInfo
        ) != ESP_OK
    )
    {
        Serial.println(
            "PEER ADD FAILED"
        );

        return;
    }


    Serial.println(
        "LIVE DRAW TX READY"
    );
}


// ========================================
// LOOP
// ========================================

void loop()
{
    /*
       PC sends 4 bytes:

       x0
       y0
       x1
       y1
    */


    if (
        Serial.available() >= 4
    )
    {
        uint8_t x0 =
            Serial.read();

        uint8_t y0 =
            Serial.read();

        uint8_t x1 =
            Serial.read();

        uint8_t y1 =
            Serial.read();


        // =================================
        // CLEAR COMMAND
        // =================================

        if (
            x0 == 255 &&
            y0 == 255 &&
            x1 == 255 &&
            y1 == 255
        )
        {
            clearScreen();
        }


        // =================================
        // DRAW COMMAND
        // =================================

        else
        {
            sendLine(
                x0,
                y0,
                x1,
                y1
            );
        }
    }
}