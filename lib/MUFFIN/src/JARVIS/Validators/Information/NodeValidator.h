/**
 * @file NodeValidator.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Node 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2025-02-26
 * @version 1.2.13
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 * 
 * @todo Node 설정 유형별로 테스트 케이스를 설계한 다음 다음 유닛 테스트를 작성할 것
 */




#pragma once

#include <ArduinoJson.h>
#include <vector>
#include <regex>

#include "Common/Status.h"
#include "JARVIS/Include/Base.h"
#include "JARVIS/Include/DataUnitOrder.h"
#include "JARVIS/Include/TypeDefinitions.h"
#include "Protocol/MQTT/Include/TypeDefinitions.h"



namespace muffin { namespace jvs {

    class NodeValidator
    {
    public:
        NodeValidator();
        virtual ~NodeValidator() {}
    private:
        using cin_vector = std::vector<config::Base*>;
    private:
        prtcl_ver_e mProtocolVersion;
    public:
        std::pair<rsc_e, std::string> Inspect(const JsonArray arrayCIN, prtcl_ver_e protocolVersion, cin_vector* outVector);
    private:
        rsc_e validateMandatoryKeys(const JsonObject json);
        rsc_e validateMandatoryValues(const JsonObject json);
        std::pair<rsc_e, std::string> validateNodeArea();
        std::pair<rsc_e, std::string> validateBitIndex();
        std::pair<rsc_e, std::string> validateAddressQuantity();
        std::pair<rsc_e, std::string> validateNumericScale();
        std::pair<rsc_e, std::string> validateNumericOffset();
        std::pair<rsc_e, std::string> validateDataUnitOrders();
        std::pair<rsc_e, std::string> validateDataTypes();
        std::pair<rsc_e, std::string> validateFormatString();
    private:
        std::pair<rsc_e, std::vector<dt_e>> processDataTypes(JsonArray dataTypes);
        std::pair<rsc_e, std::vector<DataUnitOrder>> processDataUnitOrders(JsonVariant dataUnitOrders);
        // Status emplaceCIN(config::Base* cin, cin_vector* outVector);
    private:
        void convertToPrecision(JsonVariant precision);
        void convertToArraySampleInterval(JsonVariant arraySamepleInterval);
        void convertToArrayIndex(JsonArray arrayIndex);
        void convertToTopic(const uint8_t topic);
        void convertToAdressType(const uint8_t type);
        void convertToAddress(JsonVariant address);
        void convertToNodeArea(JsonVariant nodeArea);
        void convertToBitIndex(JsonVariant bitIndex);
        void convertToAddressQuantity(JsonVariant addressQuantity);
        void convertToNumericScale(JsonVariant numericScale);
        void convertToNumericOffset(JsonVariant numericOffset);
        std::pair<rsc_e, ord_t> convertToDataUnitOrderType(const std::string& value);
        std::pair<rsc_e, dt_e> convertToDataType(const uint8_t dataType);
        void convertToFormatString(const JsonVariant formatString);
    private:
        char mNodeID[5];
        std::pair<rsc_e, adtp_e> mAddressType;
        std::pair<rsc_e, addr_u> mAddress;
        std::pair<rsc_e, node_area_e> mNodeArea;
        std::pair<rsc_e, uint8_t> mBitIndex;
        std::pair<rsc_e, uint8_t> mAddressQuantity;
        std::pair<rsc_e, scl_e> mNumericScale;
        std::pair<rsc_e, float> mNumericOffset;
        std::pair<rsc_e, std::vector<DataUnitOrder>> mDataUnitOrders;
        std::pair<rsc_e, std::vector<dt_e>> mDataTypes;
        std::pair<rsc_e, std::string> mFormatString;
        std::pair<rsc_e, mqtt::topic_e> mTopic;
        std::pair<rsc_e, std::vector<std::array<uint16_t, 2>>> mArrayIndex;
        std::pair<rsc_e, uint16_t> mArraySampleInterval;
        std::pair<rsc_e, uint8_t> mPrecision;
        bool mIsEventType = false;
    private:
        std::vector<fmt_spec_e> mVectorFormatSpecifier;
    };
}}