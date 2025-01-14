/**
 * @file Ethernet.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Ethernet 통신을 사용하는데 필요한 기능을 제공하는 클래스를 선언합니다.
 * 
 * @date 2024-09-04
 * @version 1.0.0
 * 
 * @todo Network 모듈 단에서 Ethernet 인터페이스의 MAC 주소를 어떻게 읽어가는
 * 것이 좋을지 결정하지 못하였습니다. 현재는 다음의 두 개의 방안을 고민 중입니다.
 *   - 방법 #1: GetMacAddress() 함수를 정적 메서드로 선언
 *   - 방법 #2: Ethernet() -> Init() -> GetMacAddress() 순서로 호출
 * 
 * @todo Arduino 프레임워크의 제약으로 인해 Disconnect(), Reconnect() 함수는 
 * 개발하지 않기로 결정하였습니다. 향후에 ESP-IDF 프레임워크로 이전하게 된다면
 * Disconnect(), Reconnect() 함수를 개발해야 합니다.
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <ETH.h>

#include "Network/INetwork.h"
#include "Jarvis/Config/Network/Ethernet.h"
#include "Common/Logger/Logger.h"



namespace muffin {

    class Ethernet : public INetwork
    {
    public:
        Ethernet();
        virtual ~Ethernet() override;
    private:
        eth_phy_type_t mPhyChipsetType = ETH_PHY_LAN8720;
        eth_clock_mode_t mPhyClockMode = ETH_CLOCK_GPIO0_IN;
        uint8_t mPhyMDIO = ETH_PHY_MDIO;
        uint8_t mPhyMDC = ETH_PHY_MDC;
        uint8_t mPhyAddress = 1;
        uint8_t mPhyPower = 32;
    private:
        char mMacAddress[13];
        char mHostname[18];

    public:
        typedef enum class EthernetFiniteStateMachineEnum
            : int8_t
        {
            FAILED_TO_START_PHY          = -2,
            FAILED_TO_CONFIGURE          = -1,
            NOT_INITIALIZED_YET          =  0,
            SUCCEDDED_TO_INITIALIZE      =  1,
            SUCCEDDED_TO_CONFIGURE       =  2,
            SUCCEDDED_TO_START_PHY       =  3,
            SUCCEDDED_TO_CONNECT         =  4,
            SUCCEDDED_TO_GET_IP_ADDRESS  =  5,
            ETHERNET_PHY_HAS_STOPPED     = 10,
            ETHERNET_PHY_DISCONNECTED    = 11
        } state_e;
    public:
        virtual Status Init() override;
        virtual Status Config(jarvis::config::Base* config) override;
        virtual Status Connect() override;
        virtual Status Disconnect() override;
        virtual Status Reconnect() override;
        virtual bool IsConnected() const override;
        virtual IPAddress GetIPv4() const override;
        const char* GetMacAddress() const;
        state_e GetState() const;
        Status SyncWithNTP();
    private:
        static void getArduinoEthernetEvent(arduino_event_id_t event);
    private:
        static state_e mState;
        bool mHasBegun = false;
        jarvis::config::Ethernet mConfig;
        bool mIsArduinoEventCallbackRegistered = false;
    };


    extern Ethernet* ethernet;
}