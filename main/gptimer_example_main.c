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
bool continuous_mode = false;

void timer_callback(void *param)
{
    count++;
}

void i2c_slave_init()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_SLAVE,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .slave = {
            .slave_addr = I2C_SLAVE_ADDRESS,
            .maximum_speed = 100000 // 100kHz
        }};

    i2c_param_config(I2C_PORT, &conf);
    i2c_driver_install(I2C_PORT, I2C_MODE_SLAVE, 1024, 1024, 0);
}

void clear_i2c_buffer()
{
    uint8_t dummy;
    while (i2c_slave_read_buffer(I2C_PORT, &dummy, 1, 0) > 0)
    {
        // clear the buffer
    }
}

void app_main(void)
{
    printf("Starting ESP32 application\n");

    i2c_slave_init();

    const esp_timer_create_args_t my_timer_args =
        {
            .callback = &timer_callback,
            .name = "Timer Interrupt"};
    esp_timer_handle_t timer_handler;
    esp_timer_create(&my_timer_args, &timer_handler);
    esp_timer_start_periodic(timer_handler, 10000); // 10ms interval

    uint8_t received_data = 0;

    while (true)
    {
        int bytes_read = i2c_slave_read_buffer(I2C_PORT, &received_data, 1, pdMS_TO_TICKS(100));
        if (bytes_read > 0)
        {
            if (received_data == 2)
            {
                printf("Swtichting mode...\n");
                continuous_mode = !continuous_mode;
            }
            else if (received_data == 1 && !continuous_mode)
            {
                printf("Sent Count in trigger mode: %d\n", count);
                clear_i2c_buffer();
                uint8_t data_to_send[2] = {(uint8_t)(count >> 8), (uint8_t)(count & 0xFF)};
                i2c_slave_write_buffer(I2C_PORT, data_to_send, 2, pdMS_TO_TICKS(100));
            }
        }
        if (continuous_mode)
        {
            printf("Sent Count in continuous_mode: %d\n", count);
            clear_i2c_buffer();
            uint8_t data_to_send[2] = {(uint8_t)(count >> 8), (uint8_t)(count & 0xFF)};
            i2c_slave_write_buffer(I2C_PORT, data_to_send, 2, pdMS_TO_TICKS(100));
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
