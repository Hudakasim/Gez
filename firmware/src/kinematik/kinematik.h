#ifndef KINEMATIK_H
#define KINEMATIK_H

// Ters kinematik: v, w (robot) → hedef tekerlek hızları (rad/s)
// Hedef hızlar global değişkenlere yazılır
void kinematikHesapla(float v, float w);

// Hesaplanan hedef hızlar (rad/s)
extern float hedefRadsSag;
extern float hedefRadsSol;

#endif
