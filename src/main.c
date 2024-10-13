#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"

#define GPIO_IN_REG         0x3FF4403C  // GPIO Input Reg 0-31
#define GPIO_IN1_REG        0x3FF44040  // GPIO Input Reg 32-39
#define GPIO_OUT_W1TS_REG   0x3FF44008  // GPIO Output Set Reg 0-31
#define GPIO_OUT_W1TC_REG   0x3FF4400C  // GPIO Output Clear Reg 0-31
#define GPIO_ENABLE_REG     0x3FF44020  // GPIO Enable 0-31
#define GPIO_OUT1_W1TS_REG  0x3FF44014  // GPIO Output Set Reg 32-39
#define GPIO_OUT1_W1TC_REG  0x3FF44018  // GPIO Output Clear Reg 32-39
#define GPIO_ENABLE1_REG    0x3FF4402C  // GPIO Enable 32-39

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

volatile uint32_t* gpio_in_reg        = (volatile uint32_t*) GPIO_IN_REG;
volatile uint32_t* gpio_in1_reg       = (volatile uint32_t*) GPIO_IN1_REG;
volatile uint32_t* gpio_out_w1ts_reg  = (volatile uint32_t*) GPIO_OUT_W1TS_REG;
volatile uint32_t* gpio_out_w1tc_reg  = (volatile uint32_t*) GPIO_OUT_W1TC_REG;
volatile uint32_t* gpio_enable_reg    = (volatile uint32_t*) GPIO_ENABLE_REG;
volatile uint32_t* gpio_out1_w1ts_reg = (volatile uint32_t*) GPIO_OUT1_W1TS_REG;
volatile uint32_t* gpio_out1_w1tc_reg = (volatile uint32_t*) GPIO_OUT1_W1TC_REG;
volatile uint32_t* gpio_enable1_reg   = (volatile uint32_t*) GPIO_ENABLE1_REG;

void app_main() {

  init_gpio();

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
  // Enable GPIO Pins
  *gpio_enable_reg  |= (1 << LEFT_SENSOR_PIN);
  *gpio_enable_reg  |= (1 << CENTER_SENSOR_PIN);
  *gpio_enable_reg  |= (1 << RIGHT_SENSOR_PIN);
  *gpio_enable_reg  |= (1 << AIN1);
  *gpio_enable1_reg |= (1 << (AIN2 - 32));
  *gpio_enable_reg  |= (1 << BIN1);
  *gpio_enable_reg  |= (1 << BIN2);
  

  // Initialize IR sensor GPIO pins
  *gpio_in_reg |= (1 << LEFT_SENSOR_PIN);
  *gpio_in_reg |= (1 << CENTER_SENSOR_PIN);
  *gpio_in_reg |= (1 << RIGHT_SENSOR_PIN);

  // Initialize motor control GPIO pins
  *gpio_out_w1ts_reg  |= (1 << AIN1);
  *gpio_out1_w1ts_reg |= (1 << (AIN2 - 32));
  *gpio_out_w1ts_reg  |= (1 << BIN1);
  *gpio_out_w1ts_reg  |= (1 << BIN2);
}

void move_forward() {
  *gpio_out_w1ts_reg  |= (1 << AIN1);
  *gpio_out1_w1tc_reg |= (1 << (AIN2 - 32));
  *gpio_out_w1ts_reg  |= (1 << BIN1);
  *gpio_out_w1tc_reg  |= (1 << BIN2);
}

void turn_left() {
  *gpio_out_w1tc_reg  |= (1 << AIN1);
  *gpio_out1_w1tc_reg |= (1 << (AIN2 - 32));
  *gpio_out_w1ts_reg  |= (1 << BIN1);
  *gpio_out_w1tc_reg  |= (1 << BIN2);
}

void turn_right() {
  *gpio_out_w1ts_reg  |= (1 << AIN1);
  *gpio_out1_w1tc_reg |= (1 << (AIN2 - 32));
  *gpio_out_w1tc_reg  |= (1 << BIN1);
  *gpio_out_w1tc_reg  |= (1 << BIN2);
}

void stop_motors() {
  *gpio_out_w1tc_reg  |= (1 << AIN1);
  *gpio_out1_w1tc_reg |= (1 << (AIN2 - 32));
  *gpio_out_w1tc_reg  |= (1 << BIN1);
  *gpio_out_w1tc_reg  |= (1 << BIN2);
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
