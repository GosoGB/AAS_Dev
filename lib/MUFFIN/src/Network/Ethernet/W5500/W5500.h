/**
 * @file W5500.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-18
 * 
 * @note 
 *      - Max socket: 8 simultaneously
 *      - Buffer size: 32KB for Send/Receive
 *      - Auto negotiation full, half and 10, 100-based
 *      - No IP fragmentation
 *      - SPI communication (Mode 0)
 *          - SPI speed: 80MHz Max.
 *          - SPI mode: 0 or 3 mode supported
 *          - Procedure
 *             - assert cs pin before starting a transmission
 *             - mosi signal is shifted out on falling edge while
 *               miso signal is being sampled on rising edge
 *             - deassert cs pin after transmission is finished
 *      - PHY
 *          - No auto MDI-X supported
 *             - Use straight cable for network switch or router.
 *             - Use crossover cable for network nodes except for switch and router.
 *             - If counterpart supports auto MDI-X, than both cables can be used.
 *          - 
 * 
 * @copyright Copyright (c) 2025
 */




#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <IPAddress.h>
#include <SPI.h>
#include <sys/_stdint.h>

#include "Common/Status.h"
#include "Include/TypeDefinitions.h"

constexpr uint32_t MHz = 1000 * 1000;



namespace muffin {


    namespace w5500 {
        class Socket;
    }


    class W5500
    {
    friend class w5500::Socket;
    
    public:
        W5500(const w5500::if_e idx);
        ~W5500() {}
    public:
        Status Init(const uint8_t mhz = 1);
    public:
        // Static IP configuration
        Status SetIPv4(const IPAddress ipv4);
        Status SetGateway(const IPAddress ipv4);
        Status SetSubnetmask(const IPAddress ipv4);
        Status SetDNS1(const IPAddress ipv4);
        Status SetDNS2(const IPAddress ipv4);
    private:
        Status setIPv4(const w5500::ipv4_type_e type, const IPAddress ipv4);
    public:
        Status GetMacAddress(uint8_t mac[]);
        Status GetMacAddress(char mac[]);
        uint16_t GetEphemeralPort();
    private:
        /**
         * @brief W5500 모듈을 리셋합니다.
         * @todo
         *        mRESET 핀이 내장된 모듈만 리셋할 수 있는 경우에는
         *        링크 모듈은 어떻게 리셋할지 생각해봐야 합니다.
         * 
         * @param[in]  none
         * @param[out] none
         * 
         * @return
         *      - void
         */
        void resetW5500();

        /**
         * @brief W5500 모듈의 SPI 통신을 초기화합니다.
         * @todo  통신 설정을 변경할 수 있는 기능을 추가해야 합니다.
         * 
         * @param[in]  mhz   SPI 통신 클럭
         * @param[out] none
         * 
         * @return
         *      - void
         */
        void initSPI(const uint8_t mhz);

        
        /**
         * @brief W5500 모듈에 MAC 주소를 설정합니다.
         * 
         * @param[in]  none
         * @param[out] none
         * 
         * @return
         *      - GOOD  MAC 주소가 정상적으로 설정됨
         *      - BAD_DEVICE_FAILURE  MAC 주소를 읽는 데 실패함
         *      - BAD_DEVICE_FAILURE  읽어들인 MAC 주소로 설정하는 데 실패함
         */
        Status setMacAddress();


    /**
     * @brief Read/Write 함수 쪽에 인터럽트나 상태 레지스터 보고서 상태를 결정하는 로직 추가할 필요 있겠음
     */
    private:
        Status read(const uint16_t address, const uint8_t control, uint8_t* output);
        Status read(const uint16_t address, const uint8_t control, const size_t length, uint8_t* output);
        Status retrieveCRB(const w5500::crb_addr_e address, uint8_t*  output);
        Status retrieveCRB(const w5500::crb_addr_e address, uint16_t* output);
        Status retrieveCRB(const w5500::crb_addr_e address, uint32_t* output);
        Status retrieveCRB(const w5500::crb_addr_e address, const size_t length, uint8_t* output);
        Status retrieveSRB(const w5500::sock_id_e idx, const w5500::srb_addr_e address, uint8_t*  output);
        Status retrieveSRB(const w5500::sock_id_e idx, const w5500::srb_addr_e address, uint16_t* output);
        Status retrieveSRB(const w5500::sock_id_e idx, const w5500::srb_addr_e address, uint32_t* output);
        Status retrieveSRB(const w5500::sock_id_e idx, const w5500::srb_addr_e address, const size_t length, uint8_t* output);
    private:
        Status write(const uint16_t address, const uint8_t control, const uint8_t input);
        Status write(const uint16_t address, const uint8_t control, const size_t length, const uint8_t input[]);
        Status writeCRB(const w5500::crb_addr_e address, const uint8_t  data);
        Status writeCRB(const w5500::crb_addr_e address, const uint16_t data);
        Status writeCRB(const w5500::crb_addr_e address, const uint32_t data);
        Status writeCRB(const w5500::crb_addr_e address, const size_t length, const uint8_t data[]);
        Status writeSRB(const w5500::sock_id_e idx, const w5500::srb_addr_e address, const uint8_t  data);
        Status writeSRB(const w5500::sock_id_e idx, const w5500::srb_addr_e address, const uint16_t data);
        Status writeSRB(const w5500::sock_id_e idx, const w5500::srb_addr_e address, const uint32_t data);
        Status writeSRB(const w5500::sock_id_e idx, const w5500::srb_addr_e address, const size_t length, const uint8_t data[]);
    private:
        const uint8_t mCS;
        static const uint8_t mMOSI     = 11;
        static const uint8_t mMISO     = 13;
        static const uint8_t mSCLK     = 12;
        static const uint8_t mRESET    = 48;
        static SPISettings mSpiConfig;
    private:
        static constexpr uint16_t DEFAULT_EPHEMERAL_PORT = 0xC000;
        uint16_t mEphemeralPort = DEFAULT_EPHEMERAL_PORT;
    private:
        w5500::crb_t mCRB;
        bool mHasMacAddress = false;
        SemaphoreHandle_t xSemaphore;
        IPAddress mDNS1;
        IPAddress mDNS2;
    };
}