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

//define your default values here, if there are different values in config.json, they are overwritten.
char url_port[100]; //domain + port, www.example.org:8080 or www.example.org
char key_str[32];
char node_id_str[32];

uint8_t key_arr[8];
uint8_t node_id_arr[2];

//default custom static IP
char static_ip[16] = "10.0.1.56";
char static_gw[16] = "10.0.1.1";
char static_sn[16] = "255.255.255.0";

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

  if (SPIFFS.begin()) {
    //SPIFFS.format(); //clean filesystem for testing
    Serial.println("mounted file system");

    //if (false) {
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          Serial.println("\nAARRRGGHHH SEGFAULTSAAAHHH");

          if(json["ip"]) {

            strcpy(url_port, json["url_port"]);
            strcpy(key_str, json["key_str"]);
            strcpy(node_id_str, json["node_id_str"]);

            Serial.println("setting custom ip from config");
            strcpy(static_ip, json["ip"]);
            strcpy(static_gw, json["gateway"]);
            strcpy(static_sn, json["subnet"]);
            Serial.println(static_ip);
          } else {
            Serial.println("no custom ip in config");
          }

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
    if (SPIFFS.exists("/params.raw")) {
      File params = SPIFFS.open("params.raw");
      size_t size = params.size();
      if (size == 10) {
        std::unique_ptr<char[]> buf(new char[size]);
        params.readBytes(buf.get(), size);

        memcpy(node_id_arr, buf.get(), 2);
        memcpy(key_arr, buf.get(), 8);
        Serial.println("wrote things");
      }
      Serial.println("did not write");
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
}

void reset(){
  WiFiManager wm;
  wm.erase();
  ESP.restart();
}

void SerializeUint64(uint8_t* buf, uint64_t uval) {
    *(buf+0) = uval;
    *(buf+1) = uval >> 8;
    *(buf+2) = uval >> 16;
    *(buf+3) = uval >> 24;
    *(buf+4) = uval >> 32;
    *(buf+5) = uval >> 40;
    *(buf+6) = uval >> 48;
    *(buf+7) = uval >> 56;
}

void SerializeUint16(uint8_t* buf, uint16_t uval) {
    *(buf+0) = uval;
    *(buf+1) = uval >> 8;
}

void SerialiseNodeIdKey() {
  uint64_t key;
  uint16_t node_id;

  std::stringstream key_ss;
  key_ss << key_str;
  key_ss >> key;

  std::stringstream node_id_ss;
  node_id_ss << node_id_str;
  node_id_ss >> node_id;

  Serial.println("saving config");
  uint8_t key_arr[8];
  uint8_t node_id_arr[2];
  SerializeUint16(node_id_arr, node_id);
  SerializeUint64(key_arr, key);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  setupSpiffs();

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
  WiFiManagerParameter custom_url_port("url_port", "url port", "blabla", 100);
  WiFiManagerParameter custom_node_id("node_id", "node id", "hihihi", 32);
  WiFiManagerParameter custom_key("key", "key", "lolz", 32);

  //add all your parameters here
  wm.addParameter(&custom_url_port);
  wm.addParameter(&custom_node_id);
  wm.addParameter(&custom_key);

  // set static ip
  IPAddress _ip,_gw,_sn;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);
  //wm.setSTAStaticIPConfig(_ip, _gw, _sn);

  //reset settings - wipe credentials for testing
  //wm.resetSettings();

  //automatically connect using saved credentials if they exist
  //If connection fails it starts an access point with the specified name
  //here  "AutoConnectAP" if empty will auto generate basedcon chipid, if password is blank it will be anonymous
  //and goes into a blocking loop awaiting configuration
  wm.setConfigPortalTimeout(portal_timeout);
  if (!wm.autoConnect(access_point_name, access_point_password)) {
    Serial.println("failed to connect and hit timeout, going to sleep");
    esp_sleep_enable_timer_wakeup(sleep_between_connect_attempts);
    esp_deep_sleep_start(); //https://github.com/SensorsIot/ESP32-Deep-Sleep
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    //read updated parameters
    strcpy(url_port, custom_url_port.getValue());
    strcpy(key_str, custom_key.getValue());
    strcpy(node_id_str, custom_node_id.getValue());


    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["url_port"]    = url_port;
    json["key_str"]     = key_str;
    json["node_id_str"] = node_id_str;

    json["ip"]      = WiFi.localIP().toString();
    json["gateway"] = WiFi.gatewayIP().toString();
    json["subnet"]  = WiFi.subnetMask().toString();

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();

    File params = SPIFFS.open("/params.raw", "w");
    if (!params) {
      Serial.println("failed to open config file for writing");
    }
    params.write(node_id_arr, 2);
    params.write(key_arr, 8);

    //end save
    shouldSaveConfig = false;
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.subnetMask());


  //enable reset interupt
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);

  Serial.println("hi");
}



void loop() {
  Serial.println("hello");
  uint8_t payload[sensordata_length+10];

  memcpy(payload, node_id_arr, 2);
  memcpy(payload, key_arr, 8);
  get_payload(payload+10);

  HTTPClient https;

  std::string url ("https://www.");
  url.append(url_port);
  url.append("/newdata");
  Serial.println(url.c_str() );
  Serial.println(url_port );

  //https.begin("http://example.com/index.html");
  //
  https.begin(url.c_str()); //Specify the URL and certificate
  // start connection and send Post
  int httpCode = https.POST(payload, sensordata_length+10);
  //int httpCode = https.GET();

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

  Serial.println("done http Post");
  // put your main code here, to run repeatedly:
  esp_sleep_enable_timer_wakeup(sleep_between_measurements);
  //esp_deep_sleep_start();
  Serial.println("should never print");
  delay(5000);
}
