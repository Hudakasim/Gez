# Gez

Otonom diferansiyel sürüş robotu. ROS2 Humble + micro-ROS tabanlı.

## Yapı

firmware/    ESP32 firmware (PlatformIO + micro-ROS)

ros2_ws/     ROS2 workspace (URDF, launch dosyaları)


## Donanım

- Raspberry Pi 4 (8GB) - Ubuntu 22.04, ROS2 Humble
- ESP32 DevKit V1 - micro-ROS Client, PID kontrolü
- 2x Namiki 22CL-3501PG DC motor (80:1 redüktör, quadrature encoder)
- 2x IBT-2 motor sürücü
- RPLIDAR C1 (montaj devam ediyor)

## Çalıştırma

RPi'de:
```bash
ros2 launch gez_bringup gez.launch.py
```

Laptop'tan teleop ile sürmek için:
```bash
ros2 run teleop_twist_keyboard teleop_twist_keyboard
```

RViz görselleştirme:
```bash
rviz2
```
