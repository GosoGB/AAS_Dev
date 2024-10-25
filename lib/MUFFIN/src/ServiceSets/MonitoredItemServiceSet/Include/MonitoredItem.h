/**
 * @file MonitoredItem.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 노드, 변수, 속성, 이벤트를 모니터링 할 때 사용되는 MonitoredItem 클래스를 선언합니다.
 * 
 * @note MonitoredItem 개체에 대한 식별자는 부호없는 8비트 정수입니다. 이는 설계 단계에서 ESP32의
 *       자원을 고려했을 때 256개 미만의 개체만이 생성될 것이라는 가정 위에서 설계했기 때문입니다.
 * 
 * @todo CreateMonitoredItemService에서 256개 이상의 개체가 생성되지 않도록 검사하는 기능을 구현해야 합니다.
 * 
 * @date 2024-10-25
 * @version 0.0.1
 * 
 * @copyright Copyright (c) EdgecrBoss Inc. 2024
 */




#pragma once

#include <string>
#include <sys/_stdint.h>
#include <vector>

#include "Common/Status.h"



namespace muffin {

    typedef int64_t UtcTime;

    typedef enum class MonitoredItem_MonitoringMode_Enum
        : uint8_t
    {
        DISABLED   = 0,
        SAMPLING   = 1,
        REPORTING  = 2
    } monitoring_mode_e;

    typedef enum class DataChangeFilter_DataChangeTrigger_Enum
        : uint8_t
    {
        STATUS                  = 0,
        STATUS_VALUE            = 1,
        STATUS_VALUE_TIMESTAMP  = 2
    } data_chengetrigger_e;

    typedef enum class DataChangeFilter_DeadbandType_Enum
        : uint8_t
    {
        NONE      = 0,
        ABSOLUTE  = 1,
        PERCENT   = 2
    } deadband_type_e;

    typedef struct DataChangeFilter_Type
    {
        data_chengetrigger_e Trigger  : 8;
        deadband_type_e DeadbandType  : 8;
        double DeadbandValue;
    } data_change_filter_t;
    

    typedef enum class AggregateFilter_AggregateType_Enum
        : uint8_t
    {
        NONE      = 0,
        ABSOLUTE  = 1,
        PERCENT   = 2
    } aggregate_type_e;

    typedef struct AggregateFilter_Type
    {
        UtcTime StartTime;
        std::vector<std::string> AggregateType;
        double ProcessingInterval;
        // AggregateConfiguration;
    } aggregate_filter_t;



    class MonitoredItem
    {
    public:
        MonitoredItem(const uint8_t monitoredItemID);
        virtual ~MonitoredItem();
    public:
        Status SetSamplingInterval(const uint32_t intervalInMillis);
        /**
         * @todo 노드가 아니어도 변수, 속성, 이벤트를 모니터링 할 수도 있어야 하기 때문에 
         *       향후 매개변수로 <nodeID>가 아닌 <ReadValueID>를 받도록 수정해야 합니다.
         */
        Status SetItemToMonitor(const std::string& nodeID);
        Status SetMonitoringMode(const monitoring_mode_e monitoringMode);
    private:
        const uint8_t mMonitoredItemID;
        uint32_t mSamplingInterval;
        /**
         * @todo <std::string mNodeID>에서 <ReadValueID mReadValueID>로 변경해야 합니다.
         */
        std::string mNodeID;
        monitoring_mode_e mMonitoringMode;
        data_change_filter_t mDataChangeFilter;
        aggregate_filter_t mAggregateFilter;
        uint32_t mQueueSize;
        bool mDiscardOldest = true;
    };
}


/**
 * @brief Client-defined entity in the Server used to monitor Attributes or EventNotifiers 
 *        for new values or Event occurrences and that generates Notifications for them
 * 
 * @brief MonitoredItems are entities in the Server created by the Client that 
 *        monitor AddressSpace Nodes and, indirectly, their real-world counterparts. 
 *        When they detect a data change or an event/alarm occurrence, they generate 
 *        a Notification that is transferred to the Client by a Subscription.
 */