#include "error.hpp"

uint8_t add_fields(uint8_t* payload, Error::Code error){
    uint8_t* fields = payload+11;
    switch(error) {
        case Error::CANT_FIND_BME680:
            fields[0] = 0; 
            fields[1] = 1;
            fields[2] = 2;
            fields[2] = 3;
            return 4;
        case Error::CANT_CONFIGURE_MHZ19:
            fields[0] = 5; 
            return 1;
        case Error::MAX44009_LIB_ERROR:
            fields[0] = 4; 
            return 1;

        case Error::INCORRECT_KEY_ID_STRING:
        case Error::CANT_OPEN_FILE_FOR_WRITING:
        case Error::CANT_WRITE_TO_FILE:
        case Error::FILE_DOES_NOT_EXIST:
        case Error::CANT_OPEN_FILE_FOR_READING:
        case Error::FILE_HAS_INCORRECT_SIZE:
        case Error::READ_MORE_THEN_PARAMS:
        case Error::INVALID_SERVER_RESPONSE:
        case Error::UNKNOWN:
            fields[0] = UINT8_MAX;
            return 1;
            break;
        case Error::NONE :
            Serial.println("NONE ERROR SHOULD NEVER BE REPORTED");
            return 0; 
    }
    return 0;
}

void Log::add_to_log(Error::Code code){
    //nothing to do if not unique
    for(auto& entry: log){
        if (entry.error_code == code) return;
    }

    log[next_pos] = LogEntry {code, false};
    if (current_len < 4) current_len++;
    if (next_pos == 3){
        next_pos = 0;
    } else { 
        next_pos++;
    }
}

void Log::update_server(){
    for(int i=0; i<current_len; i++){
        auto entry = log[i];
        if (entry.logged_at_server == false) {
            //send something to server [TODO, waiting on server impl]
            std::string url ("https://www.");
            url.append(url_port);
            url.append("/post_error");

            uint8_t payload[10+10];
            memcpy(payload, &node_id, 2);
            memcpy(payload+2, &key, 8);
            payload[10] = entry.error_code;
            uint8_t error_length = add_fields(payload, entry.error_code);
            Serial.println(error_length);
            
            HTTPClient https;
            https.begin(url.c_str()); //Specify the URL and certificate
            // start connection and send Post           
            int httpCode = https.POST((uint8_t*)payload, 11+error_length);
            if (httpCode == 200) {
                entry.logged_at_server = true;
                Serial.println("reported error to server");
            } else {
                Serial.println("could not report error to sever");
            }
        }
    }
}

Log Error::log;

bool Error::handle_error(){

    switch(value) {
        case NONE :
            Serial.println("handling NONE");
            return true; 

        case CANT_FIND_BME680:
        case CANT_CONFIGURE_MHZ19:
        case MAX44009_LIB_ERROR:
            add_to_log();
        case INCORRECT_KEY_ID_STRING:
            handle_unrecoverable();
            break;

        case CANT_OPEN_FILE_FOR_WRITING:
        case CANT_WRITE_TO_FILE:
        case FILE_DOES_NOT_EXIST:
        case CANT_OPEN_FILE_FOR_READING:
        case FILE_HAS_INCORRECT_SIZE:
        case READ_MORE_THEN_PARAMS:
        case INVALID_SERVER_RESPONSE:
        case UNKNOWN:
            Serial.println("ADDING TO LOG");
            add_to_log();
            handle_possible_recoverable();
    };
    return false;
}

void Error::add_to_log(){
    Serial.print("error value: ");
    Serial.println(value);
    Error::log.add_to_log(value);
}

void Error::handle_unrecoverable(){
    Serial.println("unrecoverable error happend, blinking until reset");

    if (WiFi.status() == WL_CONNECTED) {
        Error::log.update_server();
    } else {
        Serial.println("could not log error to sever as wifi was not connected");
    }

    constexpr uint64_t sleep_duration_us = 0.5*1000*1000; //0.2 seconds in microseconds
    esp_sleep_enable_timer_wakeup(sleep_duration_us);
    pinMode(LED_BUILTIN, OUTPUT);

    while(true){ //can only exit via hardware reset/power toggle or hardware interrupt
        esp_light_sleep_start();
        //digitalWrite(LED_BUILTIN, HIGH);
        esp_light_sleep_start();
        //digitalWrite(LED_BUILTIN, LOW);
    }
}

void Error::handle_possible_recoverable(){
    Serial.println("possible_recoverable happend, blinking then restarting");
    Serial.flush();

    //TODO if log fails keep trying throughout sleep period
    if (WiFi.status() == WL_CONNECTED) {
        Error::log.update_server();
    }

    constexpr int blink_frequency_s = 1.5;
    constexpr uint64_t sleep_duration_us = blink_frequency_s*1000*1000; //0.2 seconds in microseconds
    constexpr int wait_time = 60; //wait time in seconds
    esp_sleep_enable_timer_wakeup(sleep_duration_us);
    pinMode(LED_BUILTIN, OUTPUT);

    for(int i=0; i<wait_time/(2*blink_frequency_s); i++){ //can only exit via hardware reset/power toggle or hardware interrupt
        esp_light_sleep_start();
        //digitalWrite(LED_BUILTIN, HIGH);
        esp_light_sleep_start();
        //digitalWrite(LED_BUILTIN, LOW);
    }
    ESP.restart();
}