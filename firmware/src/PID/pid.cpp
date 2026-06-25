#include <Arduino.h>
#include "pid.h"
#include "motor.h"
#include "kinematik.h"
#include "sabitler.h"
#include "pinler.h"

// --- ENCODER ---
ESP32Encoder encSag;
ESP32Encoder encSol;

// --- HIZ ÖLÇÜMÜ ---
float gercekRadsSag = 0.0;
float gercekRadsSol = 0.0;

// --- PID KATSAYILARI ---
static float Kp = 150.0;
static float Ki = 600.0;
static float Kd = 1.0;

// --- PID DURUM DEĞİŞKENLERİ ---
static float anlikHedefRadsSag = 0.0;
static float anlikHedefRadsSol = 0.0;
static float hataSag = 0, toplamHataSag = 0, oncekiHataSag = 0;
static float hataSol = 0, toplamHataSol = 0, oncekiHataSol = 0;
static float pwmSag = 0, pwmSol = 0;

// --- TIMER ---
hw_timer_t*    pidTimer = NULL;
volatile bool  pidTetik = false;
void IRAM_ATTR onPidTimer() { pidTetik = true; }

void pidBaslat() {
  // Encoder kurulumu
  ESP32Encoder::useInternalWeakPullResistors = puType::up;
  encSag.attachFullQuad(ENC_SAG_A, ENC_SAG_B);
  encSol.attachFullQuad(ENC_SOL_A, ENC_SOL_B);
  encSag.setCount(0);
  encSol.setCount(0);

  // Timer kurulumu (100ms)
  pidTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(pidTimer, &onPidTimer, true);
  timerAlarmWrite(pidTimer, 100000, true);
  timerAlarmEnable(pidTimer);
}

void pidResetle() {
  toplamHataSag = 0; toplamHataSol = 0;
  oncekiHataSag = 0; oncekiHataSol = 0;
  // Dur komutu ise rampi da sıfırla
  if (hedefRadsSag == 0 && hedefRadsSol == 0) {
    anlikHedefRadsSag = 0;
    anlikHedefRadsSol = 0;
  }
}

void pidDongusu() {
  // 1. Enkoder oku ve sıfırla
  int32_t deltaSag = encSag.getCount(); encSag.clearCount();
  int32_t deltaSol = encSol.getCount(); encSol.clearCount();

  // 2. Hız hesapla (low-pass filtreli)
  float yeniSag = (deltaSag / TICK_PER_TUR) * (2.0 * PI) / ORNEKLEME_SN * ENC_DIR_SAG;
  float yeniSol = (deltaSol / TICK_PER_TUR) * (2.0 * PI) / ORNEKLEME_SN * ENC_DIR_SOL;
  gercekRadsSag = 0.7 * gercekRadsSag + 0.3 * yeniSag;
  gercekRadsSol = 0.7 * gercekRadsSol + 0.3 * yeniSol;

  // 3. Hatalı encoder okumalarını filtrele
  if (abs(gercekRadsSag) > MAX_RADS * 1.5) gercekRadsSag = 0;
  if (abs(gercekRadsSol) > MAX_RADS * 1.5) gercekRadsSol = 0;

  // 4. Ramp (yumuşak hızlanma)
  const float MAX_ARTIS = 1.5;
  if (hedefRadsSag > anlikHedefRadsSag) {
    anlikHedefRadsSag = min(anlikHedefRadsSag + MAX_ARTIS, hedefRadsSag);
  } else if (hedefRadsSag < anlikHedefRadsSag) {
    anlikHedefRadsSag = max(anlikHedefRadsSag - MAX_ARTIS, hedefRadsSag);
  }
  if (hedefRadsSol > anlikHedefRadsSol) {
    anlikHedefRadsSol = min(anlikHedefRadsSol + MAX_ARTIS, hedefRadsSol);
  } else if (hedefRadsSol < anlikHedefRadsSol) {
    anlikHedefRadsSol = max(anlikHedefRadsSol - MAX_ARTIS, hedefRadsSol);
  }

  // 5. Dur kontrolü
  if (hedefRadsSag == 0 && hedefRadsSol == 0 &&
      anlikHedefRadsSag == 0 && anlikHedefRadsSol == 0) {
    pwmSag = 0; toplamHataSag = 0; oncekiHataSag = 0;
    pwmSol = 0; toplamHataSol = 0; oncekiHataSol = 0;
    motorYaz(0, 0);
    return;
  }

  // 6. Sağ PID
  hataSag = anlikHedefRadsSag - gercekRadsSag;
  toplamHataSag += hataSag * ORNEKLEME_SN;
  toplamHataSag = constrain(toplamHataSag, -5, 5);
  float dHataSag = (hataSag - oncekiHataSag) / ORNEKLEME_SN;
  pwmSag = (Kp * hataSag) + (Ki * toplamHataSag) + (Kd * dHataSag);
  pwmSag = constrain(pwmSag, -1023, 1023);
  oncekiHataSag = hataSag;

  // 7. Sol PID
  hataSol = anlikHedefRadsSol - gercekRadsSol;
  toplamHataSol += hataSol * ORNEKLEME_SN;
  toplamHataSol = constrain(toplamHataSol, -5, 5);
  float dHataSol = (hataSol - oncekiHataSol) / ORNEKLEME_SN;
  pwmSol = (Kp * hataSol) + (Ki * toplamHataSol) + (Kd * dHataSol);
  pwmSol = constrain(pwmSol, -1023, 1023);
  oncekiHataSol = hataSol;

  // 8. Motora yaz
  motorYaz(pwmSag, pwmSol);
}
