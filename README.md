| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- |

# EE3 - Home Automation System
This repository currently stores the code for the main node, e.g. ESP32S3, for the EE3 project.

Below is short explanation of all the files in the project folder.

```
├── CMakeLists.txt
├── components
│   ├── dimmable_light
│   │   ├── CMakeLists.txt
│   │   ├── dimmable_light.c
│   │   └── include
│   │       └── dimmable_light.h
│   ├── heating_temp_sensor
│   │   ├── CMakeLists.txt
│   │   ├── heating_temp_sensor.c
│   │   └── include
│   │       └── heating_temp_sensor.h
│   ├── microphone
│   │   ├── CMakeLists.txt
│   │   ├── include
│   │   │   └── microphone.h
│   │   └── microphone.c
│   ├── motion_sensor
│   │   ├── CMakeLists.txt
│   │   ├── include
│   │   │   └── motion_sensor.h
│   │   └── motion_sensor.c
│   ├── MQTT
│   │   ├── CMakeLists.txt
│   │   ├── include
│   │   │   └── MQTT.h
│   │   └── MQTT.c
│   ├── normal_light
│   │   ├── CMakeLists.txt
│   │   ├── include
│   │   │   └── normal_light.h
│   │   └── normal_light.c
│   ├── RGB_light
│   │   ├── CMakeLists.txt
│   │   ├── include
│   │   │   └── RGB_light.h
│   │   └── RGB_light.c
│   ├── speaker
│   │   ├── CMakeLists.txt
│   │   ├── include
│   │   │   └── speaker.h
│   │   └── speaker.c
│   ├── WiFi
│   │   ├── CMakeLists.txt
│   │   ├── include
│   │   │   └── WiFi.h
│   │   └── WiFi.c
│   └── wit_ai
│       ├── CMakeLists.txt
│       ├── include
│       │   └── wit_ai.h
│       └── wit_ai.c
├── main
│   ├── CMakeLists.txt
│   ├── config.h
│   ├── Kconfig.projbuild
│   ├── main.c
│   └── WavData.h
└── README.md               This is the file you are currently reading
```
