#include "send_data.hpp"


/// take care to explicitly call the desttructor of HTTPClient
/// before the WiFiClientSecure is deleted
Error post_payload(uint8_t* payload, const char* url_port, int sensordata_length){

  WiFiClientSecure *client = new WiFiClientSecure;
  if (!client){ 
    Serial.println("Can not create wifi_client_secure");
    delete client;
    return Error::CAN_NOT_CREATE_CLIENT;  
  }
  client -> setCACert(rootCACertificate);

  HTTPClient https;

  std::string url ("https://www.");
  url.append(url_port);
  url.append("/post_data");

  Serial.println(url_port);
  Serial.println(url.c_str());

  bool ok = https.begin(*client, url.c_str()); // start connection
  if (!ok) {
    Serial.println("could not connect to https server");
    https.~HTTPClient(); //explicit destructor call
    delete client;
    return Error::CAN_NOT_CONNECT_TO_SERVER;
  }
  
  //send Post
  int httpCode = https.POST(payload, sensordata_length+api_key_size);
  https.end();//TODO check if needed?

  if (httpCode <= 0) {
    Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    https.end();
    https.~HTTPClient(); //explicit destructor call
    delete client;
    return Error::INVALID_SERVER_RESPONSE;
  } 
  //HTTP header has been send and Server response header has been handled
  Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
  
  https.~HTTPClient(); //explicit destructor call
  delete client;
  return Error::NONE;
}