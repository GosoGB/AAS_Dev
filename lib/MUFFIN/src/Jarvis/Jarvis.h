/**
 * @file Jarvis.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MODLINK 설정을 담당하는 JARVIS 클래스를 선언합니다.
 * 
 * @date 2024-10-15
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <ArduinoJson.h>
#include <map>
#include <vector>

#include "Common/Status.h"
#include "Include/Base.h"
#include "Include/TypeDefinitions.h"
#include "Validators/ValidationResult.h"



namespace muffin {
    
    class Jarvis
    {
    public:
        Jarvis(Jarvis const&) = delete;
        void operator=(Jarvis const&) = delete;
        static Jarvis* GetInstanceOrCrash();
        static Jarvis& GetInstance();
    private:
        Jarvis();
        virtual ~Jarvis();
    private:
        static Jarvis* mInstance;
    
    public:
        jarvis::ValidationResult Validate(JsonDocument& json);
        void Clear();
    public:
        std::map<jarvis::cfg_key_e, std::vector<jarvis::config::Base*>>::iterator begin();
        std::map<jarvis::cfg_key_e, std::vector<jarvis::config::Base*>>::iterator end();
        std::map<jarvis::cfg_key_e, std::vector<jarvis::config::Base*>>::const_iterator begin() const;
        std::map<jarvis::cfg_key_e, std::vector<jarvis::config::Base*>>::const_iterator end() const;
    private:
        using vectorCIN = std::vector<jarvis::config::Base*>;
        std::map<jarvis::cfg_key_e, vectorCIN> mMapCIN; // CIN stands for "config instance"
    };
}