/**
 * @file main.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크가 적용된 펌웨어의 진입점입니다.
 * 
 * @date 2025-01-20
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <MUFFIN.h>
#include "Protocol/EthernetIP/ciplibs/cip_client.h"
#include "Protocol/EthernetIP/ciplibs/cip_msr.h"
#include "Protocol/EthernetIP/ciplibs/cip_single.h"
#include "Protocol/EthernetIP/ciplibs/cip_types.h"
#include "Protocol/EthernetIP/ciplibs/cip_path.h"
#include "Protocol/EthernetIP/ciplibs/cip_util.h"
#include "Protocol/EthernetIP/ciplibs/eip_session.h"
#include "Protocol/EthernetIP/ciplibs/eip_connection.h"
#include "Protocol/EthernetIP/ciplibs/eip_types.h"
#include "Network/Ethernet/W5500/W5500.h"
#include "JARVIS/Config/Network/Ethernet.h"


#if defined(DEBUG)
    #include <Esp.h>
    #include <Common/Logger/Logger.h>
    #include <LittleFS.h>
#endif


using namespace muffin;
using namespace muffin::w5500;

W5500 ethernetChip(if_e::EMBEDDED);
EIPSession session;

void setup()
{
    logger.Init();
    delay(1000);

    ethernetChip.Init();

    jvs::config::Ethernet config;

    config.SetDHCP(false);
    config.SetStaticIPv4(IPAddress(10, 11, 12, 11));
    config.SetGateway(IPAddress(10, 11, 12, 1));
    config.SetSubnetmask(IPAddress(255, 255, 255, 0));
    config.SetDNS1(IPAddress(8, 8, 8, 8));
    config.SetDNS2(IPAddress(8, 8, 8, 8));

    ethernetChip.Config(&config);
    ethernetChip.Connect();

    EthernetClient client(ethernetChip, sock_id_e::SOCKET_0);
    session.client = client;
    
    session.targetIP = IPAddress(10, 11, 12, 10);

    //session.targetIP = IPAddress(10, 11, 12, 10);
    session.targetPort = 44818;

    if (!eipInit(session, session.targetIP, session.targetPort)) {
        Serial.println("[CIP] eipInit failed");
        return;
    }
    if (!registerSession(session)) {
        Serial.println("[CIP] RegisterSession failed");
        return;
    }
    /* forwardOpen ( 클래스 3에서는 사용 안함 )
    if (!forwardOpen(session)) {
        Serial.println("[CIP] ForwardOpen failed");
        return;
    }
    */

    Serial.println("[CIP] Session opened");
}
// void setup()
// {
//     MUFFIN muffin;
//     // muffin.Start();
    
//     logger.Init();

//     W5500 ethernetChip(muffin::w5500::if_e::EMBEDDED);

//     ethernetChip.Init();
//     ethernetChip.setLocalIP(IPAddress(10,11,12,11));
//     ethernetChip.setSubnetmask(IPAddress(255,255,255,0));
//     ethernetChip.setGateway(IPAddress(10,11,12,1));
//     ethernetChip.Connect();

//     w5500::EthernetClient client(ethernetChip, w5500::sock_id_e::SOCKET_0);
//     session.client = client;
//     session.targetIP = IPAddress(10, 11, 12, 10);
//     session.targetPort = 44818;

//     if (!eipInit(session, session.targetIP, session.targetPort)) 
//     {
//         LOG_DEBUG(logger,"[CIP] eipInit failed");
//         return;
//     }

//     if (!registerSession(session)) 
//     {
//         LOG_DEBUG(logger,"[CIP] RegisterSession failed");
//         return;
//     }

//     LOG_DEBUG(logger,"[CIP] Session opened");

// }

