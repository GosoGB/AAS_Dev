/**
 * @file Jarvis.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 설정을 담당하는 JARVIS 클래스를 선언합니다.
 * 
 * @date 2024-10-15
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"

#include "Config/Information/Alarm.h"
#include "Config/Information/Node.h"
#include "Config/Information/OperationTime.h"
#include "Config/Information/Production.h"
#include "Config/Interfaces/Rs232.h"
#include "Config/Interfaces/Rs485.h"
#include "Config/Network/CatM1.h"
#include "Config/Network/Ethernet.h"
#include "Config/Network/WiFi4.h"
#include "Config/Operation/Operation.h"
#include "Config/Protocol/ModbusRTU.h"
#include "Config/Protocol/ModbusTCP.h"
#include "Include/Helper.h"
#include "Jarvis.h"
#include "Validators/Validator.h"



namespace muffin {
        
    Jarvis::Jarvis()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    Jarvis::~Jarvis()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    jarvis::ValidationResult Jarvis::Validate(JsonDocument& json)
    {
        jarvis::Validator validator;
        return validator.Inspect(json, &mMapCIN);
    }
}