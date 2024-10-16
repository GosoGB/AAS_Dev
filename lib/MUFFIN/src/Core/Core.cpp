// /**
//  * @file Core.cpp
//  * @author Lee, Sang-jin (lsj31@edgecross.ai)
//  * 
//  * @brief MUFFIN 프레임워크 내부의 핵심 기능을 제공하는 클래스를 정의합니다.
//  * 
//  * @date 2024-10-16
//  * @version 0.0.1
//  * 
//  * @copyright Copyright (c) Edgecross Inc. 2024
//  */




// #include "Common/Assert.h"
// #include "Common/Logger/Logger.h"

// #include "Core.h"
// #include "IM/MacAddress/MacAddress.h"

// #include "Include/Helper.h"

// #include "Jarvis/Config/Network/CatM1.h"
// #if defined(MODLINK_T2)
//     #include "Jarvis/Config/Network/Ethernet.h"
// #elif defined(MODLINK_B)
//     #include "Jarvis/Config/Network/Ethernet.h"
//     #include "Jarvis/Config/Network/WiFi4.h"
// #endif
// #include "Jarvis/Jarvis.h"

// #include "Network/CatM1/CatM1.h"
// #if defined(MODLINK_T2)
//     #include "Network/Ethernet/Ethernet.h"
// #elif defined(MODLINK_B)
//     #include "Network/Ethernet/Ethernet.h"
//     #include "Network/WiFi4/WiFi4.h"
// #endif

// #include "Protocol/MQTT/BrokerInfo.h"
// #include "Protocol/MQTT/CatMQTT/CatMQTT.h"

// #include "Protocol/HTTP/CatHTTP/CatHTTP.h"
// #include "Protocol/HTTP/Include/RequestHeader.h"

// #include "Storage/ESP32FS/ESP32FS.h"



// namespace muffin {

//     Core& Core::GetInstance() noexcept
//     {
//         if (mInstance == nullptr)
//         {
//             logger = new(std::nothrow) muffin::Logger();
//             if (logger == nullptr)
//             {
//                 esp_restart();
//             }
            
//             mInstance = new(std::nothrow) Core();
//             if (mInstance == nullptr)
//             {
//                 LOG_ERROR(logger, "FAILED TO ALLOCATE MEMROY FOR MUFFIN CORE");
//                 esp_restart();
//             }
//         }
        
//         return *mInstance;
//     }

//     Core::Core()
//     {
//     #if defined(DEBUG)
//         LOG_VERBOSE(logger, "Constructed at address: %p", this);
//     #endif
//     }
    
//     Core::~Core()
//     {
//     #if defined(DEBUG)
//         LOG_VERBOSE(logger, "Constructed at address: %p", this);
//     #endif
//     }

//     Status Core::Init()
//     {
//         /**
//          * @todo Reset 사유에 따라 자동으로 초기화 하는 기능의 개발이 필요합니다.
//          * @details JARVIS 설정으로 인해 런타임에 크래시 같은 문제가 있을 수 있습니다.
//          *          이러한 경우에는 계속해서 반복적으로 MODLINK가 리셋되는 현상이 발생할
//          *          수 있습니다. 따라서 reset 사유를 확인하여 JARVIS 설정을 초기화 하는
//          *          기능이 필요합니다. 단, 다른 부서와의 협의가 선행되어야 합니다.
//          */
//         mResetReason = esp_reset_reason();
//         printResetReason(mResetReason);

//         MacAddress* mac = MacAddress::GetInstance();
//         if (mac == nullptr)
//         {
//             ASSERT(mac != nullptr, "FATAL ERROR OCCURED: FAILED TO READ MAC ADDRESS DUE TO MEMORY OR DEVICE FAILURE");
            
//             LOG_ERROR(logger, "FAILED TO READ MAC ADDRESS DUE TO MEMORY OR DEVICE FAILURE");
//             esp_restart();
//         }

//         ESP32FS* esp32FS = ESP32FS::GetInstance();
//         if (esp32FS == nullptr)
//         {
//             ASSERT(esp32FS != nullptr, "FATAL ERROR OCCURED: FAILED TO ALLOCATE MEMORY FOR ESP32 FILE SYSTEM");

//             LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR ESP32 FILE SYSTEM");
//             esp_restart();
//         }
        
