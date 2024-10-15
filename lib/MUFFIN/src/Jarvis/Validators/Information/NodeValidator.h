/**
 * @file NodeValidator.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Node 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2024-10-14
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <ArduinoJson.h>
#include <vector>
#include <regex>

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"
#include "Jarvis/Include/DataUnitOrder.h"
#include "Jarvis/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis {

    class NodeValidator
    {
    public:
        NodeValidator();
        virtual ~NodeValidator();
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        std::pair<rsc_e, std::string> Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector);
    private:
        rsc_e validateMandatoryKeys(const JsonObject json);
        rsc_e validateMandatoryValues(const JsonObject json);
        rsc_e validateModbusArea();
        rsc_e validateBitIndex();
        rsc_e validateAddressQuantity();
        rsc_e validateNumericScale();
        rsc_e validateNumericOffset();
        rsc_e validateMappingRules();
        rsc_e validateDataUnitOrders();
        rsc_e validateDataTypes();
        rsc_e validateFormatString();
    private:
        std::pair<rsc_e, std::vector<dt_e>> processDataTypes(JsonArray dataTypes);
        std::pair<rsc_e, std::vector<DataUnitOrder>> processDataUnitOrders(JsonVariant dataUnitOrders);
        // Status emplaceCIN(config::Base* cin, cin_vector* outVector);
    private:
        std::pair<rsc_e, adtp_e> convertToAdressType(const uint8_t type);
        std::pair<rsc_e, addr_u> convertToAddress(JsonVariant address);
        std::pair<rsc_e, mb_area_e> convertToModbusArea(JsonVariant modbusArea);
        std::pair<rsc_e, uint8_t> convertToBitIndex(JsonVariant bitIndex);
        std::pair<rsc_e, uint8_t> convertToAddressQuantity(JsonVariant addressQuantity);
        std::pair<rsc_e, scl_e> convertToNumericScale(JsonVariant numericScale);
        std::pair<rsc_e, float> convertToNumericOffset(JsonVariant numericOffset);
        std::pair<rsc_e, std::map<uint16_t, std::string>> convertToMappingRules(JsonObject mappingRules);
        std::pair<rsc_e, ord_t> convertToDataUnitOrderType(const std::string& value);
        std::pair<rsc_e, dt_e> convertToDataType(const uint8_t dataType);
        std::pair<rsc_e, std::string> convertToFormatString(const JsonVariant formatString);
    private:
        std::string mNodeID;
        std::pair<rsc_e, adtp_e> mAddressType;
        std::pair<rsc_e, addr_u> mAddress;
        std::pair<rsc_e, mb_area_e> mModbusArea;
        std::pair<rsc_e, uint8_t> mBitIndex;
        std::pair<rsc_e, uint8_t> mAddressQuantity;
        std::pair<rsc_e, scl_e> mNumericScale;
        std::pair<rsc_e, float> mNumericOffset;
        std::pair<rsc_e, std::map<uint16_t, std::string>> mMappingRules;
        std::pair<rsc_e, std::vector<DataUnitOrder>> mDataUnitOrders;
        std::pair<rsc_e, std::vector<dt_e>> mDataTypes;
        std::pair<rsc_e, std::string> mFormatString;
        std::string mUID;
        std::string mDisplayName;
        std::string mDisplayUnit;
        bool mIsEventType = false;
    private:        
        std::vector<fmt_spec_e> mVectorFormatSpecifier;
        const std::regex mPatternUID;
    };
}}