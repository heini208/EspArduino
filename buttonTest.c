#include <stdio.h>
#include "driver/gpio.h" // ESP32 GPIO functions
#include "freertos/FreeRTOS.h" // FreeRTOS for task delays
#include "freertos/task.h" // Task delay function

#define BUTTON_PIN GPIO_NUM_21 // GPIO21 connected to button

void app_main(void) {
    printf("Serial communication initialized at 9600 baud\n");

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    while (1) {
        int buttonState = gpio_get_level(BUTTON_PIN);

        printf("Button state: %d\n", buttonState);

        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}
