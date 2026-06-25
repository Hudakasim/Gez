#ifndef MOTOR_H
#define MOTOR_H

// PWM modülünü başlat ve pinleri bağla
void motorBaslat();

// İki motora aynı anda PWM yaz (pozitif = ileri, negatif = geri)
void motorYaz(float pwmSag, float pwmSol);

#endif
