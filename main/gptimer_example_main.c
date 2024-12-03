#include <stdio.h>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"

#define I2C_SLAVE_ADDRESS 0x08
#define I2C_PORT I2C_NUM_0
#define SDA_PIN 21
#define SCL_PIN 22

uint16_t count = 0;

void timer_callback(void *param)
{
    count++;
}

void i2c_slave_init()
{
    printf("Initializing I2C slave...\n"); 

    i2c_config_t conf = {
        .mode = I2C_MODE_SLAVE,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .slave = {
            .slave_addr = I2C_SLAVE_ADDRESS,
            .maximum_speed = 100000 // 100kHz
        }
    };

    esp_err_t param_config_status = i2c_param_config(I2C_PORT, &conf);
    if (param_config_status == ESP_OK)
    {
        printf("I2C parameter configuration successful.\n");
    }
    else
    {
        printf("I2C parameter configuration failed. Error: %d\n", param_config_status);
    }

    esp_err_t driver_install_status = i2c_driver_install(I2C_PORT, I2C_MODE_SLAVE, 1024, 1024, 0);
    if (driver_install_status == ESP_OK)
    {
        printf("I2C driver installation successful.\n");
    }
    else
    {
        printf("I2C driver installation failed. Error: %d\n", driver_install_status);
    }
}

void app_main(void)
{
    printf("Starting ESP32 application...\n");

    // Initialize I2C Slave
    i2c_slave_init();

    // Setup Timer
    const esp_timer_create_args_t my_timer_args = 
    {
        .callback = &timer_callback,
        .name = "Timer Interrupt"
    };

    esp_timer_handle_t timer_handler;
    esp_err_t timer_create_status = esp_timer_create(&my_timer_args, &timer_handler);
    if (timer_create_status == ESP_OK)
    {
        printf("Timer created successfully.\n");
    }
    else
    {
        printf("Timer creation failed. Error: %d\n", timer_create_status);
    }

    esp_err_t timer_start_status = esp_timer_start_periodic(timer_handler, 10000);  // 10ms interval
    if (timer_start_status == ESP_OK)
    {
        printf("Timer started successfully.\n");
    }
    else
    {
        printf("Timer start failed. Error: %d\n", timer_start_status);
    }

    uint8_t received_data = 0;

    while (true)
    {
        // Read data from the I2C master
        int bytes_read = i2c_slave_read_buffer(I2C_PORT, &received_data, 1, pdMS_TO_TICKS(100));
        if (bytes_read > 0)
        {
            printf("Data received from master: %d\n", received_data);

            if (received_data == 1)
            {
                uint8_t data_to_send[2] = { (uint8_t)(count >> 8), (uint8_t)(count & 0xFF) };
                printf("Sending count: %d\n", count);

                esp_err_t send_status = i2c_slave_write_buffer(I2C_PORT, data_to_send, 2, pdMS_TO_TICKS(100));
                if (send_status == ESP_OK)
                {
                    printf("Count sent successfully.\n");
                }
                else
                {
                    printf("Failed to send count. Error: %d\n", send_status);
                }
            }
        }
        else if (bytes_read < 0)
        {
            printf("Error reading from master. Error: %d\n", bytes_read);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS); 
    }
}
