#include "read_sensors.h"



bool get_payload(uint8_t* payload){

    uint8_t encoded[8] = {0,0,0,0,0,0,0,0};

    uint16_t decode_add = 40;

	uint16_t test_value = (10 + decode_add)*10;
    int package_offset_bits = 37;
    int length_bits = 10;
	encode(encoded, test_value, package_offset_bits, length_bits);

    return true;
}
