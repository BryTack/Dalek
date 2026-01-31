#line 1 "D:\\Arduino\\Projects\\Daleks\\utils\\serialLogger.h"
#pragma once
#include <Arduino.h>
#include <stdarg.h>

enum class LogLevel {
    DEBUG = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3
};

class SerialLog {
public:
    // ---- Global singleton instance ----
    static SerialLog& instance() {
        static SerialLog inst;
        return inst;
    }

    // ---- Initialize Serial ----
    void begin(unsigned long baud = 115200) {
        if (!initialized) {
            Serial.begin(baud);
            initialized = true;
        }
    }

    // ---- Log level control ----
    void setLevel(LogLevel lvl) { minLevel = lvl; }
    LogLevel getLevel() const { return minLevel; }

    // ---- Timestamp control ----
    void setShowTime(bool enabled) { showTime = enabled; }
    bool getShowTime() const { return showTime; }

    // ---------- Simple log methods ----------
    void debug(const char* msg) { logInternal(LogLevel::DEBUG, "DEBUG", msg); }
    void info (const char* msg) { logInternal(LogLevel::INFO,  "INFO",  msg); }
    void warn (const char* msg) { logInternal(LogLevel::WARN,  "WARN",  msg); }
    void error(const char* msg) { logInternal(LogLevel::ERROR, "ERROR", msg); }

    // ---------- printf-style log methods ----------
    void debugf(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        logInternalV(LogLevel::DEBUG, "DEBUG", fmt, args);
        va_end(args);
    }

    void infof(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        logInternalV(LogLevel::INFO, "INFO", fmt, args);
        va_end(args);
    }

    void warnf(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        logInternalV(LogLevel::WARN, "WARN", fmt, args);
        va_end(args);
    }

    void errorf(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        logInternalV(LogLevel::ERROR, "ERROR", fmt, args);
        va_end(args);
    }

private:
    bool initialized = false;
    bool showTime    = false;               // timestamps OFF by default
    LogLevel minLevel = LogLevel::DEBUG;

    SerialLog() = default;                  // private constructor → singleton
    SerialLog(const SerialLog&) = delete;
    SerialLog& operator=(const SerialLog&) = delete;

    // ----- Core logging logic (non-variadic) -----
    void logInternal(LogLevel lvl, const char* lvlStr, const char* msg) {
        if (!initialized || lvl < minLevel) return;

        if (showTime) {
            Serial.print("[");
            Serial.print(millis());
            Serial.print(" ms]");
        }

        Serial.print("[");
        Serial.print(lvlStr);
        Serial.print("] ");
        Serial.println(msg);
    }

    // ----- Core logging logic (variadic via va_list) -----
    void logInternalV(LogLevel lvl, const char* lvlStr,
                      const char* fmt, va_list args) {
        if (!initialized || lvl < minLevel) return;

        char buffer[128];
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        logInternal(lvl, lvlStr, buffer);
    }
};
