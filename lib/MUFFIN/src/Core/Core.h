/**
 * @file Core.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크 내부의 핵심 기능을 제공하는 클래스를 선언합니다.
 * 
 * @date 2024-10-21
 * @version 0.0.1
 * 
 * @todo 사용자로부터 네트워크 설정 정보를 받는 기능을 구현해야 합니다.
 *       네트워크 설정 변경과 같은 사유로 인해 인터넷에 연결될 수 없는 상황일 때,
 *       사용자로부터 네트워크 설정을 받아 적용하는 프로세스를 만들어야 합니다.
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Common/Status.h"
#include "Protocol/MQTT/Include/Message.h"
#include "Jarvis/Validators/ValidationResult.h"



namespace muffin {
    
    class Core
    {
    public:
        Core(Core const&) = delete;
        void operator=(Core const&) = delete;
        static Core& GetInstance() noexcept;
    private:
        Core();
        virtual ~Core();
    private:
        static Core* mInstance;

    public:
        void Init();
    
    public:
        void RouteMqttMessage(const mqtt::Message& message);
    private:
        void startJarvisTask(const std::string& payload);
        static void onJarvisValidationResult(jarvis::ValidationResult result);
    private:
        static jarvis::ValidationResult mJarvisValidationResult;
    private:
        esp_reset_reason_t mResetReason;
        static constexpr uint8_t MAX_RETRY_COUNT = 5;
        static constexpr uint16_t SECOND_IN_MILLIS = 1000;
        static constexpr uint16_t KILLOBYTE = 1024;
    };
}