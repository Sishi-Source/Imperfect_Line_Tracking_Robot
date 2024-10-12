#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_wifi.h"

// Define GPIO pins for the IR sensors
#define LEFT_SENSOR_PIN   21
#define CENTER_SENSOR_PIN 19
#define RIGHT_SENSOR_PIN  18

// Define GPIO pins for motor control
#define PWMA 32 // Motor A speed
#define AIN1 25 // Motor A direction
#define AIN2 33 // Motor A reverse
#define PWMB 14 // Motor B speed
#define BIN1 26 // Motor B direction
#define BIN2 27 // Motor B reverse

// Define threshold for line detection
#define LINE_DETECTED 0 // 0 means line detected (LOW output from sensor)

void init_gpio();
void move_forward();
void turn_left();
void turn_right();
void stop_motors();
void disable_bt();
void disable_wifi();
void init_pwm_channel(int gpio, int channel);
void set_speed(int channel, int speed);

void app_main() {
  init_gpio();
  disable_wifi();

  init_pwm_channel(PWMA, LEDC_CHANNEL_0);
  init_pwm_channel(PWMB, LEDC_CHANNEL_0);

  set_speed(LEDC_CHANNEL_0, 50);

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

void init_pwm_channel(int gpio, int channel) {
  ledc_channel_config_t pwm_config = {
    .channel = channel,
    .duty = 0,
    .gpio_num = gpio,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .hpoint = 0,
    .timer_sel = LEDC_TIMER_0
  };
  ledc_channel_config(&pwm_config);
}

void set_speed(int channel, int speed) {
  int duty_cycle = (speed * 8191)/100;
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, duty_cycle);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel);
}
