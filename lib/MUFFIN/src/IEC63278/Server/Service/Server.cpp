/**
 * @file Server.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-21
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#include "../../Container/Container.hpp"
#include "../../Container/include/AASXLoader.hpp"
#include "../Server.hpp"
#include "./RequestHandler.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    Server* Server::GetInstance()
    {
        if (mInstance == nullptr)
        {
            void* allocatedMemory = psram::allocate(sizeof(Server));
            if (allocatedMemory == nullptr)
            {
                log_e("BAD OUT OF MEMORY");
                return nullptr;
            }

            mInstance = new(allocatedMemory) Server();
        }

        return mInstance;
    }


    void HandleRequest(void* pvParams)
    {
        while (true)
        {
            server.handleClient();
            delay(500);
        }
    }

    void Server::Init()
    {
        AASXLoader aasxLoader;
        aasxLoader.Start();
        
        // 루트 경로 (/)에 대한 GET 요청 핸들러
        server.on("/", HTTP_GET, handleRoot);

        // GET 요청 핸들러: SubmodelElement
        server.onNotFound(HandleGetSubmodelElement);

        // GET 요청 핸들러: GetAllAssetAdministrationShells
        server.on("/shells", HTTP_GET, HandleGetAllAssetAdministrationShells);

        // GET 요청 핸들러: GetAllSubmodels
        server.on("/submodels", HTTP_GET, HandleGetAllSubmodels);

        // HTTP 서버 시작
        server.begin();

        xTaskCreatePinnedToCore(
            HandleRequest,
            "AasRequestHandleTask",
            10240,
            NULL,
            0,
            NULL,
            0
        );
    }


    Server* Server::mInstance = nullptr;
}}