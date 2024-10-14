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
        Status Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector);
    private:
        Status validateMandatoryKeys(const JsonObject json);
        Status validateMandatoryValues(const JsonObject json);
        Status validateModbusArea();
        Status validateBitIndex();
        Status validateAddressQuantity();
        Status validateNumericScale();
        Status validateNumericOffset();
        Status validateMappingRules();
        Status validateDataUnitOrders();
        Status validateDataTypes();
        Status validateFormatString();
    private:
        std::pair<Status, std::vector<dt_e>> processDataTypes(JsonArray dataTypes);
        std::pair<Status, std::vector<DataUnitOrder>> processDataUnitOrders(JsonVariant dataUnitOrders);
        // Status emplaceCIN(config::Base* cin, cin_vector* outVector);
    private:
        std::pair<Status, adtp_e> convertToAdressType(const uint8_t type);
        std::pair<Status, addr_u> convertToAddress(JsonVariant address);
        std::pair<Status, mb_area_e> convertToModbusArea(JsonVariant modbusArea);
        std::pair<Status, uint8_t> convertToBitIndex(JsonVariant bitIndex);
        std::pair<Status, uint8_t> convertToAddressQuantity(JsonVariant addressQuantity);
        std::pair<Status, scl_e> convertToNumericScale(JsonVariant numericScale);
        std::pair<Status, float> convertToNumericOffset(JsonVariant numericOffset);
        std::pair<Status, std::map<uint16_t, std::string>> convertToMappingRules(JsonObject mappingRules);
        std::pair<Status, ord_t> convertToDataUnitOrderType(const std::string& value);
        std::pair<Status, dt_e> convertToDataType(const uint8_t dataType);
        std::pair<Status, std::string> convertToFormatString(const JsonVariant formatString);
    private:
        std::string mNodeID;
        std::pair<Status, adtp_e> mAddressType;
        std::pair<Status, addr_u> mAddress;
        std::pair<Status, mb_area_e> mModbusArea;
        std::pair<Status, uint8_t> mBitIndex;
        std::pair<Status, uint8_t> mAddressQuantity;
        std::pair<Status, scl_e> mNumericScale;
        std::pair<Status, float> mNumericOffset;
        std::pair<Status, std::map<uint16_t, std::string>> mMappingRules;
        std::pair<Status, std::vector<DataUnitOrder>> mDataUnitOrders;
        std::pair<Status, std::vector<dt_e>> mDataTypes;
        std::pair<Status, std::string> mFormatString;
        std::string mUID;
        std::string mDisplayName;
        std::string mDisplayUnit;
        bool mIsEventType = false;
    private:        
        std::vector<fmt_spec_e> mVectorFormatSpecifier;
        const std::regex mPatternUID;
    };
}}