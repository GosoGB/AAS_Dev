// /**
//  * @file RS485.h
//  * @author Lee, Sang-jin (lsj31@edgecross.ai)
//  * 
//  * @brief RS-485/422 시리얼 통신 인터페이스 클래스를 선언합니다.
//  * @note 본 클래스는 ANANLOG DEVICES 사의 MAX485 트랜스시버를 사용합니다.
//  * 
//  * @date 2024-10-19
//  * @version 0.0.1
//  * 
//  * @todo 김병우 수석께 DE/RE 핀 번호를 받아야 합니다.
//  * 
//  * @copyright Copyright (c) Edgecross Inc. 2024
//  */




// #if defined(MODLINK_L) || defined(MODLINK_ML10)


// #pragma once

// #include <HardwareSerial.h>
// #include <Stream.h>
// #include <sys/_stdint.h>

// #include "Common/Status.h"
// #include "Jarvis/Config/Interfaces/Rs485.h"



// namespace muffin {

//     class RS485 : public Stream
//     {
//     public:
//         RS485(RS485 const&) = delete;
//         void operator=(RS485 const&) = delete;
//         static RS485* GetInstanceOrNull();
//         static RS485& GetInstance();
//     private:
//         RS485();
//         virtual ~RS485();


//     public:
//         Status Config(jarvis::config::Rs485* config);
//     private:
//         static constexpr uint8_t DE_PIN_NUMBER =  4;
//         static constexpr uint8_t RE_PIN_NUMBER =  5;
//         static constexpr uint8_t TX_PIN_NUMBER = 17;
//     };
// }


// #endif