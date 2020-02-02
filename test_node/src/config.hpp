#ifndef CONFIG
#define CONFIG
#include <stdint.h>

extern const int uS_TO_S_FACTOR;  /* Conversion factor for micro seconds to seconds */
extern const int TIME_TO_SLEEP; /* Time ESP32 will go to sleep (in seconds) */

extern const char* ACCESS_POINT_NAME;
extern const char* ACCESS_POINT_PASSW;
extern const int portal_timeout; //seconds

extern const int sleep_between_connect_attempts;
extern const int sleep_between_measurements;
extern const uint8_t interruptPin;

extern const int api_key_size;
//extern const uint16_t default_node_id = 0;

extern const char* rootCACertificate;
///////////////////////////////////

extern const uint16_t node_id;
extern const uint64_t key;
extern const char* ssid;     //  your network SSID (name)
extern const char* pass;  // your network password
extern const char* url_port;

#endif
