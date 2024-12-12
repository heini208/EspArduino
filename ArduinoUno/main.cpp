#include <Wire.h>

#define ESP32_I2C_ADDRESS 0x08
#define BUTTON_PIN 2

bool continuousMode = false;
bool lastButtonState = HIGH;

void setup()
{
    Wire.begin(); // Start I2C as Master
    Serial.begin(9600);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop()
{
    bool buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW && lastButtonState == HIGH)
    {
        // Toggle mode on button press
        continuousMode = !continuousMode;

        if (continuousMode)
        {
            Serial.println("Switching to continuous mode...");
            // Send '2' to ESP32 to switch it to continuous mode
            sendCommandToESP32(2);
        }
        else
        {
            Serial.println("Switching to trigger mode...");
            // In trigger mode, ESP32 only sends data after receiving '1'
            // No immediate action needed here, just note the mode change
        }

        delay(200); // Debounce delay
    }
    lastButtonState = buttonState;

    if (continuousMode)
    {
        // In continuous mode, just request data repeatedly without triggers
        requestDataFromESP32();
        delay(500);
    }
    else
    {
        // In trigger mode, send a trigger and then get one reading
        sendCommandToESP32(1);
        delay(100); // Wait for ESP32 to prepare the response
        requestDataFromESP32();
        delay(1000);
    }
}

void sendCommandToESP32(uint8_t cmd)
{
    int retries = 0;
    byte error;
    do
    {
        Wire.beginTransmission(ESP32_I2C_ADDRESS);
        Wire.write(cmd);
        error = Wire.endTransmission();
        if (error != 0)
        {
            Serial.print("Transmission failed (Error ");
            Serial.print(error);
            Serial.println("), retrying...");
            retries++;
            delay(100);
        }
    } while (error != 0 && retries < MAX_RETRIES);

    if (error != 0)
    {
        Serial.println("Failed to communicate with ESP32 after retries.");
    }
}

void requestDataFromESP32()
{
    Wire.requestFrom(ESP32_I2C_ADDRESS, 2);
    if (Wire.available() == 2)
    {
        uint16_t count = (Wire.read() << 8) | Wire.read();
        Serial.print("Count from ESP32: ");
        Serial.println(count);
    }
    else
    {
        Serial.println("Failed to receive data from ESP32.");
    }
}
