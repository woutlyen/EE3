# EE3 - Home Automation System
This repository currently stores the code for the main node (ESP32S3), the entry node (STM8), lights/heating node (STM8), NTC temperature sensor and for the AS608 fingerprint sensor for the EE3 project.

Below is short explanation of all the files in the ESP32 folder.

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
│   ├── mirf
│   │   ├── CMakeLists.txt
│   │   ├── include
│   │   │   └── mirf.h
│   │   └── mirf.c
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
│   ├── SD
│   │   ├── CMakeLists.txt
│   │   ├── include
│   │   │   └── SD.h
│   │   └── SD.c
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
