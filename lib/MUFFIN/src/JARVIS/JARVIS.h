/**
 * @file JARVIS.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 설정을 담당하는 JARVIS 클래스를 선언합니다.
 * 
 * @date 2025-01-21
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <ArduinoJson.h>
#include <map>
#include <vector>

#include "Common/Status.h"
#include "JARVIS/Include/Base.h"
#include "JARVIS/Include/TypeDefinitions.h"
#include "JARVIS/Validators/ValidationResult.h"



namespace muffin {
    
    class JARVIS
    {
    public:
        JARVIS() {}
        virtual ~JARVIS() {}    
    public:
        jvs::ValidationResult Validate(JsonDocument& json);
        void Clear();
    public:
        std::map<jvs::cfg_key_e, std::vector<jvs::config::Base*>>::iterator begin();
        std::map<jvs::cfg_key_e, std::vector<jvs::config::Base*>>::iterator end();
        std::map<jvs::cfg_key_e, std::vector<jvs::config::Base*>>::const_iterator begin() const;
        std::map<jvs::cfg_key_e, std::vector<jvs::config::Base*>>::const_iterator end() const;
    private:
        using vectorCIN = std::vector<jvs::config::Base*>;
        std::map<jvs::cfg_key_e, vectorCIN> mMapCIN; // CIN stands for "config instance"
    };


    extern JARVIS* jarvis;
}