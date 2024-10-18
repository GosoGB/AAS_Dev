/**
 * @file Core.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크 내부의 핵심 기능을 제공하는 클래스를 정의합니다.
 * 
 * @date 2024-10-16
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Core.h"
#include "DataFormat/JSON/JSON.h"
#include "Include/Helper.h"
#include "Initializer/Initializer.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "Task/MqttTask.h"
#include "Task/JarvisTask.h"





namespace muffin {

    Core& Core::GetInstance() noexcept
    {
        if (mInstance == nullptr)
        {
            logger = new(std::nothrow) muffin::Logger();
            if (logger == nullptr)
            {
                ASSERT(false, "FATAL ERROR OCCURED: FAILED TO ALLOCATE MEMORY FOR LOGGER");
                esp_restart();
            }
            
            mInstance = new(std::nothrow) Core();
            if (mInstance == nullptr)
            {
                ASSERT(false, "FATAL ERROR OCCURED: FAILED TO ALLOCATE MEMORY FOR MUFFIN CORE");
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMROY FOR MUFFIN CORE");
                esp_restart();
            }
        }
        
        return *mInstance;
    }

    Core::Core()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }

    Core::~Core()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    void Core::Init()
    {
        /**
         * @todo Reset 사유에 따라 자동으로 초기화 하는 기능의 개발이 필요합니다.
         * @details JARVIS 설정으로 인해 런타임에 크래시 같은 문제가 있을 수 있습니다.
         *          이러한 경우에는 계속해서 반복적으로 MODLINK가 리셋되는 현상이 발생할
         *          수 있습니다. 따라서 reset 사유를 확인하여 JARVIS 설정을 초기화 하는
         *          기능이 필요합니다. 단, 다른 부서와의 협의가 선행되어야 합니다.
         */
        mResetReason = esp_reset_reason();
        printResetReason(mResetReason);

        Initializer initializer;
        initializer.StartOrCrash();

        constexpr uint8_t MAX_RETRY_COUNT = 5;
        constexpr uint16_t SECOND_IN_MILLIS = 1000;
        /**
         * @todo 설정이 정상적이지 않을 때 어떻게 처리할 것인지 결정해야 합니다.
         * @details 현재는 설정에 실패했을 때, 최대 5번까지 다시 시도하게 되어 있습니다.
         *          설정에 성공했다면 루프를 빠져나가게 됩니다.
         */
        for (uint8_t i = 0; i < MAX_RETRY_COUNT; ++i)
        {
            Status ret = initializer.Configure();
            if (ret == Status::Code::GOOD)
            {
                LOG_INFO(logger, "MUFFIN is configured and ready to go!");
                break;
            }
            else
            {
                if ((i + 1) < MAX_RETRY_COUNT)
                {
                    LOG_ERROR(logger, "FAILED TO CONFIGURE MUFFIN: %s", ret.c_str());
                }
                else
                {
                    LOG_WARNING(logger, "[TRIAL: #%u] CONFIGURATION WAS UNSUCCESSFUL: %s", i, ret.c_str());
                }
                vTaskDelay((5 * SECOND_IN_MILLIS) / portTICK_PERIOD_MS);
            }
        }

        StartTaskMQTT();
    }

    void Core::HandleMqttMessage(const mqtt::Message& message)
    {
        const mqtt::topic_e topic = message.GetTopicCode();
        const std::string payload = message.GetPayload();
        
        JSON json;
        JsonDocument doc;
        Status retJSON = json.Deserialize(payload, &doc);
        if (retJSON != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE JSON: %s", retJSON.c_str());
        }

        mqtt::CatMQTT& catMqtt = mqtt::CatMQTT::GetInstance();
        switch (topic)
        {
        case mqtt::topic_e::JARVIS_REQUEST:
            if (retJSON != Status::Code::GOOD)
            {
                mqtt::Message responseMessage;
                responseMessage.SetTopic(mqtt::topic_e::JARVIS_RESPONSE);
                responseMessage.SetPayload(CreateDecodingErrorPayload(retJSON.c_str()));
                catMqtt.Publish(responseMessage);
                LOG_DEBUG(logger, "Pub Message: %s", responseMessage.GetPayload());
            }
            else
            {
                doc = FetchJarvis();
                mqtt::Message reportMessage = JarvisTask(doc);
                catMqtt.Publish(reportMessage);
                LOG_DEBUG(logger, "Pub Message: %s", reportMessage.GetPayload());
            }
            break;
        default:
            break;
        }
        
    }


    Core* Core::mInstance = nullptr;
}