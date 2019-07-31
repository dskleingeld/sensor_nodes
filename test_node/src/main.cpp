#include <FS.h>          // this needs to be first, or it all crashes and burns...
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include "wificonfig.hpp"
#include "read_sensors.hpp"
#include "send_data.hpp"
#include "config.hpp"

#ifdef ESP32
  #include <SPIFFS.h>
#endif

volatile bool shouldReset = false;
Sensors sensors;

WiFiManager wm;
Params params;

WiFiManagerParameter key_and_id_param("api_key", "api key", "0:00000000", 32);
WiFiManagerParameter url_port_param("url_port", "url and port", "server.com:8080", 80);

void saveParamsCallback();
void reset();
void IRAM_ATTR handleInterrupt();

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

    sensors.init().handle_error() ;
    sensors.configure().handle_error();

    #define FORMAT_SPIFFS_IF_FAILED true
    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
      //SPIFFS.format(); //cleans filesystem for testing
      Serial.println("failed to mount FS"); return;
    }

    //load params
    if (load_params_from_FS(params).is_err() ){
      set_params_for_portal(params, key_and_id_param, url_port_param);
    }

    //reset settings - wipe credentials for testing
    //wm.resetSettings();
    wm.addParameter(&key_and_id_param);
    wm.addParameter(&url_port_param);
    wm.setConfigPortalBlocking(true);
    wm.setSaveParamsCallback(saveParamsCallback);

    //automatically connect using saved credentials if they exist
    //If connection fails it starts an access point with the specified name
    if(wm.autoConnect("AutoConnectAP")){
        Serial.println("connected...yeey :)");
    }
    else {
        Serial.println("failed to connect and hit timeout");
        //TODO go deep sleep for a while.
    }

    //enable reset interupt
    pinMode(interruptPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);

}

void loop() {
  uint8_t payload[sensordata_length+10];
  memcpy(payload, &params.node_id, 2);
  memcpy(payload+2, &params.key, 8);
  
  read_to_package(sensors, payload+10).handle_error();
  post_payload(payload, params.url_port, sensordata_length).handle_error();

  Error::log.update_server();
  if (shouldReset) {reset();}

  //esp_sleep_enable_timer_wakeup(sleep_between_measurements);
  //esp_deep_sleep_start();
  Serial.println("did not go to deep sleep");
  delay(5000);
}

void IRAM_ATTR handleInterrupt() {
  detachInterrupt(digitalPinToInterrupt(interruptPin));
  shouldReset = true;
}

void saveParamsCallback () {
  get_params_from_portal(params, key_and_id_param, url_port_param).handle_error();
  save_params_to_FS(params).handle_error();
}