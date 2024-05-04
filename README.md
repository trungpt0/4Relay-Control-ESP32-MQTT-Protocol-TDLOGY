| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- |

# Control 4-channel Relays using ESP32 with MQTT Protocol

This project uses ESP-MQTT library which implements mqtt client to connect to mqtt broker.

## How to use

ESP32: subscriber
* idf.py -p (PORT) flash monitor
Users: publisher
* mosquitto_pub -h mqtt.eclipseprojects.io -t data -m "RELAY?:ON"
* Example: mosquitto_pub -h mqtt.eclipseprojects.io -t data -m "RELAY1:ON"

### Hardware Required

This example can be executed on any ESP32 board, the only required interface is WiFi and connection to internet.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output

```
TOPIC=data
DATA=RELAY1:ON
I (193962) MQTT_ESP32_4RELAY:: MQTT_EVENT_DATA
TOPIC=data
DATA=RELAY2:ON
I (200412) MQTT_ESP32_4RELAY:: MQTT_EVENT_DATA
TOPIC=data
DATA=RELAY3:ON
I (205022) MQTT_ESP32_4RELAY:: MQTT_EVENT_DATA
TOPIC=data
DATA=RELAY4:ON
I (211572) MQTT_ESP32_4RELAY:: MQTT_EVENT_DATA
TOPIC=data
DATA=RELAY1:OFF
I (216792) MQTT_ESP32_4RELAY:: MQTT_EVENT_DATA
TOPIC=data
DATA=RELAY2:OFF
I (220692) MQTT_ESP32_4RELAY:: MQTT_EVENT_DATA
TOPIC=data
DATA=RELAY3:OFF
I (224992) MQTT_ESP32_4RELAY:: MQTT_EVENT_DATA
TOPIC=data
DATA=RELAY4:OFF
I (547172) MQTT_ESP32_4RELAY:: MQTT_EVENT_DATA
```