#include <Wire.h>

#define ESP32_I2C_ADDRESS 0x08
#define MAX_RETRIES 5

void setup()
{
    Wire.begin(); 
    Serial.begin(9600);
}

void loop()
{
    int retries = 0;
    byte error;



    do
    {
        Wire.beginTransmission(ESP32_I2C_ADDRESS);

        Wire.write(1); // Send trigger
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
        delay(1000); 
        return;      
    }

    delay(100); 


    Wire.requestFrom(ESP32_I2C_ADDRESS, 2);

    Serial.println(Wire.available());
    if (Wire.available() == 2)
    {
        uint16_t count = Wire.read() << 8 | Wire.read(); 
        Serial.print("Count from ESP32: ");
        Serial.println(count);
    }
    else
    {
        Serial.println("Failed to receive data from ESP32.");
    }

    delay(1000); 
}
