/**
 * @file main.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크가 적용된 펌웨어의 진입점입니다.
 * 
 * @date 2024-10-16
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <MUFFIN.h>

#include "Network/CatM1/CatM1.h"
#include "Storage/CatFS/CatFS.h"



void setup()
{
    MUFFIN muffin;
    muffin.Start();


    using namespace muffin;
    
    CatM1& catM1 = CatM1::GetInstance();
    CatFS* catFS = CatFS::CreateInstanceOrNULL(catM1);
    Status ret = catFS->Begin();
    LOG_DEBUG(logger, "Return: %s", ret.c_str());
}

void loop()
{
    /**
     * @brief Arduino ESP32 Core 프레임워크의 "main.cpp" 파일에 정의된 "loopTask"를 정지합니다.
     */
    vTaskDelete(NULL);

    /**
     * @todo "HardwareSerial Serial(0)" 포트의 RxD로 데이터를 쓸 때, ESP32에서의 처리를 구현해야 합니다.
     * @details "loopTask" 내부에는 "HardwareSerial Serial(0)" 포트로 데이터가 들어오면 비록 비어있긴
     *          해도 "serialEvent(void)" 함수를 호출하고 있습니다. 
     */
}