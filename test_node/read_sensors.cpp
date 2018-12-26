#include "read_sensors.h"


bool get_payload(uint8_t* payload){

  float t = 20;
  float h = 50.01;

  uint16_t encode_add = 20;
  uint16_t to_encode = (t + encode_add)/1;
  int package_offset_bits = 0;
  int length_bits = 10;
  encode(to_encode, payload, package_offset_bits, length_bits);

  Serial.println("encoded_line:");
  for(int i=0; i<sensordata_length; i++){
    Serial.print(*(payload+1));
    Serial.print(",");
  }
  Serial.println("");

  //decode_add = 0;
  //to_encode = (h + decode_add)/0.10000000149011612;
  //package_offset_bits = 11;
  //length_bits = 10;
  //encode(payload, to_encode, package_offset_bits, length_bits);

  return true;
}
