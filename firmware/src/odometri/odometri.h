#ifndef ODOMETRI_H
#define ODOMETRI_H

// Her PID döngüsünde çağrılır, x/y/theta günceller
void odometriGuncelle();

// Dışarıdan okumak için (ROS yayınlayacak)
extern float odomX;
extern float odomY;
extern float odomTheta;
extern float vRobot;
extern float wRobot;

#endif
