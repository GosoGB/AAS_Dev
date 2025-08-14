/**
 * @file SubmodelsSerializer.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-14
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#include "../../Container/Container.hpp"
#include "../../Include/Converter.hpp"
#include "../Include/SubmodelsSerializer.hpp"



namespace muffin { namespace aas {

    
    psram::string SubmodelsSerializer::Encode(const Identifier& id)
    {
        Container* container = Container::GetInstance();
        const AssetAdministrationShell* aas = container->Get;
        if (aas == nullptr)
        {
            log_d("AAS Not found with shortId: '%s'", id.c_str());
            return psram::string();
        }

        psram::string output;
        JsonDocument doc = encode(*aas);
        serializeJson(doc, output);
        return output;
    }


    psram::string SubmodelsSerializer::EncodeAll()
    {
        ;
    }


    JsonDocument SubmodelsSerializer::encode(const Submodel& submodel)
    {
        ;
    }
}}