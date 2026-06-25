#include <Arduino.h>

#include "motor.h"
#include "pid.h"
#include "kinematik.h"
#include "odometri.h"
#include "ros.h"

// PID Task (Core 1'de sürekli çalışır)
void pidTask(void* parameter) {
  for (;;) {
    if (pidTetik) {
      pidTetik = false;
      pidDongusu();
      odometriGuncelle();
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void setup() {
  // 1. ROS başlat (Serial transport)
  rosBaslat();

  // 2. Motor PWM kurulumu
  motorBaslat();

  // 3. PID + encoder + timer
  pidBaslat();

  // 4. PID task'ı Core 1'de başlat
  xTaskCreatePinnedToCore(
    pidTask,
    "PID_Task",
    10000,
    NULL,
    1,
    NULL,
    1   // CORE 1
  );
}

void loop() {
  // Core 0: micro-ROS işleri
  rosDongusu();
}
