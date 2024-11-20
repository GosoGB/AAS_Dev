/**
 * @file Initializer.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크의 초기화를 담당하는 클래스를 선언합니다.
 * 
 * @date 2024-10-30
 * @version 1.0.0
 * 
 * @todo Option #1: MODLINK 모델 별로 기본 설정이 달라지는 부분을 고려해야 합니다.
 * @todo Option #2: 블루투스를 이용해서 사용자가 설정할 수 있게 만들어야 합니다.
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include "Common/Status.h"



namespace muffin {

    class Initializer
    {
    public:
        Initializer();
        virtual ~Initializer();
    public:
        void StartOrCrash();
        Status Configure();
    private:
        Status configureWithoutJarvis();
        Status configureWithJarvis();
    private:
        static constexpr const char* JARVIS_FILE_PATH = "/jarvis/config.json";
        bool mIsMqttTopicCreated = false;
        bool mIsCatMQTTConnected = false;
        bool mIsCatHTTPConfigured = false;
    };
}