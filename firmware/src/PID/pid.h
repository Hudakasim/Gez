#ifndef PID_H
#define PID_H

#include <Arduino.h>
#include <ESP32Encoder.h>

// Encoder ve PID modülünü başlat
void pidBaslat();

// Her 100ms'de timer tarafından çağrılır
void pidDongusu();

// Hedef değişince PID durumunu sıfırla (cmd_vel callback için)
void pidResetle();

// Dışarıdan erişilecek hız ölçümleri (odometri kullanır)
extern float gercekRadsSag;
extern float gercekRadsSol;

// Encoder objeleri (odometri tick okuyabilsin diye)
extern ESP32Encoder encSag;
extern ESP32Encoder encSol;

// PID timer ve tetik bayrağı
extern hw_timer_t* pidTimer;
extern volatile bool pidTetik;

#endif
