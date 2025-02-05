/**
 * @file main.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크가 적용된 펌웨어의 진입점입니다.
 * 
 * @date 2025-01-20
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <MUFFIN.h>
#if defined(DEBUG)
    #include <Esp.h>
    #include <Common/Logger/Logger.h>
#endif



void setup()
{
    MUFFIN muffin;
    muffin.Start();
}

void loop()
{
#if defined(DEBUG)
    // LOG_DEBUG(muffin::logger, "Remained Heap: %u Bytes", ESP.getFreeHeap());
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
    vTaskDelete(NULL);
#else
    /**
    vTaskDelete(NULL);
     * @brief Arduino ESP32 Core 프레임워크의 "main.cpp" 파일에 정의된 "loopTask"를 정지합니다.
     */
#endif



/**
 * @todo "HardwareSerial Serial(0)" 포트의 RxD로 데이터를 쓸 때, ESP32에서의 처리를 구현해야 합니다.
 * @details "loopTask" 내부에는 "HardwareSerial Serial(0)" 포트로 데이터가 들어오면 비록 비어있긴
 *          해도 "serialEvent(void)" 함수를 호출하고 있습니다. 
 */
}