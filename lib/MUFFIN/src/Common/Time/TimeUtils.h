/**
 * @file TimeUtils.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 시간 정보를 생성, 관리, 교환하기 위한 기능을 선언
 * 
 * @date 2024-08-31
 * @version 0.0.1
 * 
 * @todo Need to check if timezone is valid at 
 * the interface between the fw and the cloud.
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
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

    namespace jarvis {
        enum class ServerNetworkInterfaceCardEnum;  // forward declaration
        typedef ServerNetworkInterfaceCardEnum snic_e; // typedef for convenience
    }
    Status SyncWithNTP(const jarvis::snic_e snic);
}