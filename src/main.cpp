/**
 * @file main.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크가 적용된 펌웨어의 진입점입니다.
 * 
 * @date 2024-10-16
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */



#include <Arduino.h>
#include <MUFFIN.h>
#include <OTA/MEGA2560/MEGA2560.h>



void setup()
{
    delay(10 * 1000); // MEGA 펌웨어 업로드 시간 확보용

    MUFFIN muffin;
    muffin.Start();
    
    muffin::ota::MEGA2560 mega2560;
    mega2560.Init();
    mega2560.LoadAddress(0);
    mega2560.ProgramFlashISP();
    delay(100);
    mega2560.LeaveProgrammingMode();
    mega2560.TearDown();
    // mega2560.LoadAddress(1100);
    // mega2560.ReadFlashISP(256);
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