#ifndef COMPR_H
#define COMPR_H

#include <stdint.h>
#include <Arduino.h>

uint8_t div_up(uint8_t x, uint8_t y);

void encode(uint32_t to_encode, uint8_t line[], uint8_t bit_offset, uint8_t length);

#endif
