#ifndef WIFICONFIG_H
#define WIFICONFIG_H

#include "error.hpp"

#include <cstring>
#include <sstream>
#include <cstdio>
#include <stdio.h>

#include <Arduino.h>
#include <WiFiManager.h>

#ifdef ESP32
  #include <SPIFFS.h>
#endif

class Error; // forward declaration

uint64_t uint64_t_from_str(char* str);
uint16_t uint16_t_from_str(char* str);

class Params {
    public:
    uint16_t node_id;
    uint64_t key;
    char url_port[100];

    void get_node_id_str();
    void key_str();
};

//bool get_params_from_portal();
Error get_params_from_portal(Params &params, WiFiManagerParameter &key_and_id, WiFiManagerParameter &url_port_param);
void set_params_for_portal(Params &params, WiFiManagerParameter &key_and_id, WiFiManagerParameter &url_port_param);

Error save_params_to_FS(Params &params);
Error load_params_from_FS(Params &params);
#endif