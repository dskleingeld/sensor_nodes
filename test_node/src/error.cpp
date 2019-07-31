#include "error.hpp"

extern WiFiManager wm;

void Log::add_to_log(ErrorCode code){
    //nothing to do if not unique
    for(auto& entry: log){
        if (entry.error_code == code) return;
    }
    
    log[next_pos] = LogEntry {code, false};

    if (next_pos == 3){
        next_pos = 0;
    } else { 
        next_pos++;
    } 
}

void Log::update_server(){
    for(auto& entry: log){
        if (entry.logged_at_server == false) {
            //send something to server [TODO, waiting on server impl]
            entry.logged_at_server = true;
        }
    }
}

bool Error::handle_error(){

    switch(value) {
        case NONE :
            return true; 

        case CANT_FIND_BME680:
        case CANT_CONFIGURE_MHZ19:
        case MAX44009_LIB_ERROR:
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
            add_to_log();
            handle_possible_recoverable();
    };
    return false;
}

void Error::add_to_log(){
    Error::log.add_to_log((ErrorCode)value);
}

void Error::handle_unrecoverable(){
    Serial.println("unrecoverable error happend, blinking until reset");

    constexpr uint64_t sleep_duration_us = 0.5*1000*1000; //0.2 seconds in microseconds
    esp_sleep_enable_timer_wakeup(sleep_duration_us);
    pinMode(LED_BUILTIN, OUTPUT);

    while(true){ //can only exit via hardware reset/power toggle or hardware interrupt
        esp_light_sleep_start();
        digitalWrite(LED_BUILTIN, HIGH);
        esp_light_sleep_start();
        digitalWrite(LED_BUILTIN, LOW);

        if (shouldReset) {reset();}
    }
}

void Error::handle_possible_recoverable(){
    Serial.println("possible_recoverable happend, blinking then restarting");

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
        digitalWrite(LED_BUILTIN, HIGH);
        esp_light_sleep_start();
        digitalWrite(LED_BUILTIN, LOW);
    }
    ESP.restart();
}

void reset(){
  wm.erase(); //TODO fix that reset funct works.
  ESP.restart();
}