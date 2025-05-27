/**
 * @file Validator.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 설정 정보의 유효성을 검사하기 위한 모듈 클래스를 선언합니다.
 * 
 * @date 2025-01-23
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 * 
 * @todo 시간이 부족해서 코드나 함수 레벨에서 중복된 작업들이 많습니다.
 *       향후 유지보수를 위해서 이렇게 중복된 코드나 함수를 정리해서 
 *       하나의 표준화 된 함수로 만들어 적용하는 작업이 필요합니다.
 */




#pragma once

#include <ArduinoJson.h>
#include <map>
#include <set>
#include <vector>

#include "Common/Status.h"
#include "JARVIS/Include/Base.h"
#include "JARVIS/Include/TypeDefinitions.h"
#include "JARVIS/Validators/ValidationResult.h"



namespace muffin { namespace jvs {

    class Validator
    {
    public:
        Validator() {}
        virtual ~Validator() {}
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        ValidationResult Inspect(JsonDocument& jsonDocument, std::map<cfg_key_e, cin_vector>* mapCIN);
    private:
        std::pair<rsc_e, std::string> validateMetaData(const JsonObject json);
        std::pair<rsc_e, std::string> validateSerialPort(const cfg_key_e key, const JsonArray json, prtcl_ver_e protocolVersion, cin_vector* outputVector);
        std::pair<rsc_e, std::string> validateNicLAN(const cfg_key_e key, const JsonArray json, prtcl_ver_e protocolVersion, cin_vector* outputVector);
        std::pair<rsc_e, std::string> validateNicLTE(const cfg_key_e key, const JsonArray json, prtcl_ver_e protocolVersion);
        std::pair<rsc_e, std::string> validateModbus(const cfg_key_e key, const JsonArray json, prtcl_ver_e protocolVersion, cin_vector* outputVector);
        std::pair<rsc_e, std::string> validateMelsec(const cfg_key_e key, const JsonArray json, prtcl_ver_e protocolVersion, cin_vector* outputVector);
        std::pair<rsc_e, std::string> validateOperation(const JsonArray json, prtcl_ver_e protocolVersion);
        std::pair<rsc_e, std::string> validateNode(const JsonArray json, prtcl_ver_e protocolVersion, cin_vector* outputVector);
        std::pair<rsc_e, std::string> validateAlarm(const JsonArray json, prtcl_ver_e protocolVersion, cin_vector* outputVector);
        std::pair<rsc_e, std::string> validateOperationTime(const JsonArray json, prtcl_ver_e protocolVersion, cin_vector* outputVector);
        std::pair<rsc_e, std::string> validateProductionInfo(const JsonArray json, prtcl_ver_e protocolVersion, cin_vector* outputVector);
    private:
        std::pair<rsc_e, std::string> removeNotUsedKeys(JsonObject container);
        std::pair<rsc_e, std::string> emplacePairsForCIN(std::map<cfg_key_e, cin_vector>* mapCIN);
    private:
        ValidationResult createInspectionReport();
    private:
        /*Metadata*/
        prtcl_ver_e mProtocolVersion;
        std::string mRequestIdentifier;
        std::set<cfg_key_e> mContainerKeySet;
    private:
        bool mIsUncertain = false;
        std::pair<rsc_e, std::string> mPairUncertainRSC;
        std::map<cfg_key_e, std::pair<rsc_e, std::string>> mMapRSC;
        std::string mResponseDescription;
    };
}}