#include "send_data.hpp"

Error post_payload(uint8_t* payload, char* url_port, int sensordata_length){

  HTTPClient https;

  std::string url ("https://www.");
  url.append(url_port);
  url.append("/post_data");

  Serial.println(url_port);
  Serial.println(url.c_str());

  https.begin(url.c_str()); //Specify the URL and certificate
  // start connection and send Post
  int httpCode = https.POST(payload, sensordata_length+api_key_size);

  https.end();//TODO check if needed?

  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
  } else {
    Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    return Error::INVALID_SERVER_RESPONSE;
  }

  return Error::NONE;
}