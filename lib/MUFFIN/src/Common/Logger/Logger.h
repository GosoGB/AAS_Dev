/**
 * @file Logger.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 로그 메시지를 생성하고 관리하는 클래스를 선언합니다.
 * 
 * @date 2024-08-31
 * @version 0.0.1
 * 
 * @todo 시리얼 모니터 외의 sink로 전송하는 기능 구현
 * @todo ATmega2560 칩셋 대상 기능 구현
 * 
 * @copyright Copyright Edgecross Inc. (c) 2023-2024
 */




#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <set>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/_stdint.h>
#include <string>



#define LOG_ERROR(_logger, fmt, ...)                \
    if (_logger != nullptr)                         \
    {                                               \
        (_logger)->Log(                             \
            muffin::log_level_e::LOG_LEVEL_ERROR,   \
            __COUNTER__,                            \
            __FILE__,                               \
            __FUNCTION__,                           \
            __LINE__, fmt,                          \
            ##__VA_ARGS__);                         \
    }

#define LOG_WARNING(_logger, fmt, ...)              \
    if (_logger != nullptr)                         \
    {                                               \
        (_logger)->Log(                             \
            muffin::log_level_e::LOG_LEVEL_WARNING, \
            __COUNTER__,                            \
            __FILE__,                               \
            __FUNCTION__,                           \
            __LINE__, fmt,                          \
            ##__VA_ARGS__);                         \
    }

#define LOG_INFO(_logger, fmt, ...)                 \
    if (_logger != nullptr)                         \
    {                                               \
        (_logger)->Log(                             \
            muffin::log_level_e::LOG_LEVEL_INFO,    \
            __COUNTER__,                            \
            __FILE__,                               \
            __FUNCTION__,                           \
            __LINE__, fmt,                          \
            ##__VA_ARGS__);                         \
    }

#define LOG_DEBUG(_logger, fmt, ...)                \
    if (_logger != nullptr)                         \
    {                                               \
        (_logger)->Log(                             \
            muffin::log_level_e::LOG_LEVEL_DEBUG,   \
            __COUNTER__,                            \
            __FILE__,                               \
            __FUNCTION__,                           \
            __LINE__, fmt,                          \
            ##__VA_ARGS__);                         \
    }

#define LOG_VERBOSE(_logger, fmt, ...)              \
    if (_logger != nullptr)                         \
    {                                               \
        (_logger)->Log(                             \
            muffin::log_level_e::LOG_LEVEL_VERBOSE, \
            __COUNTER__,                            \
            __FILE__,                               \
            __FUNCTION__,                           \
            __LINE__, fmt,                          \
            ##__VA_ARGS__);                         \
    }



namespace muffin {

    typedef enum class MuffinLogLevelEnum
        : uint8_t
    {
        LOG_LEVEL_ERROR     = 0,
        LOG_LEVEL_WARNING   = 1,
        LOG_LEVEL_INFO      = 2,
        LOG_LEVEL_DEBUG     = 3,
        LOG_LEVEL_VERBOSE   = 4
    } log_level_e;

    typedef enum class MuffinLogSinkEnum
        : uint8_t
    {
        LOG_TO_SERIAL_MONITOR,
    #if defined(ESP32)
        LOG_TO_MQTT_BROKER,
        LOG_TO_SPIFFS,
        LOG_TO_MICROSD,
    #elif defined(__AVR__) || defined(__avr__)
        LOG_TO_ESP32
    #endif
    } log_sink_e;



    class Logger
    {
    public:
        Logger();
        explicit Logger(const log_level_e level);
        virtual ~Logger();
    public:
        bool GetFilePathVerbosity() const;
        void SetFilePathVerbosity(const bool isFilePathVerbose);
    public:
        log_level_e GetLevel() const;
        void SetLevel(const log_level_e& level);
    public:
        std::set<log_sink_e> GetSink() const;
        void SetSink(const std::set<log_sink_e>& sinkSet);
        void AddSinkElement(const log_sink_e sink);
        void RemoveSinkElement(const log_sink_e sink);
    public:
        void Log(const log_level_e level, const size_t counter, const char* file, const char* func, const size_t line, const char* fmt, ...);

    private:
        SemaphoreHandle_t xSemaphore;
        static const uint32_t mBaudRate = 115200;
        bool mIsFilePathVerbose = false;
    #if defined(DEBUG)
        log_level_e mLevel = log_level_e::LOG_LEVEL_VERBOSE;
    #else
        log_level_e mLevel = log_level_e::LOG_LEVEL_INFO;
    #endif

    #if defined(__AVR__) || defined(__avr__)
        std::set<log_sink_e> mSink = {
            log_sink_e::LOG_TO_SERIAL_MONITOR, 
            log_sink_e::LOG_TO_MCU_ESPRESSIF32 
        };
    #else
        std::set<log_sink_e> mSinkSet = {
            log_sink_e::LOG_TO_SERIAL_MONITOR
        };
    #endif
        const char* mLevelString[5] = {
            "ERROR", 
            "WARNING", 
            "INFO", 
            "DEBUG", 
            "VERBOSE"
        };
        const char* mColorString[5] = {
            "\033[31m", 
            "\033[38;5;208m", 
            "\033[34m", 
            "\033[32m", 
            "\033[38;5;232m"
        };
    };


    extern Logger* logger;
}