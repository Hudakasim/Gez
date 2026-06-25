# Gez 🤖

Sıfırdan tasarlanmış otonom diferansiyel sürüş robotu. ROS2 Humble + micro-ROS tabanlı.

![Status](https://img.shields.io/badge/status-development-yellow)
![ROS2](https://img.shields.io/badge/ROS2-Humble-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## Sistem Mimarisi

```
        ┌─────────────────────────────────┐
        │  Laptop                         │
        │  - RViz (görselleştirme)        │
        │  - teleop / Nav2 (komut)        │
        └───────────────┬─────────────────┘
                        │ WiFi (DDS)
                        │
        ┌───────────────▼─────────────────┐
        │  Raspberry Pi 4 (8GB)           │
        │  - ROS2 Humble                  │
        │  - micro-ROS Agent              │
        │  - URDF / robot_state_publisher │
        │  - odom_to_tf köprüsü           │
        └───────────────┬─────────────────┘
                        │ USB Serial (921600 baud)
                        │
        ┌───────────────▼─────────────────┐
        │  ESP32 DevKit V1                │
        │  - micro-ROS Client             │
        │  - PID kontrolü (Core 1)        │
        │  - Odometri hesabı              │
        └─────┬───────────┬───────────────┘
              │ PWM       │ Encoder
              ▼           ▲
        ┌──────────┐  ┌──────────┐
        │ IBT-2    │  │ Quad     │
        │ Sürücü   │  │ Encoder  │
        │ (×2)     │  │ (×2)     │
        └────┬─────┘  └────┬─────┘
             │             │
             └──┐    ┌─────┘
                ▼    │
            ┌──────────┐
            │ Namiki   │
            │ Motor    │
            │ (×2)     │
            └──────────┘
```

## Donanım

| Bileşen | Model | Açıklama |
|---------|-------|----------|
| Ana bilgisayar | Raspberry Pi 4 (8GB) | Ubuntu 22.04, ROS2 Humble |
| Düşük seviye kontrol | ESP32 DevKit V1 | Çift çekirdek, PCNT, LEDC |
| Motor | Namiki 22CL-3501PG | 12V, 80:1 redüktör, quadrature encoder |
| Motor sürücü | IBT-2 (BTS7960) | 2 adet (her motora bir) |
| Tekerlek | Ø65mm | İki tekerlek arası 169mm |
| LiDAR | RPLIDAR C1 | 🔜 montaj devam ediyor |

## Yazılım Yapısı

```
gez/
├── firmware/                  ESP32 firmware (PlatformIO)
│   ├── platformio.ini
│   └── src/
│       ├── main.cpp           Koordinasyon
│       ├── pinler.h           Pin tanımları
│       ├── sabitler.h         Sabitler (TEKERLEK_R, MAX_HIZ vb.)
│       ├── motor/             PWM motor sürüş
│       ├── kinematik/         Ters kinematik (v,w → tekerlek)
│       ├── PID/               Hız kontrolü
│       ├── odometri/          İleri kinematik (x,y,θ)
│       └── ros/               micro-ROS köprüsü
│
├── ros2_ws/                   ROS2 workspace
│   └── src/
│       ├── gez_description/   URDF + display launch
│       └── gez_bringup/       Ana sistem launch + odom_to_tf
│
└── docs/                      Teknik rapor (LaTeX)
```

## Kurulum

### 1. ROS2 Humble (Ubuntu 22.04)

```bash
# ROS2 Humble kurulumu
sudo apt update && sudo apt install software-properties-common
sudo add-apt-repository universe
sudo apt update && sudo apt install curl -y
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
    -o /usr/share/keyrings/ros-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" \
    | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null
sudo apt update
sudo apt install ros-humble-desktop ros-humble-xacro -y

# Bashrc'ye ekle
echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
```

### 2. micro-ROS Agent

```bash
mkdir -p ~/microros_ws/src
cd ~/microros_ws
git clone -b humble https://github.com/micro-ROS/micro_ros_setup.git src/micro_ros_setup
sudo apt install python3-colcon-common-extensions python3-rosdep
sudo rosdep init
rosdep update && rosdep install --from-paths src --ignore-src -y
colcon build
source install/local_setup.bash

ros2 run micro_ros_setup create_agent_ws.sh
ros2 run micro_ros_setup build_agent.sh
source install/local_setup.bash

# Bashrc'ye ekle
echo "source ~/microros_ws/install/local_setup.bash" >> ~/.bashrc
```

### 3. Bu repoyu klonla

```bash
cd ~
git clone https://github.com/KULLANICI_ADIN/gez.git
cd gez
```

### 4. ESP32 Firmware

PlatformIO kurulu olmalı (VSCode eklentisi veya CLI):

```bash
cd ~/gez/firmware
pio run --target upload
```

ESP32'nin USB portuna bağlı olduğundan emin ol (`/dev/ttyUSB0`).

### 5. ROS2 Paketleri

```bash
cd ~/gez/ros2_ws
colcon build
source install/setup.bash

# Bashrc'ye ekle
echo "source ~/gez/ros2_ws/install/setup.bash" >> ~/.bashrc
```

## Çalıştırma

### RPi'de (tek komut)

```bash
ros2 launch gez_bringup gez.launch.py
```

Bu launch dosyası şunları başlatır:
- micro-ROS Agent (ESP32 ile köprü)
- robot_state_publisher (URDF + TF)
- odom_to_tf (odom → base_link TF yayını)

### Laptop'tan görselleştirme

```bash
rviz2
```

RViz ayarları:
- **Fixed Frame**: `odom`
- **Add → RobotModel** → Description Topic: `/robot_description`

### Klavye ile sürüş

```bash
ros2 run teleop_twist_keyboard teleop_twist_keyboard
```

## ROS2 Topic'leri

| Topic | Tip | Yön | Açıklama |
|-------|-----|-----|----------|
| `/cmd_vel` | geometry_msgs/Twist | ESP32 → sub | Hız komutu (v, ω) |
| `/odom` | nav_msgs/Odometry | ESP32 → pub | Konum + hız |
| `/joint_states` | sensor_msgs/JointState | ESP32 → pub | Tekerlek açıları |
| `/tf` | tf2_msgs/TFMessage | RPi → pub | odom → base_link |
| `/tf_static` | tf2_msgs/TFMessage | RPi → pub | URDF link'leri |

## Durum

### ✅ Tamamlanan
- [x] PID hız kontrolü (MATLAB ITAE ile tasarlanmış)
- [x] Diferansiyel kinematik (ters + ileri)
- [x] Encoder tabanlı odometri
- [x] micro-ROS köprüsü (ESP32 ↔ ROS2)
- [x] URDF + TF ağacı
- [x] RViz görselleştirme
- [x] teleop ile manuel sürüş
- [x] Modüler kod yapısı

### 🔜 Yapılacaklar
- [ ] RPLIDAR C1 montajı (3D baskı parça bekleniyor)
- [ ] RPLIDAR driver entegrasyonu
- [ ] SLAM Toolbox ile harita oluşturma
- [ ] Nav2 ile otonom navigasyon
- [ ] Watchdog timer (güvenlik)
- [ ] Acil durdurma butonu

