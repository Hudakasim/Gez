#include <Arduino.h>
#include "motor.h"
#include "pinler.h"
#include "sabitler.h"

void motorBaslat() {
  // Her LEDC kanalını ayarla ve pine bağla
  ledcSetup(CH_RPWM_SAG, PWM_FREQ, PWM_RES);
  ledcAttachPin(RPWM_SAG, CH_RPWM_SAG);

  ledcSetup(CH_LPWM_SAG, PWM_FREQ, PWM_RES);
  ledcAttachPin(LPWM_SAG, CH_LPWM_SAG);

  ledcSetup(CH_RPWM_SOL, PWM_FREQ, PWM_RES);
  ledcAttachPin(RPWM_SOL, CH_RPWM_SOL);

  ledcSetup(CH_LPWM_SOL, PWM_FREQ, PWM_RES);
  ledcAttachPin(LPWM_SOL, CH_LPWM_SOL);

  // Başlangıçta motoru durdur
  motorYaz(0, 0);
}

void motorYaz(float pwmSag, float pwmSol) {
  // PWM değerlerini geçerli aralığa zorla
  pwmSag = constrain(pwmSag, -1023, 1023);
  pwmSol = constrain(pwmSol, -1023, 1023);

  // Sağ motor: pozitifse ileri, negatifse geri
  if (pwmSag >= 0) {
    ledcWrite(CH_RPWM_SAG, (int)pwmSag);
    ledcWrite(CH_LPWM_SAG, 0);
  } else {
    ledcWrite(CH_RPWM_SAG, 0);
    ledcWrite(CH_LPWM_SAG, (int)(-pwmSag));
  }

  // Sol motor
  if (pwmSol >= 0) {
    ledcWrite(CH_RPWM_SOL, (int)pwmSol);
    ledcWrite(CH_LPWM_SOL, 0);
  } else {
    ledcWrite(CH_RPWM_SOL, 0);
    ledcWrite(CH_LPWM_SOL, (int)(-pwmSol));
  }
}
