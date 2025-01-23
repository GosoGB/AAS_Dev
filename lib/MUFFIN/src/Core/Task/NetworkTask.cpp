/**
 * @file NetworkTask.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크 인터페이스 사용과 관련된 태스크를 정의합니다.
 * 
 * @date 2024-12-31
 * @version 1.2.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Status.h"
#include "Common/Time/TimeUtils.h"
#include "Core/Include/Helper.h"
#include "Core/Task/NetworkTask.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "JARVIS/Include/Base.h"
#include "NetworkTask.h"
#include "Protocol/HTTP/CatHTTP/CatHTTP.h"
#include "Protocol/MQTT/CIA.h"
#include "Protocol/MQTT/CDO.h"
#include "Protocol/MQTT/Include/BrokerInfo.h"
#include "Protocol/MQTT/Include/Topic.h"
#include "Protocol/MQTT/Include/Helper.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "Protocol/MQTT/LwipMQTT/LwipMQTT.h"




namespace muffin {

    TaskHandle_t xTaskCatM1Handle;
    TaskHandle_t xTaskEthernetHandle;

    static bool s_IsCatM1Connected        = false;
    static bool s_IsCatMqttInitialized    = false;
    static bool s_IsCatMQTTConnected      = false;
    static bool s_IsCatHttpInitialized    = false;


    void implCatM1Task(void* pvParameters)
    {
        while (true)
        {
            if (catM1.IsConnected() == false)
            {
                LOG_WARNING(logger, "LTE Cat.M1 LOST CONNECTION");
                catM1.Reconnect();
                s_IsCatM1Connected       = false;
                s_IsCatMqttInitialized   = false;
                s_IsCatMQTTConnected     = false;
                s_IsCatHttpInitialized   = false;

                jvs::config::CatM1 retrievedCIN = catM1.RetrieveConfig().second;
                while (InitCatM1(&retrievedCIN) != Status::Code::GOOD)
                {
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                }
                InitCatHTTP();
                ConnectToBroker();
            }
            
            
            if ((catMqtt.IsConnected() != Status::Code::GOOD) || (s_IsCatMQTTConnected == false))
            {
                LOG_WARNING(logger, "CatMQTT LOST CONNECTION");
                ConnectToBroker();
            }


            if (s_IsCatHttpInitialized == false)
            {
                InitCatHTTP();
            }
        }
    }

    /**
     * @todo 상태 코드를 반환하도록 코드를 수정해야 합니다.
     */
    void StartCatM1Task()
    {
        if (xTaskCatM1Handle != NULL)
        {
            LOG_WARNING(logger, "THE TASK HAS ALREADY STARTED");
            return;
        }
        
        /**
         * @todo 향후 태스크의 메모리 사용량을 보고 스택 메모리 크기를 조정해야 합니다.
         */
        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            implCatM1Task,      // Function to be run inside of the task
            "CatM1Task",        // The identifier of this task for men
            4 * 1024,			// Stack memory size to allocate
            NULL,			    // Task parameters to be passed to the function
            0,				    // Task Priority for scheduling
            &xTaskCatM1Handle,  // The identifier of this task for machines
            0				    // Index of MCU core where the function to run
        );

        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The CatM1 task has been started");
            // return Status(Status::Code::GOOD);
            break;

        case pdFAIL:
            LOG_ERROR(logger, "FAILED TO START WITHOUT SPECIFIC REASON");
            // return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            break;

        case errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY:
            LOG_ERROR(logger, "FAILED TO ALLOCATE ENOUGH MEMORY FOR THE TASK");
            // return Status(Status::Code::BAD_OUT_OF_MEMORY);
            break;

        default:
            LOG_ERROR(logger, "UNKNOWN ERROR: %d", taskCreationResult);
            // return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            break;
        }
    }
}