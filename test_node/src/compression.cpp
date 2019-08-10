#include "compression.h"

uint8_t div_up(uint8_t x, uint8_t y) {
	return (x+y-1)/y;
}

///MUST get a zerod line.
void encode(uint32_t to_encode, uint8_t* line, uint8_t bit_offset, uint8_t length) {
    //~ is the cpp bitwise NOT, when applied to any type it will turn it into
    //a int, even if int is not equal to uint8_t. thus we cast directly after
    //applying ~
    uint8_t start_mask = (uint8_t)~0 >> (bit_offset % 8);

    int start_byte = (bit_offset /8) ;
    int stop_byte = ((bit_offset+length) /8 ) ;

    *(line+start_byte) |= (to_encode ) & start_mask;
    int bits_written = 8-(bit_offset % 8);

    if(length > 8) {
        //decode middle bits, no masking needed
        //for byte in line[start_byte+1..stop_byte].iter_mut(){
        for(uint8_t* byte = line+start_byte+1; byte<line+stop_byte; byte++) {
		      *byte |= (to_encode >> bits_written) ;
		      bits_written += 8;
		  }
    }

    int used_bits = bit_offset+length  -stop_byte  *8;
    uint8_t stop_mask = ~((uint8_t)~0 >> used_bits);
    stop_byte = div_up(bit_offset+length, 8) ;//starts at 0
    *(line+stop_byte-1) |= (to_encode >> (bits_written-(8-used_bits)))  & stop_mask;
}
