/**
 * @file Validator.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 설정 정보의 유효성을 검사하기 위한 모듈 클래스를 선언합니다.
 * 
 * @date 2024-10-11
 * @version 0.0.1
 * 
 * @todo 구상 검증 클래스의 Inspect 함수 내에서 매개변수로 받은 key 값이
 *       해당 클래스의 key 값과 일치하는지 검사하는 ASSERT를 추가할 것
 * 
 * @todo 전역변수인 std::nothrow 구조체를 모든 new 연산자에 대한 매개변수로서
 *       전달하지 않는 코드가 없도록 변경해야 합니다. 이는 std::bad_alloc 예외
 *       대신에 nullptr가 반환하도록 설정하기 위한 것으로 메모리 할당 실패 시
 *       별도의 예외 개체 처리 없이도 시스템이 크래시가 나지 않도록 해줍니다.
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <ArduinoJson.h>
#include <map>
#include <set>
#include <vector>

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"
#include "Jarvis/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis {

    class Validator
    {
    public:
        Validator();
        virtual ~Validator();
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        Status Inspect(const JsonDocument& jsonDocument, std::map<cfg_key_e, cin_vector>* mapCIN);
    private:
        Status emplacePairsForCIN(std::map<cfg_key_e, cin_vector>* mapCIN);
        Status validateMetaData(const JsonDocument json);
        Status validateSerialPort(const cfg_key_e key, const JsonArray json, cin_vector* outputVector);
        Status validateNicLAN(const cfg_key_e key, const JsonArray json, cin_vector* outputVector);
        Status validateNicLTE(const cfg_key_e key, const JsonArray json, cin_vector* outputVector);
        Status validateModbus(const cfg_key_e key, const JsonArray json, cin_vector* outputVector);
        Status validateOperation(const cfg_key_e key, const JsonArray json, cin_vector* outputVector);
        Status validateNode(const cfg_key_e key, const JsonArray json, cin_vector* outputVector);
        Status validateAlarm(const cfg_key_e key, const JsonArray json, cin_vector* outputVector);
        Status validateOperationTime(const cfg_key_e key, const JsonArray json, cin_vector* outputVector);
        Status validateProductionInfo(const cfg_key_e key, const JsonArray json, cin_vector* outputVector);
    private:
        /*Metadata*/
        prtcl_ver_e mProtocolVersion;
        std::string mRequestIdentifier;
        std::set<cfg_key_e> mContainerKeySet;
    };
}}