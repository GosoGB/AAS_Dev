/**
 * @file RequestHandler.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-21
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <WebServer.h>

#include "../../Container/Container.hpp"
#include "../../Serializer/Serializer.hpp"

#include "Common/Base64/Base64.hpp"
#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {

    
    WebServer server(INADDR_NONE, 8081);


    // 루트 경로 (/) 요청 처리 함수
    void handleRoot()
    {
        server.send(404, "text/html", "Not Found"); // 200 OK 응답과 HTML 내용 전송
    }

    void HandleGetAllAssetAdministrationShells()
    {
        AssetAdministrationShellSerializer serializer;
        const psram::string payload = serializer.EncodeAll();
        server.send(200, "application/json", payload.c_str());

        log_d("[%u] GET \"/shells\"", time(NULL));
    }

    void HandleGetAllSubmodels()
    {
        SubmodelsSerializer serializer;
        const psram::string payload = serializer.EncodeAll();
        server.send(200, "application/json", payload.c_str());

        log_d("[%u] GET \"/submodels\"", time(NULL));
    }

    psram::vector<psram::string> splitString(const psram::string& str, char delimiter)
    {
        psram::vector<psram::string> tokens;
        psram::string token;
        size_t start = 0;
        size_t end = str.find(delimiter); // 첫 번째 구분자 찾기

        // 구분자가 발견되지 않을 때까지 반복
        while (end != psram::string::npos)
        {
            token = str.substr(start, end - start); // 구분자 이전까지의 문자열 추출
            tokens.push_back(token);                // 추출된 문자열 저장
            start = end + 1;                        // 다음 시작 위치는 구분자 다음
            end = str.find(delimiter, start);       // 다음 구분자 찾기 (현재 시작 위치부터)
        }

        // 마지막 토큰 (또는 구분자가 없는 경우 전체 문자열) 추가
        token = str.substr(start);
        tokens.push_back(token);

        return tokens;
    }

    psram::string ProcessOperationalData(const psram::string& submodelId, const psram::string& idShort)
    {
        SubmodelsSerializer serializer;
        return serializer.EncodeProperty(submodelId, idShort);
    }

    void HandleGetSubmodelElement()
    {
        const psram::string uri = server.uri().c_str();
        psram::vector<psram::string> tokens = splitString(uri, '/');

        if ((psram::string("submodels") != tokens[1]) || (psram::string("submodel-elements") != tokens[3]))
        {
            server.send(404, "text/html", "Not Found");
            return;
        }

        psram::string submodelId = DecodeBase64(tokens[2].c_str());
        log_d("Requested Submodel ID: %s", submodelId.c_str());

        Container* container = Container::GetInstance();
        const psram::vector<Submodel> submodels = container->GetAllSubmodels();

        uint8_t smIdx = 0;
        for (const auto& submodel : submodels)
        {
            if (submodelId == submodel.GetID())
            {
                break;
            }
            ++smIdx;
        }

        if (smIdx == 5)
        {
            server.send(404, "text/html", "Not Found");
            return;
        }
        
        log_d("GET %s", server.uri().c_str());
        psram::string payload = ProcessOperationalData(submodelId, tokens[4]);
        server.send(200, "application/json", payload.c_str()); // 200 OK 응답과 텍스트 내용 전송
    }
}}
