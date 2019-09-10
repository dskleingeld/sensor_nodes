#include <WiFi.h>

#include "read_sensors.hpp"
#include "send_data.hpp"
#include "config.hpp"

IPAddress dns(1, 1, 1, 1);  //Cloudfare dns  
Sensors sensors;

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

    WiFi.begin(ssid, pass);

    sensors.init().handle_error() ;
    sensors.configure().handle_error();
}

void loop() {
  
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  IPAddress primaryDNS(8, 8, 8, 8); //optional
  IPAddress secondaryDNS(8, 8, 4, 4); //optional
  auto local_ip = WiFi.localIP();
  auto gateway_ip = WiFi.gatewayIP();
  auto subnetmask = WiFi.subnetMask();
  WiFi.config(local_ip, gateway_ip, subnetmask,primaryDNS,secondaryDNS);

  uint8_t payload[sensordata_length+10] = {0}; //creates a zerod line, critical it is all zero valued
  memcpy(payload, &node_id, 2);
  memcpy(payload+2, &key, 8);
  
  read_to_package(sensors, payload+10).handle_error();
  Serial.println(sensordata_length);
  post_payload(payload, url_port, sensordata_length).handle_error();

  Error::log.update_server();
  //esp_sleep_enable_timer_wakeup(sleep_between_measurements);
  //esp_deep_sleep_start();
  Serial.println("did not go to deep sleep");
  delay(5000);
}