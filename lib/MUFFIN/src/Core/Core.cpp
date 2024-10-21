/**
 * @file Core.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크 내부의 핵심 기능을 제공하는 클래스를 정의합니다.
 * 
 * @date 2024-10-21
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"
#include "Core.h"
#include "DataFormat/JSON/JSON.h"
#include "Include/Helper.h"
#include "Initializer/Initializer.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "Storage/ESP32FS/ESP32FS.h"
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
            
            if ((i + 1) == MAX_RETRY_COUNT)
            {
                LOG_ERROR(logger, "FAILED TO CONFIGURE MUFFIN: %s", ret.c_str());
                esp_restart();
            }
            else
            {
                LOG_WARNING(logger, "[TRIAL: #%u] CONFIGURATION WAS UNSUCCESSFUL: %s", i, ret.c_str());
            }
            vTaskDelay((5 * SECOND_IN_MILLIS) / portTICK_PERIOD_MS);
        }

        StartTaskMQTT();
    }

    void Core::RouteMqttMessage(const mqtt::Message& message)
    {
        const mqtt::topic_e topic = message.GetTopicCode();
        const std::string payload = message.GetPayload();

        switch (topic)
        {
        /**
         * @todo 현재 CIA 개체에 저장된 메시지를 Peek() 하는 게 아니라 Retrieve()로
         *       가져오기 때문에 startJarvisTask()가 실패하는 경우 해당 메시지는
         *       소실되게 됩니다. 따라서 요청이 사라지지 않게 보완하는 작업이 필요합니다.
         */
        case mqtt::topic_e::JARVIS_REQUEST:
            startJarvisTask(payload);
            return;
        /**
         * @todo 원격제어 명령 수신 토픽에 대한 처리 작업을 구현해야 합니다.
         */
        case mqtt::topic_e::REMOTE_CONTROL:
            ASSERT(false, "UNIMPLEMENTED ERROR: REMOTE CONTROL TOPIC IS NOT SUPPORTED");
            break;
        default:
            ASSERT(false, "UNDEFINED ERROR: MAY BE NEWLY DEFINED TOPIC OR AN UNEXPECTED ERROR");
            break;
        }
        
        // mqtt::CatMQTT& catMqtt = mqtt::CatMQTT::GetInstance();
    }

    void Core::startJarvisTask(const std::string& payload)
    {
        jarvis_task_params* pvParameters = new(std::nothrow) jarvis_task_params();
        if (pvParameters == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR TASK PARAMETERS");
            return;
        }

        // uintptr_t pvParameters[3];
        // pvParameters[0] = (uintptr_t)onJarvisValidationResult;
        // pvParameters[1] = (uintptr_t)&payload;
        // pvParameters[2] = (uintptr_t)&mJarvisValidationResult;

        pvParameters->Callback = onJarvisValidationResult;
        pvParameters->RequestPayload = payload;

        /**
         * @todo 스택 오버플로우를 방지하기 위해서 JARVIS 설정 정보 크기에 따라서 태스크에 할당하는 스택 메모리의 크기를 조정해야 합니다.
         * @todo 실행 도중에 태스크를 시작하면 필요한만큼의 스택을 할당할 수 없습니다. 그 외에도 태스크를 실행시키는 데 실패할 수 있습니다.
         *       이러한 경우에는 서버로 오류 메시지를 보낼 수 있도록 코드를 추가로 개발해야 합니다.
         */
        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            ProcessJarvisRequestTask,   // Function to be run inside of the task
            "ProcessJarvisRequestTask", // The identifier of this task for men
            10 * KILLOBYTE,             // Stack memory size to allocate
            pvParameters,	            // Task parameters to be passed to the function
            0,				            // Task Priority for scheduling
            NULL,                       // The identifier of this task for machines
            0				            // Index of MCU core where the function to run
        );

        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The JARVIS task has been started");
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

    void Core::onJarvisValidationResult(jarvis::ValidationResult result)
    {
        JsonDocument doc;
        JsonArray arrayCFG = doc["cfg"].to<JsonArray>();

        doc["ts"] = GetTimestampInMillis();
        doc["rsc"] = Convert.ToUInt16(result.GetRSC());
        doc["dsc"] = result.GetDescription();

        std::vector<jarvis::cfg_key_e> vectorKeyWithNG = result.RetrieveKeyWithNG();
        for (auto& key : vectorKeyWithNG)
        {
            arrayCFG.add(Convert.ToString(key));
        }

        std::string payload;
        serializeJson(doc, payload);

        mqtt::Message message(mqtt::topic_e::JARVIS_RESPONSE, payload);
        LOG_DEBUG(logger, "JARVIS Response: %s", message.GetPayload());

        mqtt::CatMQTT& catMqtt = mqtt::CatMQTT::GetInstance();
        for (uint8_t i = 0; i < MAX_RETRY_COUNT; ++i)
        {
            Status ret = catMqtt.Publish(message);
            if (ret == Status::Code::GOOD)
            {
                break;
            }
            else if ((i + 1) == MAX_RETRY_COUNT)
            {
                /**
                 * @todo 메시지 전송에 실패했을 때 나중에 다시 전송을 시도할 수 있도록 기능을 보완해야 합니다.
                 */
                LOG_ERROR(logger, "FAILED TO PUBLISH MESSAGE: %s", ret.c_str());
                /**
                 * @todo 메시지 전송에 실패한 경우에 return 해서 빠져나갈지 아니면 설정 정보를
                 *        저장한 다음 설정 정보를 적용할지 여부를 결정해야 합니다.
                 */
                ASSERT(false, "IMPLEMENTATION ERROR: NEED TO DECIDE BETWEEN EARLY EXIT AND APPLYING THE CONFIG");
                break;
            }
            else
            {
                LOG_WARNING(logger, "[TRIAL: #%u] PUBLISH WAS UNSUCCESSFUL: %s", i, ret.c_str());
            }
        }


        if (result.GetRSC() >= jarvis::rsc_e::UNCERTAIN)
        {
            LOG_WARNING(logger, "INVALID JARVIS CONFIGURATION: NO SAVING AND APPLYING");
            return ;
        }


        LOG_INFO(logger, "JARVIS configuration is valid. Proceed to save and apply");
        std::string jarvisPayload;
        RetrieveJarvisRequestPayload(&jarvisPayload);


        // ESP32FS& esp32FS = ESP32FS::GetInstance();
        // /** 
        //  * @todo string이 아니라 enum class와 ConvertClass를 사용하도록 코드를 수정해야 합니다.
        //  */
        // File file = esp32FS.Open("/jarvis/config.json", "w", true);

        // /**
        //  * @todo 성능 향상을 위해 속도가 더 빠른 buffered IO 방식으로 코드를 수정해야 합니다.
        //  */
        // for (size_t i = 0; i < jarvisPayload.length(); ++i)
        // {
        //     file.write(jarvisPayload[i]);
        // }

        // /**
        //  * @todo 저장 성공 유무를 확인할 수 있는 기능을 추가해야 합니다.
        //  */
        // file.close();

        /**
         * @todo 설정 정보가 올바르게 설정되었는지 확인하는 기능을 추가해야 합니다.
         */
        ApplyJarvisTask();
    }


    Core* Core::mInstance = nullptr;
    jarvis::ValidationResult Core::mJarvisValidationResult;
}