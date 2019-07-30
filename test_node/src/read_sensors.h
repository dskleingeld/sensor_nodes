#ifndef READ_SENSORS_H
#define READ_SENSORS_H

#include <stdint.h>
#include "compression.h"

#include <Arduino.h>
#include <Wire.h>

#include <Max44009.h>
#include <MHZ19.h>
#include <Zanshin_BME680.h>

extern HardwareSerial serial2;
constexpr int sensordata_length = 0+12+16+20+27+18+11;
bool init_sensors();

void read_to_package(uint8_t* payload);

// pressure in 1/100 pascal, humidity in 1/1000 percent, temperature in 1/100 degree, gas in 1/100 mOhm,
void encode_package(uint8_t* payload, int32_t temperature, 
  int32_t humidity, int32_t pressure, int32_t gas, float lux, int co2ppm);

// pressure in 1/100 pascal, humidity in 1/1000 percent, temperature in 1/100 degree, gas in 1/100 mOhm,
void print_values(uint8_t* payload, int32_t temperature, int32_t humidity, 
     int32_t pressure, int32_t gas, float lux, int co2ppm);

#endif
