#ifndef READ_SENSORS_H
#define READ_SENSORS_H

#include <stdint.h>
#include "compression.h"
#include "error.hpp"

#include <Arduino.h>
#include <Wire.h>

#include <Max44009.h>
#include <MHZ19.h>
#include <Zanshin_BME680.h>

constexpr int divide_ceil(int x, int y) {
	return (x + y - 1) / y;
}

constexpr int sensordata_length = divide_ceil(0+12+16+20+27+18+11, 8);

class Sensors {
  public:
    Sensors();
    Error init();
    Error configure();

    BME680_Class bme680;
    MHZ19 mhz19;
    Max44009 max44009;
};

Error read_to_package(Sensors &sensors, uint8_t* payload);

// pressure in 1/100 pascal, humidity in 1/1000 percent, temperature in 1/100 degree, gas in 1/100 mOhm,
void encode_package(uint8_t* payload, int32_t temperature, 
  int32_t humidity, int32_t pressure, int32_t gas, float lux, int co2ppm);

// pressure in 1/100 pascal, humidity in 1/1000 percent, temperature in 1/100 degree, gas in 1/100 mOhm,
void print_values(uint8_t* payload, int32_t temperature, int32_t humidity, 
     int32_t pressure, int32_t gas, float lux, int co2ppm);

#endif