//         /**
//          * @todo LittleFS 파티션의 포맷 여부를 로우 레벨 API로 확인해야 합니다.
//          * @details 현재는 파티션 마운트에 실패할 경우 파티션을 자동으로 포맷하게
//          *          코드를 작성하였습니다. 다만, 일시적인 하드웨어 실패가 발생한
//          *          경우에도 파티션이 포맷되는 문제가 있습니다.
//          */
//         if (esp32FS->Begin(true) != Status::Code::GOOD)
//         {
//             ASSERT(false, "FATAL ERROR OCCURED: FAILED TO MOUNT ESP32 FILE SYSTEM TO OPERATING SYSTEM");

//             LOG_ERROR(logger, "FAILED TO MOUNT ESP32 FILE SYSTEM TO THE OS");
//             esp_restart();
//         }

        
//         if (esp32FS->DoesExist("/jarvis/config.json") == Status::Code::GOOD)
//         {
//             initWithJARVIS();
//         }
//         else
//         {
//             initWithoutJARVIS();
//         }

//         return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
//     }

//     Status Core::initWithoutJARVIS()
//     {
//         /**
//          * @todo Option #1: MODLINK 모델 별로 기본 설정이 달라지는 부분을 고려해야 합니다.
//          * @todo Option #2: 블루투스를 이용해서 사용자가 설정할 수 있게 만들어야 합니다.
//          */
//         jarvis::config::CatM1* config = new(std::nothrow) jarvis::config::CatM1();
//         if (config == nullptr)
//         {
//             return Status(Status::Code::BAD_OUT_OF_MEMORY);
//         }
//         config->SetModel(jarvis::md_e::LM5);
//         config->SetCounty(jarvis::ctry_e::KOREA);


//         CatM1* catM1 = CatM1::GetInstance();
//         if (config == nullptr)
//         {
//             return Status(Status::Code::BAD_OUT_OF_MEMORY);
//         }
//         catM1->Config(config);
//         catM1->Init();

//         while (catM1->GetState() != CatM1::state_e::SUCCEDDED_TO_START)
//         {
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//         }

//         while (catM1->Connect() != Status::Code::GOOD)
//         {
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//         }

//         mqtt::BrokerInfo info(mMacAddressEthernet.c_str());
//     #if defined(DEBUG)
//         /**
//          * @brief DEBUGGING 환경에서 "일시적"으로 사용하기 때문에 
//          *        상태 코드를 확인 작업을 구현하지 않았습니다.
//          */
//         info.SetHost("mqtt.vitcon.iotops.opsnow.com");
//         info.SetUsername("vitcon");
//         info.SetPassword("tkfkdgo5!@#$");
//     #endif

//         mqtt::CatMQTT* catMQTT = mqtt::CatMQTT::GetInstance(*catM1, info);
//         Status ret = catMQTT->Init(network::lte::pdp_ctx_e::PDP_01, network::lte::ssl_ctx_e::SSL_0);
//         if (ret != Status::Code::GOOD)
//         {
//             LOG_ERROR(logger, "FAILED TO INIT CatMQTT: %s", ret.c_str());
//             return ret;
//         }
        
//         while (catMQTT->Connect() != Status::Code::GOOD)
//         {
//             vTaskDelay(3000 / portTICK_PERIOD_MS);
//         }

//         std::vector<mqtt::Message> vec;
//         mqtt::Message message(mMacAddressEthernet, mqtt::topic_e::JARVIS_REQUEST, "");
//         /**
//          * @todo emplace_back에 대한 예외처리 구현해야 합니다.
//          */
//         vec.emplace_back(message);
//         ret = catMQTT->Subscribe(vec);
//         if (ret != Status::Code::GOOD)
//         {
//             LOG_ERROR(logger, "FAILED TO SUBSCRIBE JARVIS TOPIC: %s", ret.c_str());
//             return ret;
//         }

//     /*  JARVIS 없을 때 설정하는 건 CatMQTT 초기화까지만 하면 될 거 같음

//         http::CatHTTP* catHTTP = http::CatHTTP::GetInstance(*catM1);
//         ret = catHTTP->Init(network::lte::pdp_ctx_e::PDP_01, network::lte::ssl_ctx_e::SSL_0);
//         if (ret != Status::Code::GOOD)
//         {
//             LOG_ERROR(logger, "FAILED TO INIT CatHTTP: %s", ret.c_str());
//             return ret;
//         }

//         http::RequestHeader header(
//             rest_method_e::GET,
//             http_scheme_e::HTTPS,
//             "api.mfm.edgecross.dev",
//             443,
//             "/api/mfm/device/write",
//             "MODLINK-L/0.0.1"
//         );

//         catHTTP->GET(header);
//     */

//         return ret;
//     }

//     Status Core::initWithJARVIS()
//     {
//         return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
//     }


//     Core* Core::mInstance = nullptr;
// }