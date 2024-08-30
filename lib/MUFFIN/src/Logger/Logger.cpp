/**
 * @file Logger.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 로그 메시지를 생성하고 관리하는 클래스를 정의합니다.
 * 
 * @date 2024-08-31
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2023-2024
 */




#include <sstream>
#include <HardwareSerial.h>

#include "GreetingMessage.h"
#include "Logger.h"
#include "Time/TimeUtils.h"



namespace muffin {

	Logger::Logger()
	{
		if (Serial == false)
		{
			Serial.begin(mBaudRate);
			vTaskDelay(10 / portTICK_PERIOD_MS);
			Serial.println(F(welcomAsciiArt));
		}
	}


	Logger::Logger(const log_level_e level)
		: mLevel(level)
	{
		if (Serial == false)
		{
			Serial.begin(mBaudRate);
			vTaskDelay(10 / portTICK_PERIOD_MS);
			Serial.println(F(welcomAsciiArt));
		}
	}


	Logger::~Logger()
	{
		LOG_DEBUG(logger, "Destroyed at address: %p", this);
	}


    log_level_e Logger::GetLevel() const
    {
        return mLevel;
    }


	void Logger::SetLevel(const log_level_e& level)
	{
		mLevel = level;
	}


    std::set Logger::GetSinkSet() const
    {
        return mSinkSet;
    }


	void Logger::SetSink(const std::set<log_sink_e>& sinkSet)
	{
		assert(sinkSet.size() > 0);
		mSinkSet = sinkSet;
	}


    void Logger::RemoveSinkElement(const log_sink_e sink)
    {
        mSinkSet.erase(sink);
    }


	void Logger::Log(const log_level_e level, const size_t counter, const char* file, const char* func, const size_t line, const char* fmt, ...)
	{
	    if (mLevel < level || mSinkSet.size() == 0)
    	{
        	return ;
    	}

		// send the log message to sink set
		for (const auto& sink : mSinkSet)
		{
			if (sink == log_sink_e::LOG_TO_SERIAL_MONITOR)
			{
            #if defined(ESP32)
                // generate the log message using arg list
                va_list args;
                va_start(args, fmt);
                int length = vsnprintf(nullptr, 0, fmt, args);
                std::string message(length + 1, '\0');
                vsnprintf(&message[0], length + 1, fmt, args);
                va_end(args);
		
				// generate a metadata string for the log
				std::stringstream metadata;
				metadata <<  mColorString[level]     <<
					"["  <<  mLevelString[level]     << "]"
					"["  <<  GetDatetime()   	     << "]"
					"["  <<  file << ":" << line 	 << "]"
					"["  <<  func << ":" << counter  << "] ";

				// combine the log message with metadata
				const char* log = metadata.str().append(message).c_str();

				// mush flush the txd buffer after writing
				Serial.println(log);
				Serial.flush();
            #endif
			}
		}
	}


	Logger* logger = nullptr;
}