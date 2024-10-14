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

/**
 * @brief std::pair<Status, mb_area_e> NodeValidator::convertToModbusArea(const adtp_e addressType, JsonVariant modbusArea)
 * 
 * @code {.language-id}
 * if (addressType != adtp_e::NUMERIC)
 * {
 *     LOG_ERROR(logger, "MODBUS AREA CANNOT BE SET WHEN ADDRESS TYPE IS NOT NUMERIC");
 *     return std::make_pair(Status(Status::Code::BAD_CONFIGURATION_ERROR), mb_area_e::COILS);
 * }
 * 
 * if (area == mb_area_e::COILS || area == mb_area_e::DISCRETE_INPUT)
 * {
 *     if (mVectorDataUnitOrder.size() != 0)
 *     {
 *         LOG_ERROR(logger, "MODBUS AREA CANNOT BE SET TO \"COILS\" OR \"DISCRETE INPUT\" WHEN DATA UNIT ORDER IS ENABLED");
 *         return Status(Status::Code::BAD_CONFIGURATION_ERROR);
 *     }
 *     
 *     if (mVectorDataType.size() != 1)
 *     {
 *         LOG_ERROR(logger, "MODBUS AREA CANNOT BE SET TO \"COILS\" OR \"DISCRETE INPUT\" WHEN MULTIPLE DATA TYPES ARE ENABLED");
 *         return Status(Status::Code::BAD_CONFIGURATION_ERROR);
 *     }
 *     
 *     if (mVectorDataType.front() != dt_e::BOOLEAN)
 *     {
 *         LOG_ERROR(logger, "MODBUS AREA CANNOT BE SET TO \"COILS\" OR \"DISCRETE INPUT\" WHEN DATA TYPE IS NOT BOOLEAN");
 *         return Status(Status::Code::BAD_CONFIGURATION_ERROR);
 *     }
 * }
 * @endcode
 */


/**
 * @brief Status NodeValidator::createDataUnitOrdersWithJson(JsonPair json)
 * 
 * @code {.language-id}
 * if (mVectorDataType.size() != mVectorDataUnitOrder.size())
 * {
 *     LOG_ERROR(logger, "THE LENGTH OF DATA TYPES AND DATA UNIT ORDER MUST BE EQUAL");
 *     return Status(Status::Code::BAD_ENCODING_ERROR);
 * }
 * 
 * for (uint8_t i = 0; i < mVectorDataUnitOrder.size(); ++i)
 * {
 *     const DataUnitOrder& dataUnitOrder = mVectorDataUnitOrder.at(i);
 *     const size_t totalSize = dataUnitOrder.RetrieveTotalSize();
 *     ASSERT((totalSize != 0), "TOTAL SIZE OF A DATA UNIT ORDER CANNOT BE 0");
 * 
 *     const dt_e dataType = mVectorDataType.at(i);
 *     if (dataType != dt_e::BOOLEAN)
 *     {
 *         LOG_ERROR(logger, "DATA UNIT ORDER CANNOT BE SET WHEN DATA TYPE IS BOOLEAN");
 *         return Status(Status::Code::BAD_CONFIGURATION_ERROR);
 *     }
 * 
 *     const size_t dataTypeSize =
 *         (dataType == dt_e::INT8  || dataType == dt_e::UINT8)  ?  8 :
 *         (dataType == dt_e::INT16 || dataType == dt_e::UINT16) ? 16 :
 *         (dataType == dt_e::INT32 || dataType == dt_e::UINT32) ? 32 :
 *         (dataType == dt_e::INT64 || dataType == dt_e::UINT64) ? 64 :
 *         (dataType == dt_e::FLOAT32) ? 32 :
 *         (dataType == dt_e::FLOAT64) ? 64 :
 *         // (dataType == dt_e::STRING)   8;
 * 
 *     if (totalSize != dataTypeSize)
 *     {
 *         LOG_ERROR(logger, "TOTAL SIZE OF DATA UNIT ORDER DOES NOT MATCH THE SIZE OF DATA TYPE");
 *         return Status(Status::Code::BAD_CONFIGURATION_ERROR);
 *     }
 * }
 * @endcode
 */