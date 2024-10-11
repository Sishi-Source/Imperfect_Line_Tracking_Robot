#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_wifi.h"

// Define GPIO pins for the IR sensors
#define LEFT_SENSOR_PIN   21
#define CENTER_SENSOR_PIN 19
#define RIGHT_SENSOR_PIN  18

// Define GPIO pins for motor control
#define AIN1  25 // Motor A direction
#define AIN2  26 // Motor A reverse
#define BIN1  27 // Motor B direction
#define BIN2  14 // Motor B reverse

// Define threshold for line detection
#define LINE_DETECTED 0 // 0 means line detected (LOW output from sensor)

void init_gpio();
void move_forward();
void turn_left();
void turn_right();
void stop_motors();
void disable_bt();
void disable_wifi();

void app_main() {
  init_gpio();
  disable_wifi();

  while (1) {
    int left_sensor = gpio_get_level(LEFT_SENSOR_PIN);
    int center_sensor = gpio_get_level(CENTER_SENSOR_PIN);
    int right_sensor = gpio_get_level(RIGHT_SENSOR_PIN);

    // Line following logic
    if (center_sensor == LINE_DETECTED) {
        move_forward();
    } else if (left_sensor == LINE_DETECTED) {
        turn_right();
    } else if (right_sensor == LINE_DETECTED) {
        turn_left();
    } else {
        stop_motors();
    }

    vTaskDelay(pdMS_TO_TICKS(100)); // Delay for a while before the next iteration
  }
}

void init_gpio() {
  // Initialize IR sensor GPIO pins
  gpio_set_direction(LEFT_SENSOR_PIN, GPIO_MODE_INPUT);
  gpio_set_direction(CENTER_SENSOR_PIN, GPIO_MODE_INPUT);
  gpio_set_direction(RIGHT_SENSOR_PIN, GPIO_MODE_INPUT);

  // Initialize motor control GPIO pins
  gpio_set_direction(AIN1, GPIO_MODE_OUTPUT);
  gpio_set_direction(AIN2, GPIO_MODE_OUTPUT);
  gpio_set_direction(BIN1, GPIO_MODE_OUTPUT);
  gpio_set_direction(BIN2, GPIO_MODE_OUTPUT);
}

void move_forward() {
  gpio_set_level(AIN1, 1);
  gpio_set_level(AIN2, 0);
  gpio_set_level(BIN1, 1);
  gpio_set_level(BIN2, 0);
}

void turn_left() {
  gpio_set_level(AIN1, 0);
  gpio_set_level(AIN2, 0);
  gpio_set_level(BIN1, 1);
  gpio_set_level(BIN2, 0);
}

void turn_right() {
  gpio_set_level(AIN1, 1);
  gpio_set_level(AIN2, 0);
  gpio_set_level(BIN1, 0);
  gpio_set_level(BIN2, 0);
}

void stop_motors() {
  gpio_set_level(AIN1, 0);
  gpio_set_level(AIN2, 0);
  gpio_set_level(BIN1, 0);
  gpio_set_level(BIN2, 0);
}

void disable_wifi() {
  esp_wifi_stop();
  esp_wifi_disconnect();
  esp_wifi_deinit();
}
