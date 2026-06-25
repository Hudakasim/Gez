#include <Arduino.h>
#include "kinematik.h"
#include "sabitler.h"

// Global hedef hızlar
float hedefRadsSag = 0.0;
float hedefRadsSol = 0.0;

void kinematikHesapla(float v, float w) {
  // v'yi sınırlara getir (önce min sonra max)
  if (v > 0 && v < MIN_HIZ) v = MIN_HIZ;
  if (v < 0 && v > -MIN_HIZ) v = -MIN_HIZ;
  v = constrain(v, -MAX_HIZ, MAX_HIZ);

  // Tekerlek hızlarını hesapla (m/s)
  float v_sag = v + (w * TEKERLEK_MESAFESI / 2.0);
  float v_sol = v - (w * TEKERLEK_MESAFESI / 2.0);

  // rad/s'e çevir ve sınırla
  hedefRadsSag = constrain(v_sag / TEKERLEK_R, -MAX_RADS, MAX_RADS);
  hedefRadsSol = constrain(v_sol / TEKERLEK_R, -MAX_RADS, MAX_RADS);
}
