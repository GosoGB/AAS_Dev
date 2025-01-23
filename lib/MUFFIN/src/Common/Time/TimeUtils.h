/**
 * @file TimeUtils.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 시간 정보를 생성, 관리, 교환하기 위한 기능을 선언
 * 
 * @date 2025-01-23
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 * 
 * @todo Need to check if timezone is valid at 
 * the interface between the fw and the cloud.
 */




#pragma once

#include <string>
#include <sys/_stdint.h>
#include <sys/time.h>

#include "Common/Status.h"



namespace muffin {

    /**
     * @brief Set the System Time object
     * @param ts timestamp
     * @return Status MUFFIN status code
     */
    Status SetSystemTime(const time_t& ts);

    /**
     * @brief Set the Timezone object
     * @param tz timezone
     * @return Status MUFFIN status code
     * 
     * @todo Need to check if timezone is valid 
     * at the interface between the firmware and
     * the cloud.
     */
    Status SetTimezone(const std::string& tz);

    /**
     * @brief Get the Timestamp object
     * @return time_t unix time
     */
    time_t GetTimestamp();

    /**
     * @brief Calculate timestamp when the next minute starts
     * 
     * @param currentTimestamp current timestamp
     * @return time_t unix time
     */
    time_t CalculateTimestampNextMinuteStarts(const time_t currentTimestamp);

    /**
     * @brief Get the Millis Timestamp object
     * @return uint64_t  unix time in miiliseconds
     */
    uint64_t GetTimestampInMillis();

    /**
     * @brief Get the start of the hour in KST (Korean Standard Time) in milliseconds.
     * @return uint64_t start of the hour unix time in miiliseconds
     */
    uint64_t TimestampToExactHourKST();

    /**
     * @brief Get the Datetime object
     * @return std::string datetime
     */
    std::string GetDatetime();

    /**
     * @brief Convert 'timestamp' into datetime string
     * @param timestamp unix time
     * @return std::string datetime
     */
    std::string Convert2Datetime(const time_t timestamp);

    namespace jvs {
        enum class ServerNetworkInterfaceCardEnum;  // forward declaration
        typedef ServerNetworkInterfaceCardEnum snic_e; // typedef for convenience
    }
    Status SyncWithNTP(const jvs::snic_e snic);
}