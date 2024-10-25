/**
 * @file MonitoredItem.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 노드, 변수, 속성, 이벤트를 모니터링 할 때 사용되는 MonitoredItem 클래스를 정의합니다.
 * 
 * @date 2024-10-25
 * @version 0.0.1
 * 
 * @copyright Copyright (c) EdgecrBoss Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Convert/ConvertClass.h"
#include "MonitoredItem.h"



namespace muffin {

    MonitoredItem::MonitoredItem(const uint8_t monitoredItemID)
        : mMonitoredItemID(monitoredItemID)
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    MonitoredItem::~MonitoredItem()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }
    
}


// struct UA_MonitoredItem {
//     /* Status and Settings */
//     UA_MonitoringMode monitoringMode;
//     UA_TimestampsToReturn timestampsToReturn;
//     UA_Boolean registered;       /* Registered in the server / Subscription */
//     UA_DateTime triggeredUntil;  /* If the MonitoringMode is SAMPLING,
//                                   * triggering the MonitoredItem puts the latest
//                                   * Notification into the publishing queue (of
//                                   * the Subscription). In addition, the first
//                                   * new sample is also published (and not just
//                                   * sampled) if it occurs within the duration of
//                                   * one publishing cycle after the triggering. */
//     UA_MonitoringParameters parameters;

//     /* Sampling */
//     UA_MonitoredItemSamplingType samplingType;
//     union {
//         UA_UInt64 callbackId;
//         UA_MonitoredItem *nodeListNext; /* Event-Based: Attached to Node */
//         LIST_ENTRY(UA_MonitoredItem) samplingListEntry; /* Publish-interval: Linked in
//                                                          * Subscription */
//     } sampling;
//     UA_DataValue lastValue;

//     /* Triggering Links */
//     size_t triggeringLinksSize;
//     UA_UInt32 *triggeringLinks;

//     /* Notification Queue */
//     NotificationQueue queue;
//     size_t queueSize; /* This is the current size. See also the configured
//                        * (maximum) queueSize in the parameters. */
//     size_t eventOverflows; /* Separate counter for the queue. Can at most double
//                             * the queue size */
// };