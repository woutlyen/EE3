| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- |

# EE3 - Home Automation System
This repository currently stores the code for the main node, e.g. ESP32S3, for the EE3 project.

Below is short explanation of all the files in the project folder.

```
├── CMakeLists.txt
├── components
│   ├── microphone
│   │   ├── CMakeLists.txt
│   │   ├── include
│   │   │   └── microphone.h
│   │   └── microphone.c
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
│   └── main.c
└── README.md               This is the file you are currently reading
```
