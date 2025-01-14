/**
 * @file TypeDefinitions.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief MUFFIN에서 사용하는 커스텀 정보 모델 데이터 타입들을 정의합니다.
 * 
 * @date 2025-01-14
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once



namespace muffin {

    typedef enum class McuTypeEnum
    {
        MCU_ESP32,
    #if defined(MODLINK_T2) || defined(MODLINK_B)
        MCU_ATmega2560
    #endif
    } mcu_type_e;
}