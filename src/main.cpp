/**
 * @file main.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2024-10-16
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <MUFFIN.h>


// static std::string PROGMEM JARVIS_DEFAULT = R"({"ver":"v1","cnt":{"rs232":[],"rs485":[{"prt":2,"bdr":9600,"dbit":8,"pbit":0,"sbit":1}],"wifi":[],"eth":[],"catm1":[{"md":"LM5","ctry":"KR"}],"mbrtu":[{"ptr":2,"sid":1,"nodes":["no01","no02"]}],"mbtcp":[],"op":[{"snic":"lte","exp":true,"intvPoll":1,"intvSrv":60,"ota":false}],"node":[{"id":"no01","adtp":0,"addr":0,"area":4,"bit":null,"qty":1,"scl":-1,"ofst":null,"dt":3,"map":null,"ord":null,"uid":"DI1","fmt":null,"name":"테스트","unit":"N/A","event":true},{"id":"no01","adtp":0,"addr":1,"area":4,"bit":3,"qty":null,"scl":null,"ofst":null,"dt":3,"map":{"0":"닫힘","1":"열림"},"ord":"nuill","uid":"DO1","name":"테스트","unit":"N/A","event":true}],"alarm":[],"optime":[],"prod":[]}})";

void setup()
{
    MUFFIN muffin;
    muffin.Start();
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


//     /***********************************************************/

//     muffin::http::HttpHeader header1(
//         muffin::rest_method_e::GET,
//         muffin::http_scheme_e::HTTP,
//         "ec2-3-38-208-214.ap-northeast-2.compute.amazonaws.com",
//         5000,
//         "/api/server/time",
//         "MODLINK-L/0.0.1"
//     );
//     http.GET(header1);

//     muffin::http::HttpHeader header2(
//         muffin::rest_method_e::POST,
//         muffin::http_scheme_e::HTTP,
//         "ec2-3-38-208-214.ap-northeast-2.compute.amazonaws.com",
//         5000,
//         "/api/firmware/version",
//         "MODLINK-L/0.0.1"
//     );

//     muffin::http::HttpBody body("application/x-www-form-urlencoded");
//     body.AddProperty("cp", "1000");
//     body.AddProperty("md", "TEST");
//     http.POST(header2, body);
//     delay(UINT32_MAX);