#ifndef SABITLER_H
#define SABITLER_H

#include <Arduino.h>

// ENCODER
const float TICK_PER_TUR = 640.0;
const float ENC_DIR_SAG  = 1.0;
const float ENC_DIR_SOL  = -1.0;

// ZAMANLAMA
const float ORNEKLEME_SN = 0.1;   // 100 ms PID periyodu

// PWM
const int PWM_FREQ = 250;
const int PWM_RES  = 10;

// MOTOR LIMITLERI
const float MAX_RADS = 30.0;      // Tekerlek max açısal hız
const float MAX_HIZ  = 0.38;      // Robot max doğrusal hız (m/s)
const float MIN_HIZ  = 0.01;      // Robot min doğrusal hız

// ROBOT MEKANİĞİ
const float TEKERLEK_CAPI     = 0.065;
const float TEKERLEK_R        = TEKERLEK_CAPI / 2.0;
const float TEKERLEK_MESAFESI = 0.169;

// ROS
const size_t GEZ_DOMAIN_ID = 17;

#endif
