#include <FS.h>          // this needs to be first, or it all crashes and burns...
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include "wificonfig.hpp"

#ifdef ESP32
  #include <SPIFFS.h>
#endif

WiFiManager wm;
Params params;

WiFiManagerParameter key_and_id_param("api_key", "api key", "0:00000000", 32);
WiFiManagerParameter url_port_param("url_port", "url and port", "server.com:8080", 80);


void saveParamsCallback();

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

    //TODO start sensor reading;
    #define FORMAT_SPIFFS_IF_FAILED true
    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
      //SPIFFS.format(); //cleans filesystem for testing
      Serial.println("failed to mount FS"); return;
    }

    //load params
    if (load_params_from_FS(params)){
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
}

void loop() {
  Serial.println("hi");
  // if outdated update sensor data
  // (data is outdated on first loop)

    // put your main code here, to run repeatedly:
  delay(100);
}

void saveParamsCallback () {
  get_params_from_portal(params, key_and_id_param, url_port_param);
  save_params_to_FS(params);
}