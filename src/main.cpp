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
    config.SetStaticIPv4(IPAddress(10, 11, 12, 119));
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

    // ESP32 is little endian 확인
    uint16_t test = 0x0065;
    uint16_t result = ntohs(test);
    Serial.printf("[DEBUG] loop start........\n");
    Serial.printf("[DEBUG] ntohs(0x%04X) = 0x%04X\n\n\n", test, result);

    if (!session.client.connected()) {
        Serial.println("[CIP] 연결 끊김 감지됨. 재연결 시도 중...");

        // 소켓 다시 초기화 (선택 사항: 필요시)
        //EthernetClient newClient(ethernetChip, sock_id_e::SOCKET_0);
        //session.client = newClient;

        if (!eipInit(session, session.targetIP, session.targetPort)) {
            Serial.println("[CIP] 재연결: eipInit 실패");
            session.client.stop();  // 실패 시 다시
            delay(2000);
            return;
        }

        if (!registerSession(session)) {
            Serial.println("[CIP] 재연결: RegisterSession 실패");
            session.client.stop();  // 실패 시 다시
            delay(2000);
            return;
        }

        Serial.print("[CIP] 세션 핸들 등록 완료: ");
        Serial.println(session.sessionHandle, HEX);  // 16진수 출력

/*      forwardOpen ( 클래스 3에서는 사용 안함 )
        if (!forwardOpen(session)) {
            Serial.println("[CIP] 재연결: ForwardOpen 실패");
            session.client.stop();  // 실패 시 다시
            delay(2000);
            return;
        }
*/
        Serial.println("[CIP] 세션 재연결 성공");
    }

    // 세션 핸들 확인
    Serial.print("[CIP] 현재 세션 핸들: ");
    Serial.println(session.sessionHandle, HEX);    

