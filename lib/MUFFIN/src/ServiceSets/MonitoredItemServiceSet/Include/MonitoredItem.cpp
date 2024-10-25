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