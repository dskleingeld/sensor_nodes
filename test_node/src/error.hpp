#ifndef ERROR_H
#define ERROR_H

#include <cstdint>
#include <array>
#include <Arduino.h>
#include <WiFi.h> // for esp_light_sleep_start()

#include <WiFiManager.h> // for resetting wifimanager while in error handler

extern volatile bool shouldReset;

enum ErrorCode : uint8_t {
    NONE,
    CANT_FIND_BME680,
    CANT_CONFIGURE_MHZ19,
    MAX44009_LIB_ERROR,

    INCORRECT_KEY_ID_STRING,
    CANT_OPEN_FILE_FOR_WRITING,
    CANT_WRITE_TO_FILE,
    FILE_DOES_NOT_EXIST,
    CANT_OPEN_FILE_FOR_READING,
    FILE_HAS_INCORRECT_SIZE,
    READ_MORE_THEN_PARAMS,
    
    INVALID_SERVER_RESPONSE,
}; 

struct LogEntry {
    ErrorCode error_code;
    bool logged_at_server;
};

class Log {
    public:
        void add_to_log(ErrorCode error);
        void update_server();
    private:
        uint8_t next_pos = 0;
        std::array<LogEntry, 4> log;
};

class Error {
    public:
        enum Value : uint8_t {
            NONE, 
            CANT_FIND_BME680,
            CANT_CONFIGURE_MHZ19,
            MAX44009_LIB_ERROR,

            INCORRECT_KEY_ID_STRING,
            CANT_OPEN_FILE_FOR_WRITING,
            CANT_WRITE_TO_FILE,
            FILE_DOES_NOT_EXIST,
            CANT_OPEN_FILE_FOR_READING,
            FILE_HAS_INCORRECT_SIZE,
            READ_MORE_THEN_PARAMS,
            
            INVALID_SERVER_RESPONSE,
        }; 


        Error() = default;
        constexpr Error(Value anError) : value(anError){}

        operator Value() const { return value; }  // Allow switch and comparisons.
                                                    // note: Putting constexpr here causes
                                                    // clang to stop warning on incomplete
                                                    // case handling.
        explicit operator bool() = delete;        // Prevent usage: if(fruit)

        bool handle_error();
        bool is_err(){
            return handle_error();
        };
        
        //constexpr bool IsYellow() const { return value == Banana; }
        static Log log; //static memb var: global between class instances

    private:
        Value value;
        void add_to_log();
        void handle_unrecoverable();
        void handle_possible_recoverable();
};

void reset();

#endif