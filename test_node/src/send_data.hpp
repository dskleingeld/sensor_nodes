#ifndef SEND_DATA_H
#define SEND_DATA_H

#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "error.hpp"
#include "config.hpp"

Error post_payload(uint8_t* payload, const char* url_port, int sensordata_length);

#endif