#ifndef ERROR_H
#define ERROR_H

#include <cstdint>
#include <array>
#include <Arduino.h>
#include <WiFi.h> // for esp_light_sleep_start()
#include <HTTPClient.h>

#include <WiFiManager.h> // for resetting wifimanager while in error handler
#include "wificonfig.hpp" //FIXME get this into header
#include "config.hpp"

extern volatile bool shouldReset;

class Log; 

class Error {
    public:
        enum Code : uint8_t {
            UNKNOWN = 0,

            CANT_OPEN_FILE_FOR_WRITING=1,
            CANT_WRITE_TO_FILE=2,
            CANT_OPEN_FILE_FOR_READING=3,
            FILE_HAS_INCORRECT_SIZE=4,
            READ_MORE_THEN_PARAMS=5,
            
            INVALID_SERVER_RESPONSE=6,

            CANT_FIND_BME680=20,
            CANT_CONFIGURE_MHZ19=21,
            MAX44009_LIB_ERROR=22,

            //unreported errors
            FILE_DOES_NOT_EXIST,
            INCORRECT_KEY_ID_STRING,
            NONE,
        }; 


        Error() = default;
        constexpr Error(Code anError) : value(anError){}

        operator Code() const { return value; }  // Allow switch and comparisons.
                                                    // note: Putting constexpr here causes
                                                    // clang to stop warning on incomplete
                                                    // case handling.
        explicit operator bool() = delete;        // Prevent usage: if(fruit)

        bool handle_error();
        bool is_err(){
            if (value == Code::NONE) {
                Serial.println("no error");
                return false;
            } else {
                add_to_log();
                return true;
            }
        };
        //constexpr bool IsYellow() const { return value == Banana; }
        static Log log; //static memb var: global between class instances

    private:
        Code value;
        void add_to_log();
        void handle_unrecoverable();
        void handle_possible_recoverable();
};

uint8_t add_fields(uint8_t* payload, Error::Code errorcode);

struct LogEntry {
    Error::Code error_code;
    bool logged_at_server;
};

class Log {
    public:
        void add_to_log(Error::Code error);
        void update_server();
    private:
        uint8_t current_len = 0;
        uint8_t next_pos = 0;
        std::array<LogEntry, 4> log;
};

void reset();

#endif