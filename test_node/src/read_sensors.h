#ifndef READ_SENSORS
#define READ_SENSORS

#include <stdint.h>
#include "compression.h"

#include <Arduino.h>
#include <Wire.h>

constexpr int sensordata_length = 3;
bool init_sensors();
bool get_payload(uint8_t* payload);

#endif
