#include "read_sensors.h"


bool get_payload(uint8_t* payload){

  float t = 20.32;
  float h = 50.51;

  uint16_t encode_add = 20;
  uint16_t to_encode = (t + encode_add)*10;
  int package_offset_bits = 0;
  int length_bits = 10;
  encode(897, payload, package_offset_bits, length_bits);

  Serial.println("encoded_line:");
  for(int i=0; i<sensordata_length; i++){
    Serial.print(*(payload+i));
    Serial.print(",");
  }
  Serial.println("");

  encode_add = 0;
  to_encode = (h + encode_add)*10;
  package_offset_bits = 10;
  length_bits = 10;
  encode(to_encode, payload, package_offset_bits, length_bits);

  return true;
}
