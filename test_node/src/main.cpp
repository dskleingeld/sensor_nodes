#include <FS.h>          // this needs to be first, or it all crashes and burns...
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager using dev branch
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <HTTPClient.h>

#ifdef ESP32
  #include <SPIFFS.h>
#endif

#include <stdint.h>
#include <string>
#include <sstream>
#include <cstring>

#include "config.h"
#include "read_sensors.h"
#include "wificonfig.hpp"

//default custom static IP
char static_ip[16] = "10.0.1.56";
char static_gw[16] = "10.0.1.1";
char static_sn[16] = "255.255.255.0";

Key key;
UrlPort url_port;
NodeId node_id;

volatile bool shouldReset = false;

void IRAM_ATTR handleInterrupt() {
  detachInterrupt(digitalPinToInterrupt(interruptPin));
  shouldReset = true;
}

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setupSpiffs(){
  //clean FS, for testing
  //SPIFFS.format();
  //Serial.println("formatted file system");

  //read configuration from FS json
  Serial.println("mounting FS...");

  #define FORMAT_SPIFFS_IF_FAILED true
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    //SPIFFS.format(); //clean filesystem for testing
    Serial.println("failed to mount FS"); return;
  }
    //if (false) {
//should not be needed as we can get everything from the json doc
/*     if (SPIFFS.exists("/params.raw")) {
      File paramsFile = SPIFFS.open("/params.raw", FILE_READ);
      if (paramsFile) {
        size_t sizeofparams = paramsFile.size();
        if (sizeofparams == 10) {
          std::unique_ptr<char[]> buf(new char[sizeofparams]);
          paramsFile.readBytes(buf.get(), sizeofparams);

          memcpy(node_id_arr, buf.get(), 2);
          memcpy(key_arr, buf.get()+2, 8);

          Serial.println("node_id_arr: ");
          Serial.println(node_id_arr[0]);

          Serial.println("wrote things");
        } else { Serial.print("did not read params file, incorrect size: "); Serial.println(sizeofparams);}
        paramsFile.close();
      } else { Serial.print("could not open params file"); }
    } else { Serial.println("no params file");} */
  //end read
}

void reset(){
  WiFiManager wm;
  wm.erase();
  ESP.restart();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  setupSpiffs();
  load_params_from_FS(key, url_port, node_id);

  // WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;
  //wm.erase(); //FOR DEBUG ONLY

  //set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);

  // setup custom parameters
  //
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter url_port_param("url_port", "url port", "blabla", 100);
  WiFiManagerParameter key_and_id_param("api_key", "api key", "0:12345", 32);
  url_port.wifi_param = url_port_param;

  //add all your parameters here
  wm.addParameter(&url_port.wifi_param);
  wm.addParameter(&key_and_id_param);

  // set static ip
  IPAddress _ip,_gw,_sn;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);
  wm.setSTAStaticIPConfig(_ip, _gw, _sn);

  //reset settings - wipe credentials for testing
  //wm.resetSettings();


  //automatically connect using saved credentials if they exist
  //If connection fails it starts an access point with the specified name
  //here  "AutoConnectAP" if empty will auto generate basedcon chipid, if password is blank it will be anonymous
  //and goes into a blocking loop awaiting configuration
  wm.setConfigPortalTimeout(portal_timeout);
  if (!wm.autoConnect(ACCESS_POINT_NAME, ACCESS_POINT_PASSW)) {
    Serial.println("failed to connect and hit timeout, going to sleep");
    esp_sleep_enable_timer_wakeup(sleep_between_connect_attempts);
    esp_deep_sleep_start(); //https://github.com/SensorsIot/ESP32-Deep-Sleep
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected to wifi network");

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    get_params_from_portal(key, url_port, node_id, key_and_id_param);
    save_params_to_FS(key, url_port, node_id, key_and_id_param);
    
    shouldSaveConfig = false;
  }

  //enable reset interupt
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);

  Serial.println("setup completed");
}



void loop() {
  
  uint8_t payload[sensordata_length+10];
  memset (payload, 0, sensordata_length+10);

  Serial.println("node_id_arr: ");
  for(int i=0; i<2; i++){
    Serial.print(*(node_id.array+i));
    Serial.print(", ");
  }
  Serial.println("");

  Serial.println("key_arr: ");
  for(int i=0; i<8; i++){
    Serial.print(*(key.array+i));
    Serial.print(", ");
  }
  Serial.println("");

  uint8_t node_id_arr[2] = {1,0};
  memcpy(payload, node_id_arr, 2);
  memcpy(payload+2, key.array, 8);
  get_payload(payload+10);

  HTTPClient https;

  //TODO re-enable
  //std::string url ("https://www.");
  //url.append(url_port);
  //url.append("/newdata");

  std::string url("https://www.deviousd.duckdns.org:8080/newdata");
  Serial.println(url.c_str() );
  Serial.println(url_port.str );

  Serial.println("payload: ");
  for(int i=0; i<sensordata_length+10; i++){
    Serial.print(*(payload+i));
    Serial.print(", ");
  }
  Serial.println("");

  //https.begin("http://example.com/index.html");
  //
  https.begin(url.c_str()); //Specify the URL and certificate
  int httpCode = https.POST(payload, sensordata_length+10);  // start connection and send Post
  https.end();//TODO check if needed?

  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String payload = https.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
  }

  if (shouldReset) {reset();}
  esp_sleep_enable_timer_wakeup(sleep_between_measurements);
  esp_deep_sleep_start();
  Serial.println("should never print");
  delay(5000);
}
