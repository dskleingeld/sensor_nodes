#include "wificonfig.hpp"

//default custom static IP
extern char static_ip[16];
extern char static_gw[16];
extern char static_sn[16];

//bool get_params_from_portal() {
bool get_params_from_portal(Key &key, UrlPort &url_port, NodeId &node_id, WiFiManagerParameter &key_and_id) {

    char key_and_id_str[32];
    //read updated parameters
    strcpy(&url_port.str[0], url_port.wifi_param.getValue());
    strcpy(&key_and_id_str[0], key_and_id.getValue());

    
    char* split = strchr(key_and_id_str, ':'); //find where to split
    if(split==NULL){return false;}

    *split = '\0'; //add str end
    strcpy(key.str, split+1);
    strcpy(node_id.str, key_and_id_str);
    key.update_array();
    node_id.update_array();

    Serial.print("key, node_id: ");
    Serial.print(key.str);
    Serial.println(node_id.str);
    
}

bool save_params_to_FS(Key &key, UrlPort &url_port, NodeId &node_id, WiFiManagerParameter &key_and_id) {

    DynamicJsonDocument doc(1024);
    doc["url_port"]    = url_port.str;
    doc["key_str"]     = key.str;
    doc["node_id_str"] = node_id.str;

    doc["ip"]      = WiFi.localIP().toString();
    doc["gateway"] = WiFi.gatewayIP().toString();
    doc["subnet"]  = WiFi.subnetMask().toString();

    File configFile = SPIFFS.open("/config.json", FILE_WRITE);  //opens and truncates file
    if (!configFile) { Serial.println("failed to open config file for writing"); return false; }
    Serial.print("Json content: ");
    serializeJsonPretty(doc, Serial);
    serializeJson(doc, configFile);
    configFile.close();

    //should not be needed
    // File params = SPIFFS.open("/params.raw", FILE_WRITE); //opens and truncates file
    // if (!params) { Serial.println("failed to open params file for writing"); return false; }

    // //params.seek(0);
    // Serial.print("node_id_arr: ");
    // Serial.println(node_id.array[0]);
    // Serial.println(node_id.array[1]);
    // if (!params.write(node_id.array, 2)) {Serial.println("write failed!");}
    // params.write(key.array, 8);
    // params.flush();
    // params.close();

    // File params2 = SPIFFS.open("/params.raw", FILE_READ);
    // if (!params2) { Serial.println("failed to open params file for writing"); return false;} 
    // size_t size = params2.size();
    // Serial.print("params size: ");
    // Serial.println(size);
    // params2.close();
    return true;
}

DeserializationError deserialise_file(File file, DynamicJsonDocument &doc){
    size_t size = file.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    file.readBytes(buf.get(), size);
    
    //deserialise json
    auto error = deserializeJson(doc, (buf.get()));
    //Serial.print("file json:");
    //serializeJson(doc, Serial); //print to terminal
    return error;
}

bool load_params_from_FS(Key &key, UrlPort &url_port, NodeId &node_id){
    if (!SPIFFS.exists("/config.json")) { Serial.println("no existing config"); return false; }

    File config_file = SPIFFS.open("/config.json", "r");
    if (!config_file) { Serial.println("could not open existing config file"); return false; }

    DynamicJsonDocument doc(1024); 
    auto error = deserialise_file(config_file, doc);
    
    if (error) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
        return false;
    }

    const char* url_port_str =  doc["url_port"];
    const char* key_str =       doc["key_str"];
    const char* node_id_str =   doc["node_id_str"];
    const char* static_ip_str = doc["ip"];
    const char* static_gw_str = doc["gateway"];
    const char* static_sn_str = doc["subnet"];

    //check if the Json has no missing values
    if (url_port_str == nullptr || key_str == nullptr || 
        node_id_str == nullptr || static_ip_str == nullptr || 
        static_gw_str == nullptr || static_sn_str == nullptr) {
        
        Serial.println("FS params empty or corrupted"); return false;
    }

    strcpy(url_port.str, url_port_str);
    strcpy(key.str,      key_str);
    strcpy(node_id.str,  node_id_str);
    key.update_array();
    node_id.update_array();

    strcpy(static_ip, static_ip_str);
    strcpy(static_gw, static_gw_str);
    strcpy(static_sn, static_sn_str);
    Serial.println(static_ip);
    return true;
}

void Key::update_array(){
  uint64_t key;

  std::stringstream key_ss;
  key_ss << str;
  key_ss >> key;

  serialise(array, key);
}

void NodeId::update_array(){
  uint16_t key;

  std::stringstream node_id_ss;
  node_id_ss << str;
  node_id_ss >> key;

  serialise(array, key);
}

void Key::serialise(uint8_t* buf, uint64_t uval) {
    *(buf+0) = uval;
    *(buf+1) = uval >> 8;
    *(buf+2) = uval >> 16;
    *(buf+3) = uval >> 24;
    *(buf+4) = uval >> 32;
    *(buf+5) = uval >> 40;
    *(buf+6) = uval >> 48;
    *(buf+7) = uval >> 56;
}

void NodeId::serialise(uint8_t* buf, uint16_t uval) {
    *(buf+0) = uval;
    *(buf+1) = uval >> 8;
}