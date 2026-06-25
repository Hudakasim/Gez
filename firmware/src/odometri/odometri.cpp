#include <Arduino.h>
#include "odometri.h"
#include "pid.h"
#include "sabitler.h"

// Global durum
float odomX = 0.0;
float odomY = 0.0;
float odomTheta = 0.0;

float vRobot = 0.0;
float wRobot = 0.0;

void odometriGuncelle() {
  // Tekerlek hızlarını m/s'e çevir
  float v_sag_ms = gercekRadsSag * TEKERLEK_R;
  float v_sol_ms = gercekRadsSol * TEKERLEK_R;

  // İleri kinematik: tekerlek hızları → robot hızları
  vRobot = (v_sag_ms + v_sol_ms) / 2.0;
  wRobot = (v_sag_ms - v_sol_ms) / TEKERLEK_MESAFESI;

  // Euler integrasyonu ile konum güncelle
  odomX     += vRobot * cos(odomTheta) * ORNEKLEME_SN;
  odomY     += vRobot * sin(odomTheta) * ORNEKLEME_SN;
  odomTheta += wRobot * ORNEKLEME_SN;

  // Theta'yı -PI ile PI arasında tut
  while (odomTheta >  PI) odomTheta -= 2.0 * PI;
  while (odomTheta < -PI) odomTheta += 2.0 * PI;
}
