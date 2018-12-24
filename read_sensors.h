#ifndef READ_SENSORS
#define READ_SENSORS

#include <stdint.h>
#include "compression.h"

constexpr int sensordata_length = 2;

bool get_payload(uint8_t* payload);

#endif
