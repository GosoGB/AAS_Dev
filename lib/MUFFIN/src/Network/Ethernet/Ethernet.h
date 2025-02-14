/**
 * @file Ethernet.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Ethernet 통신을 사용하는데 필요한 기능을 제공하는 클래스를 선언합니다.
 * 
 * @date 2025-01-23
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 * 
 * 
 * @todo Network 모듈 단에서 Ethernet 인터페이스의 MAC 주소를 어떻게 읽어가는
 * 것이 좋을지 결정하지 못하였습니다. 현재는 다음의 두 개의 방안을 고민 중입니다.
 *   - 방법 #1: GetMacAddress() 함수를 정적 메서드로 선언
 *   - 방법 #2: Ethernet() -> Init() -> GetMacAddress() 순서로 호출
 * 
 * @todo Arduino 프레임워크의 제약으로 인해 Disconnect(), Reconnect() 함수는 
 * 개발하지 않기로 결정하였습니다. 향후에 ESP-IDF 프레임워크로 이전하게 된다면
 * Disconnect(), Reconnect() 함수를 개발해야 합니다.
 */




#if defined(MODLINK_T2) || defined(MODLINK_B)

#pragma once

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/DataStructure/bitset.h"
#include "Include/DeprecableEthernet.h"
#include "JARVIS/Config/Network/Ethernet.h"
#include "Network/INetwork.h"



namespace muffin {

    class Ethernet : public INetwork
    {
    public:
        Ethernet();
        virtual ~Ethernet() override {}
    public:
        virtual Status Init() override;
        virtual Status Config(jvs::config::Base* config) override;
        virtual Status Connect() override;
        virtual Status Disconnect() override;
        virtual Status Reconnect() override;
        virtual bool IsConnected() const override;
        virtual IPAddress GetIPv4() const override;
        virtual Status SyncNTP() override;
        virtual std::pair<Status, size_t> TakeMutex() override;
        virtual Status ReleaseMutex() override;
    private:
        static void getArduinoEthernetEvent(arduino_event_id_t event);
        void implArduinoCallback(arduino_event_id_t event);

    private:
        typedef enum class InitFlagEnum : uint8_t
        {
            INITIALIZED  = 0,
            CONFIGURED   = 1,
            HAS_STARTED  = 2,
            HAS_IPv4     = 3,
            TOP          = 4
        } flag_e;
        bitset<static_cast<uint8_t>(flag_e::TOP)> mFlogs;
    private:
        const eth_phy_type_t mPhyChipsetType = ETH_PHY_LAN8720;
        const eth_clock_mode_t mPhyClockMode = ETH_CLOCK_GPIO0_IN;
        const uint8_t mPhyMDIO = ETH_PHY_MDIO;
        const uint8_t mPhyMDC = ETH_PHY_MDC;
        const uint8_t mPhyAddress = 1;
        const uint8_t mPhyPower = 32;
    };


    extern Ethernet* ethernet;
}

#endif