void loop()
{

    uint16_t test = 0x0065;
    uint16_t result = ntohs(test);
    LOG_DEBUG(logger,"[DEBUG] loop start........\n");
    LOG_DEBUG(logger,"[DEBUG] ntohs(0x%04X) = 0x%04X\n\n\n", test, result);

    if (!session.client.connected()) 
    {
        LOG_DEBUG(logger,"[CIP] 연결 끊김 감지됨. 재연결 시도 중...");

        // 소켓 다시 초기화 (선택 사항: 필요시)
        //EthernetClient newClient(ethernetChip, sock_id_e::SOCKET_0);
        //session.client = newClient;

        if (!eipInit(session, session.targetIP, session.targetPort)) 
        {
            LOG_DEBUG(logger,"[CIP] 재연결: eipInit 실패");
            session.client.stop();  // 실패 시 다시
            delay(2000);
            return;
        }

        if (!registerSession(session)) 
        {
            LOG_DEBUG(logger,"[CIP] 재연결: RegisterSession 실패");
            session.client.stop();  // 실패 시 다시
            delay(2000);
            return;
        }

        Serial.print("[CIP] 세션 핸들 등록 완료: ");
        LOG_DEBUG(logger,"%02X",session.sessionHandle);  // 16진수 출력

/*      forwardOpen ( 클래스 3에서는 사용 안함 )
        if (!forwardOpen(session)) {
            LOG_DEBUG(logger,"[CIP] 재연결: ForwardOpen 실패");
            session.client.stop();  // 실패 시 다시
            delay(2000);
            return;
        }
*/
        LOG_DEBUG(logger,"[CIP] 세션 재연결 성공");
    }

    // 세션 핸들 확인
    LOG_DEBUG(logger,"[CIP] 현재 세션 핸들: ");
    LOG_DEBUG(logger,"%02X",session.sessionHandle);    

    // 20250629 테스트
    // Tag 일기
    std::string tagName = "Type_STRING";  // 읽을 태그 이름
    cip_data_t readData;
    Serial.printf("========================================================================\n");
    Serial.printf("=== Reading single Tag: %s \n", tagName.c_str());
    Serial.printf("========================================================================\n");
    if (!readTag(session, tagName, readData)) 
    {
        LOG_ERROR(logger,"[readSingleTag] Read FAILED, General Status = 0x%02X\n", readData.Code);
    } 
    else 
    {
        // 응답은 왔음 -> Code 검사
        if (readData.Code == 0x00) 
        {
            printCipData(readData);
        } 
        else 
        {
            LOG_ERROR(logger,"Error Code = 0x%02X\n", readData.Code);
        }
    }

    delay(10000);

    // // 결과 저장
    // std::vector<cip_data_t> readResults;
    // tagName = "Type_SINT_A1D";
    // Serial.printf("========================================================================\n");
    // Serial.printf("=== Reading array Tag: %s\n", tagName.c_str());
    // Serial.printf("========================================================================\n");

    // // 배열 요소 5개 읽기
    // if (!readTagExt( session, tagName, 5, readResults)) {
    //     Serial.printf("[readTagExt] Read FAILED\n");
    // } else {
    //     Serial.printf("[readTagExt] Read OK. Elements: %zu\n", readResults.size());

    //     for (size_t i = 0; i < readResults.size(); ++i) {
    //         const cip_data_t& elem = readResults[i];

    //         if (elem.Code != 0) {
    //             Serial.printf(" Element[%zu] ERROR (Code=0x%02X)\n", i, elem.Code);
    //         } else {
    //             Serial.printf(" Element[%zu]: ", i);
    //             printCipData(elem);  // 값 출력
    //         }
    //     }
    // }

    // Serial.println("다중 읽기 테스트");
    // Serial.println("==============");
    // std::vector<std::string> readTags = { "Type_SINT_A1D[0]", "Type_SINT_A1D[1]", "Type_SINT_A1D[2]" };
    // std::vector<cip_data_t> readResults2;
    // if (readTagsMSR(session, readTags, readResults2)) {

    //     for (size_t i = 0; i < readResults2.size(); ++i) {
    //         const cip_data_t& elem = readResults2[i];

    //         if (elem.Code != 0) {
    //             Serial.printf(" Element[%zu] ERROR (Code=0x%02X)\n", i, elem.Code);
    //         } else {
    //             Serial.printf(" Element[%zu]: ", i);
    //             printCipData(elem);  // 값 출력
    //         }
    //     }
    // } else {
    //     Serial.printf("Multi-tag read failed.\n");
    // }
    // delay(4000);







#if defined(DEBUG)
    LOG_DEBUG(muffin::logger, "Remained Heap: %u Bytes", ESP.getFreeHeap());
    LOG_DEBUG(muffin::logger, "Remained Flash: %u Bytes", LittleFS.totalBytes() - LittleFS.usedBytes());
#if defined(MT11)
    LOG_DEBUG(muffin::logger, "Remained PSRAM : %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
#endif
    vTaskDelay(5000 / portTICK_PERIOD_MS);
#else
    vTaskDelete(NULL);
    /**
     * @brief Arduino ESP32 Core 프레임워크의 "main.cpp" 파일에 정의된 "loopTask"를 정지합니다.
     */
#endif



/**
 * @todo "HardwareSerial Serial(0)" 포트의 RxD로 데이터를 쓸 때, ESP32에서의 처리를 구현해야 합니다.
 * @details "loopTask" 내부에는 "HardwareSerial Serial(0)" 포트로 데이터가 들어오면 비록 비어있긴
 *          해도 "serialEvent(void)" 함수를 호출하고 있습니다. 
 */
}