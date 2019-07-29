#ifndef WIFICONFIG_H
#define WIFICONFIG_H

#include <cstring>
#include <sstream>

#include <Arduino.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

#ifdef ESP32
  #include <SPIFFS.h>
#endif

class Key {
    public:
        uint8_t array[8];
        char str[32];
        void update_array();
    private:
        void serialise(uint8_t* buf, uint64_t val);
};

class NodeId {
    public:
        uint8_t array[2];
        char str[32];
        void update_array();
    private:
        void serialise(uint8_t* buf, uint16_t val);
};

/////////////////////////

struct UrlPort {
    char str[100];
    WiFiManagerParameter wifi_param;
};


void get_params_from_portal(Key key, UrlPort url_port, NodeId node_id, WiFiManagerParameter key_and_id);
bool save_params_to_FS(Key key, UrlPort url_port, NodeId node_id, WiFiManagerParameter key_and_id);
bool load_params_from_FS(Key key, UrlPort url_port, NodeId node_id);
#endif