/*
    test_read(session);
    delay(6000);
    test_write(session);    
    delay(6000);
*/

    // 20250629 테스트
    // Tag 일기

    uint32_t startTS = millis();
    size_t countIdx = 0;
    while (countIdx < 1)
    {
        // // 읽고자 하는 태그 이름들
        // std::vector<std::string> readTags = { 
        //     "max_read_1", 
        //     "max_read_2", 
        //     "max_read_3",
        //     "max_read_4",
        //     "max_read_5",
        //     "max_read_6",
        //     "max_read_7",
        //     "max_read_8",
        //     "max_read_9",
        //     "max_read_10",
        //     "max_read_11",
        //     "max_read_12",
        //     "max_read_13",
        //     "max_read_14",
        //     "max_read_15",
        //     "max_read_16",
        //     "max_read_17",
        //     "Type_SINT_A1D[3]",
        //     "Type_SINT_A1D[4]",
        //     "Type_SINT_A1D[5]"
        // };

        // 결과 벡터
        // std::vector<cip_data_t> readValues;

        // if (readTagsMSR(session, readTags, readValues)) {
        //     // Serial.printf("Multi-tag read success.\n");

        //     for (size_t i = 0; i < readValues.size(); ++i) {
        //         const auto& d = readValues[i];
        //         // Serial.printf("Tag %zu (%s): ", i, readTags[i].c_str());

        //         if (d.Code == 0x00) {
        //             // Serial.printf("Write success...");                
        //             // printCipData(d);
        //         } else {
        //             // 오류 응답
        //             Serial.printf("Error Code=0x%02X\n", d.Code);
        //         }
        //     }
        //     countIdx++;
        // } else {
        //     Serial.printf("Multi-tag read failed.\n");
        // }

        // std::string tagName = "Type_UDT";  // 읽을 태그 이름
        // cip_data_t readData;
        // // Serial.printf("========================================================================\n");
        // // Serial.printf("=== Reading single Tag: %s \n", tagName.c_str());
        // // Serial.printf("========================================================================\n");
        // if (!readTag(session, tagName, readData)) {
        //     Serial.printf("[readSingleTag] Read FAILED, General Status = 0x%02X\n", readData.Code);
        // } else {
        //     // 응답은 왔음 -> Code 검사
        //     if (readData.Code == 0x00) {
        //         printCipData(readData);
        //         countIdx++;
        //     } else {
        //         Serial.printf("Error Code = 0x%02X\n", readData.Code);
        //     }
        // }
        std::vector<cip_data_t> readResults;
        std::string tagName = "Type_REAL_A3D";
        // Serial.printf("========================================================================\n");
        // Serial.printf("=== Reading array Tag: %s\n", tagName.c_str());
        // Serial.printf("========================================================================\n");

        // 배열 요소 5개 읽기
        if (!readTagExt( session, tagName, 0, 3,readResults)) {
            Serial.printf("[readTagExt] Read FAILED\n");
        } else {
            // Serial.printf("[readTagExt] Read OK. Elements: %zu\n", readResults.size());

            for (size_t i = 0; i < readResults.size(); ++i) {
                const cip_data_t& elem = readResults[i];

                if (elem.Code != 0) {
                    Serial.printf(" Element[%zu] ERROR (Code=0x%02X)\n", i, elem.Code);
                } else {
                    Serial.printf(" Element[%zu]: ", i);
                    printCipData(elem);  // 값 출력
                }
            }
        }
        countIdx++;
    }

    LOG_INFO(logger,"POLLING TIME : %lu", millis() - startTS);
    
    // // 잠시 대기
    // delay(4000);
    

    // // 결과 저장
    

    // // 잠시 대기
    // delay(4000);
    // 단일 데이터 쓰기
    // std::vector<uint8_t> writeValue = {0xD2, 0x04, 0x00, 0x00}; // 1234 = 0x000004D2 (리틀 엔디안)

    // cip_data_t writeResult;
    // tagName = "Tag2";

    // Serial.printf("========================================================================\n");
    // Serial.printf("=== Writing Tag: %s \n", tagName.c_str());
    // Serial.printf("========================================================================\n");
    // Serial.printf("Value to write (DINT): %ld\n", writeValue);

    // if (!writeTag(session, tagName, writeValue, static_cast<uint16_t>(CipDataType::DINT), writeResult)) {
    //     Serial.printf("[writeSingleTag] Write FAILED, General Status = 0x%02X\n", writeResult.Code);
    // } else {
    //     // 응답 수신됨 -> 상태 코드 검사
    //     if (writeResult.Code == 0x00) {
    //         Serial.printf("[writeSingleTag] Write OK.\n");
    //     } else {
    //         Serial.printf("[writeSingleTag] Write ERROR, Code = 0x%02X\n", writeResult.Code);
    //     }        
    // }

    // // 잠시 대기
    // delay(4000);
    // // 배열에 데이터 쓰기
    // // DINT 값들
    // int32_t values[5] = {100, 200, 300, 400, 500};

    // // 데이터를 바이트 벡터로 직렬화
    // //std::vector<uint8_t> writeValue;
    // writeValue.clear();
    // for (int i = 0; i < 5; ++i) {
    //     int32_t v = values[i];
    //     writeValue.push_back(static_cast<uint8_t>(v & 0xFF));
    //     writeValue.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    //     writeValue.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    //     writeValue.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
    // }

    // // 결과 저장
    // //cip_data_t writeResult;

    // tagName = "TagArray[0]";

    // Serial.printf("========================================================================\n");    
    // Serial.printf("=== Writing array Tag: %s\n", tagName.c_str());
    // Serial.printf("========================================================================\n");    
    // Serial.printf("Values to write (DINT x5): {100, 200, 300, 400, 500}\n");

    // // 배열 요소 5개 쓰기
    // if (!writeTagExt( session, tagName, writeValue, static_cast<uint16_t>(CipDataType::DINT), 5, writeResult)) {
    //     Serial.printf("[writeTagExt] Write FAILED, General Status = 0x%02X\n", writeResult.Code);
    // } else {
    //     if (writeResult.Code == 0x00) {
    //         Serial.printf("[writeTagExt] Write OK.\n");
    //     } else {
    //         Serial.printf("[writeTagExt] Write ERROR, Code = 0x%02X\n", writeResult.Code);
    //     }
    // }

    // 잠시 대기
    // delay(4000);
    // Serial.printf("========================================================================\n");    
    // Serial.printf("=== Reading MSR Tag\n");
    // Serial.printf("========================================================================\n");        

    // // 읽고자 하는 태그 이름들
    // //std::vector<std::string> readTags = { "Tag1", "Tag2", "Flag1" };
    // std::vector<std::string> readTags = { "Type_REAL", "Type_LINT"};

    // // 결과 벡터
    // std::vector<cip_data_t> readValues;
    // if (readTagsMSR(session, readTags, readValues)) {
    //     Serial.printf("Multi-tag read success.\n");

    //     for (size_t i = 0; i < readValues.size(); ++i) {
    //         const auto& d = readValues[i];
    //         Serial.printf("Tag %zu (%s): ", i, readTags[i].c_str());

    //         if (d.Code == 0x00) {
    //             Serial.printf("Write success...");                
    //             printCipData(d);
    //         } else {
    //             // 오류 응답
    //             Serial.printf("Error Code=0x%02X\n", d.Code);
    //         }
    //     }
    // } else {
    //     Serial.printf("Multi-tag read failed.\n");
    // }


    // // 잠시 대기
    // delay(4000);

    // Serial.printf("========================================================================\n");    
    // Serial.printf("=== Writing MSR Tag\n");
    // Serial.printf("========================================================================\n");        


    // // 쓰기 대상 태그 이름들
    // std::vector<std::string> writeTags = { "Tag1_test", "Tag1_test1", "Tag1_test2"};

    // // 각 태그에 쓸 값
    // std::vector<std::vector<uint8_t>> writeValues = {
    //     {0x78, 0x56, 0x34, 0x12},  // Tag1 = DINT 0x12345678
    //     {0x11, 0x22, 0x33, 0x44},  // Tag2 = DINT 0x44332211
    //     {0x01}                     // Flag1 = BOOL true
    // };

    // // 각 태그의 데이터 타입
    // std::vector<uint16_t> dataTypes = {
    //     0xC4,  // CIP_DINT
    //     0xC4,  // CIP_DINT
    //     0xC1   // CIP_BOOL
    // };

    // // 결과 벡터
    // std::vector<cip_data_t> writeResults;

    // // 실행
    // if (writeTagsMSR(session, writeTags, writeValues, dataTypes, writeResults)) {
    //     Serial.printf("Multi-tag write completed.\n");

    //     // 결과 출력
    //     for (size_t i = 0; i < writeResults.size(); ++i) {
    //         const auto& d = writeResults[i];
    //         Serial.printf("Tag %s: ", writeTags[i].c_str());
    //         if (d.Code == 0x00) {
    //             Serial.printf("Write success.\n");
    //             printCipData(d);
    //         } else {
    //             Serial.printf("Write failed (Error=0x%02X)\n", d.Code);
    //         }
    //     }
    // } else {
    //     Serial.printf("Multi-tag write failed.\n");
    // }

    session.client.stop();



#if defined(DEBUG)
    LOG_DEBUG(muffin::logger, "Remained Heap: %u Bytes", ESP.getFreeHeap());
    LOG_DEBUG(muffin::logger, "Remained Flash: %u Bytes", LittleFS.totalBytes() - LittleFS.usedBytes());
#if defined(MT11)
    LOG_DEBUG(muffin::logger, "Remained PSRAM : %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
#endif
    vTaskDelay(1000 / portTICK_PERIOD_MS);
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