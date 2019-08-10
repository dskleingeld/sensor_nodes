#include "wificonfig.hpp"

//default custom static IP
extern char static_ip[16];
extern char static_gw[16];
extern char static_sn[16];

//bool get_params_from_portal() {
Error get_params_from_portal(Params &params, WiFiManagerParameter &key_and_id, WiFiManagerParameter &url_port_param) {

    char key_and_id_str[32];
    //read updated parameters
    strcpy(&key_and_id_str[0], key_and_id.getValue());
    char* split = strchr(key_and_id_str, ':'); //find where to split
    if(split==NULL){return Error::INCORRECT_KEY_ID_STRING;}

    *split = '\0'; //add str end
    params.key = uint64_t_from_str(split+1);
    params.node_id = uint16_t_from_str(key_and_id_str);

    strcpy(&params.url_port[0], url_port_param.getValue());

    return Error::NONE;
}

void set_params_for_portal(Params &params, WiFiManagerParameter &key_and_id, WiFiManagerParameter &url_port_param){
    
    char api_id_str[32];
    sprintf(api_id_str, "%u:%.10llu", params.node_id, params.key);

    key_and_id = WiFiManagerParameter("api_key", "api key", api_id_str, 32);
    url_port_param = WiFiManagerParameter("url_port", "url and port", params.url_port, 100);
}


Error save_params_to_FS(Params &params) {
    Serial.println("Saving params");

    File param_file = SPIFFS.open("/params.raw", FILE_WRITE); //opens and truncates file
    if (!param_file) { Serial.println("failed to open params file for writing"); return Error::CANT_OPEN_FILE_FOR_WRITING; }

    if (!param_file.write((uint8_t*)&params, sizeof(struct Params))) {
        Serial.println("write failed!");
        param_file.close();
        return Error::CANT_WRITE_TO_FILE;
    }

    param_file.flush();
    param_file.close();
    return Error::NONE;
}

Error load_params_from_FS(Params &params){
    if (!SPIFFS.exists("/params.raw")) { Serial.println("no existing params"); return Error::FILE_DOES_NOT_EXIST; }

    File param_file = SPIFFS.open("/params.raw", FILE_READ);
    if (!param_file) { Serial.println("could not open existing params file"); return Error::CANT_OPEN_FILE_FOR_READING; }

    size_t length = param_file.size();
    if (length != sizeof(struct Params)) {  
        Serial.print("did not read params file, incorrect size: "); 
        Serial.println(length);
        Serial.print("correct size: ");
        Serial.println(sizeof(struct Params));
        param_file.close();
        return Error::FILE_HAS_INCORRECT_SIZE; 
    }

    std::unique_ptr<uint8_t[]> buf(new uint8_t[length]);
    auto read = param_file.read((uint8_t*)&params, sizeof(struct Params));
    param_file.close(); 
    
    if (read != sizeof(params)){ Serial.println("params corrupt"); return Error::READ_MORE_THEN_PARAMS; }

    Serial.println("read params file without problems");
    return Error::NONE;
}

uint64_t uint64_t_from_str(char* str){
  uint64_t numb;

  std::stringstream key_ss;
  key_ss << str;
  key_ss >> numb;

  return numb;
}

uint16_t uint16_t_from_str(char* str){
  uint16_t numb;

  std::stringstream node_id_ss;
  node_id_ss << str;
  node_id_ss >> numb;

  return numb;
}