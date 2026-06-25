#ifndef ROS_H
#define ROS_H

#include <Arduino.h>

// Agent durumları
enum AgentDurum { BEKLEME, BAGLANIYOR, BAGLI, KOPUK };

// ROS başlat (Serial transport + state machine init)
void rosBaslat();

// loop() içinden sürekli çağrılır
void rosDongusu();

// Dışarıdan agent durumunu okumak için
extern AgentDurum agentDurum;

#endif
