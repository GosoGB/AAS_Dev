/**
 * @file JARVIS.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 설정을 담당하는 JARVIS 클래스를 선언합니다.
 * 
 * @date 2025-01-21
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
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
#include "JARVIS/JARVIS.h"
#include "Validators/Validator.h"



namespace muffin {

    jvs::ValidationResult JARVIS::Validate(JsonDocument& json)
    {
        jvs::Validator validator;
        return validator.Inspect(json, &mMapCIN);
    }

    void JARVIS::Clear()
    {
        mMapCIN.clear();
    }
    
    std::map<jvs::cfg_key_e, std::vector<jvs::config::Base*>>::iterator JARVIS::begin()
    {
        return mMapCIN.begin();
    }

    std::map<jvs::cfg_key_e, std::vector<jvs::config::Base*>>::iterator JARVIS::end()
    {
        return mMapCIN.end();
    }

    std::map<jvs::cfg_key_e, std::vector<jvs::config::Base*>>::const_iterator JARVIS::begin() const
    {
        return mMapCIN.cbegin();
    }

    std::map<jvs::cfg_key_e, std::vector<jvs::config::Base*>>::const_iterator JARVIS::end() const
    {
        return mMapCIN.cend();
    }


    JARVIS* jarvis = nullptr;
}