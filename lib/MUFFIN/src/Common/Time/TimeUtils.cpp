/**
 * @file TimeUtils.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 시간 정보를 생성, 관리, 교환하기 위한 기능을 정의
 * 
 * @date 2024-10-29
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include <cerrno>
#include <map>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Convert/ConvertClass.h"
#include "Jarvis/Include/TypeDefinitions.h"
#include "Network/CatM1/CatM1.h"
#include "TimeUtils.h"



namespace muffin {

    /**
     * @brief Map containing relations between timezone with  POSIX timezone string.
     */
    std::map<const std::string, const std::string> MapPTZS = {
        {"America/New_York", "EST5EDT,M3.2.0,M11.1.0"},     // 미국 동부 표준시
        {"America/Chicago", "CST6CDT,M3.2.0,M11.1.0"},      // 미국 중부 표준시
        {"America/Denver", "MST7MDT,M3.2.0,M11.1.0"},       // 미국 산지 표준시
        {"America/Los_Angeles", "PST8PDT,M3.2.0,M11.1.0"},  // 미국 태평양 표준시
        {"America/Anchorage", "AKST9AKDT,M3.2.0,M11.1.0"},  // 미국 알래스카 표준시
        {"Pacific/Honolulu", "HST10"},                      // 미국 하와이-알류샨 표준시
        {"Asia/Seoul", "KST-9"},                            // 대한민국 표준시
        {"Asia/Tokyo", "JST-9"}                             // 일본 표준시
    };

	Status SetSystemTime(const time_t& ts)
	{
		ASSERT(
			(
				(ts >= BUILD_TIME) || ((ts + 9 * 3600) >= BUILD_TIME)
			), "TIMESTAMP SMALLER THAN BUILD TIME: %u > %u", Convert.ToUInt32(ts), BUILD_TIME
		);
		
		struct timeval now = {
            .tv_sec = ts, 
            .tv_usec = 0
        };
		const int ret = settimeofday(&now, NULL);

		if (ret != 0)
		{
			switch (errno)
			{
				case EFAULT:
					LOG_ERROR(logger, "ONE OF 'tv' OR 'tz' POINTED OUTSIDE THE ACCESSIBLE ADDRESS SPACE");
					return Status(Status::Code::BAD_INVALID_ARGUMENT);
				case EINVAL:
					LOG_ERROR(logger, "TIMEZONE OR POSSIBLY SOMETHING ELSE IS INVALID");
					return Status(Status::Code::BAD_INVALID_ARGUMENT);
				case EPERM:
					LOG_ERROR(logger, "THE CALLING PROCESS HAS INSUFFICIENT PRIVILEGE");
					return Status(Status::Code::BAD_USER_ACCESS_DENIED);
				default:
					LOG_ERROR(logger, "UNKNOWN ERROR: \"errno\": %d", errno);
					return Status(Status::Code::UNCERTAIN);
			}
		}
		
		LOG_INFO(logger, "System time is set to %lu", now.tv_sec);
		return Status(Status::Code::GOOD);
	}

	Status SetTimezone(const std::string& tz)
	{
		ASSERT((MapPTZS.find(tz) != MapPTZS.end()), "UNDEFINED OR UNSUPPORTED TIMEZONE: %s", tz.c_str());

		const std::string pts = MapPTZS.at(tz); // POSIX timezone string
		const int isSet = setenv("TZ", pts.c_str(), 1);

		if (isSet != 0)
		{
			switch (errno)
			{
				case EINVAL:
					LOG_ERROR(logger, "NAME IS NULL, LENGTH IS 0, OR CONTAINING '='");
					return Status(Status::Code::BAD_INVALID_ARGUMENT);
				case ENOMEM:
					LOG_ERROR(logger, "INSUFFICIENT MEMORY TO ADD A NEW VARIABLE TO THE ENVIRONMENT");
					return Status(Status::Code::BAD_INVALID_ARGUMENT);
				default:
					LOG_ERROR(logger, "UNKNOWN ERROR: \"errno\": %d", errno);
					return Status(Status::Code::UNCERTAIN);
			}
		}
		tzset();

		LOG_INFO(logger, "Timezone is successfully set to \"%s\"", tz.c_str());
		return Status(Status::Code::GOOD);
	}

	time_t GetTimestamp()
	{
		return time(NULL);
	}

	time_t CalculateTimestampNextMinuteStarts(const time_t currentTimestamp)
	{
		const tm* localTime = localtime(&currentTimestamp);

		const int currentHour    = localTime->tm_hour;
		const int currentMinute  = localTime->tm_min;

		int nextHour    = currentHour;
		int nextMinute  = currentMinute + 1;

		if (nextMinute >= 60)
		{
			nextMinute = 0;
			++nextHour;

			if (nextHour >= 24)
			{
				nextHour = 0;
			}
		}

		tm nextLocalTime = *localTime;
		nextLocalTime.tm_hour = nextHour;
		nextLocalTime.tm_min  = nextMinute;
		nextLocalTime.tm_sec  = 0;

		return mktime(&nextLocalTime);
	}

	uint64_t GetTimestampInMillis()
	{
		struct timeval now = {
			.tv_sec  = 0,
			.tv_usec = 0
		};

		gettimeofday(&now, NULL);
		uint64_t ts = 1000L*(uint64_t)now.tv_sec + (uint64_t)now.tv_usec/1000L;
		return ts;
	}

	uint64_t TimestampToExactHourKST()
	{
		uint64_t kstTimestamp = GetTimestampInMillis() + 9 * 3600 * 1000;
		uint64_t kstHour = kstTimestamp / (1000 * 60 * 60); 
		uint64_t exactHourTimestampKST = kstHour * (1000 * 60 * 60); 

		return exactHourTimestampKST - 9 * 3600 * 1000;
	}

	std::string GetDatetime()
	{
		return Convert2Datetime(GetTimestamp());
	}

	std::string Convert2Datetime(const time_t timestamp)
	{
		struct tm* localTime = localtime(&timestamp);
		char dateTime[20] = {0};
		strftime(dateTime, 20, "%Y-%m-%dT%H:%M:%S", localTime);
		return std::string(dateTime);
	}

    Status SyncWithNTP(const jarvis::snic_e snic)
	{
		switch (snic)
		{
		case jarvis::snic_e::LTE_CatM1:
			{
				CatM1& catM1 = CatM1::GetInstance();
				return catM1.SyncWithNTP();
			}
	#if defined(MODLINK_T2) || defined(MODLINK_B)
		case jarvis::snic_e::Ethernet,:
			{
				return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
			}
    #elif defined(MODLINK_B)
		case jarvis::snic_e::Ethernet,:
			{
				return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
			}
        case jarvis::snic_e::WiFi4,:
			return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);`
	#endif
		default:
			return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
		}
	}
}