#include "read_sensors.hpp"

Sensors::Sensors()
  : bme680(), mhz19(), max44009(0x4A, 21, 22)
{
}

Error Sensors::init(){
  //RXD2 16, TXD2 17
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  mhz19.setSerial(&Serial2);

  uint64_t start = millis();
  uint64_t timeout = 1000;
  while (!bme680.begin(I2C_STANDARD_MODE)) { //start BME
    uint64_t elapsed = millis() - start;
    if(elapsed > timeout){ return Error::CANT_FIND_BME680; }
      Serial.println(F("-  Unable to find BME680. Waiting .1 second")); delay(100); 
  }
  return Error::NONE;
}

Error Sensors::configure(){
  if (!mhz19.enableABC()) {Serial.println("COULD NOT CONFIG mhz19"); return Error::CANT_CONFIGURE_MHZ19;}
  mhz19.setRange(2000);  

  bme680.setOversampling(TemperatureSensor, Oversample16); // Use enumerated type values
  bme680.setOversampling(HumiditySensor, Oversample16);
  bme680.setOversampling(PressureSensor, Oversample16);
  
  bme680.setIIRFilter(IIR4); // Setting IIR filter to a value of 4 samples;
  bme680.setGas(320,150); // 320°c for 150 milliseconds
  return Error::NONE;
}

Error read_to_package(Sensors &sensors, uint8_t* payload){

  //Serial.println(readCO2(serial2));

  // if (!mhz19.isReady()) { delay(1000); }// Checking if sensor had preheated for 3 mins
  
  int co2ppm = sensors.mhz19.readValue(); // Reading CO2 value. (Returns -1 if response wasn't received)

  float lux = sensors.max44009.getLux();
  int err = sensors.max44009.getError();
  if (err != 0) {
     Serial.print("Error:\t");
     Serial.println(err);
     return Error::MAX44009_LIB_ERROR;
  }

  int32_t temperature, humidity, pressure, gas;     // Variable to store readings
  sensors.bme680.getSensorData(temperature,humidity,pressure,gas); // Get most recent readings

  encode_package(payload, temperature, humidity, pressure, gas, lux, co2ppm);
  print_values(payload, temperature, humidity, pressure, gas, lux, co2ppm);
  
  return Error::NONE;
}

// pressure in 1/100 pascal, humidity in 1/1000 percent, temperature in 1/100 degree, gas in 1/100 mOhm,
void encode_package(uint8_t* payload, int32_t temperature, 
  int32_t humidity, int32_t pressure, int32_t gas, float lux, int co2ppm)
{
  int32_t decode_add, to_encode;
  float decode_scale, org_scale, scale_to_apply;

  float test_value = -18.32;
  org_scale = 100; //temperature is in deci degrees
  decode_scale = 0.009999999776482582;
  decode_add = -20;
  scale_to_apply = org_scale/decode_scale;
  to_encode = (uint32_t)(temperature/scale_to_apply - decode_add/decode_scale); 
  //value to encode, payload, package_offset_bits, length_bits
  //encode(to_encode, payload, 0, 13);
  
  test_value *= org_scale;
  to_encode = (uint32_t)((temperature-decode_add*org_scale)/(decode_scale*org_scale));
  encode(to_encode, payload, 0, 13);

  org_scale = 100; //temperature is in deci degrees
  decode_scale = 0.00800000037997961;
  decode_add = 0;
  to_encode = (uint32_t)(humidity/decode_scale*org_scale - decode_add/decode_scale); 
  //value to encode, payload, package_offset_bits, length_bits
  encode(to_encode, payload, 13, 14);


  org_scale = 100; //temperature is in deci degrees
  decode_scale = 0.18000000715255738;
  decode_add = 30000;
  to_encode = (uint32_t)(pressure/decode_scale*org_scale - decode_add/decode_scale); 
  //value to encode, payload, package_offset_bits, length_bits
  encode(to_encode, payload, 27, 19);


  org_scale = 100; //temperature is in deci degrees
  decode_scale = 5.0;
  decode_add = 5000.0;
  to_encode = (uint32_t)(gas/decode_scale*org_scale - decode_add/decode_scale); 
  //value to encode, payload, package_offset_bits, length_bits
  encode(to_encode, payload, 46, 18);


  org_scale = 1; //temperature is in deci degrees
  decode_scale = 0.10000000149011612;
  decode_add = 0.04500000178813934;
  to_encode = (uint32_t)(lux/decode_scale*org_scale - decode_add/decode_scale); 
  //value to encode, payload, package_offset_bits, length_bits
  encode(to_encode, payload, 64, 28);


  org_scale = 1; //temperature is in deci degrees
  decode_scale = 1;
  decode_add = 0;
  to_encode = (uint32_t)(co2ppm/decode_scale*org_scale - decode_add/decode_scale); 
  //value to encode, payload, package_offset_bits, length_bits
  encode(to_encode, payload, 92, 11);
}

// pressure in 1/100 pascal, humidity in 1/1000 percent, temperature in 1/100 degree, gas in 1/100 mOhm,
void print_values(uint8_t* payload, int32_t temperature, int32_t humidity, 
     int32_t pressure, int32_t gas, float lux, int co2ppm){

    Serial.print(temperature/100.0,2);                       // Temperature in deci-degrees
    #ifdef ESP32
      Serial.print(F("deg ")); // Esp32 compiler doesn't liked escaped string
    #else
      Serial.print(F("\xC2\xB0\C "));                          // Representation of the ° symbol
    #endif
    Serial.print(humidity/1000.0,2);                         // Humidity in milli-percent
    Serial.print(F("%Hum "));
    Serial.print(pressure/100.0,2);                          // Pressure in Pascals
    Serial.print(F("hPa "));
    Serial.print(gas/100.0,2);
    Serial.print(F("mOhm"));

    Serial.print(lux,2);                         
    Serial.print(F("lux "));
    Serial.print(co2ppm);                 
    Serial.println(F("co2 ppm "));   
}