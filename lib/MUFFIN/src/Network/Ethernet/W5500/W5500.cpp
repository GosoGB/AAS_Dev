#if defined(MT11)

/**
 * @file W5500.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-05-29
 * @version 1.4.0
 * 
 * @note
 *      Ver.1.4.0 기준, W5500 링크는 LINK #1 위치에만 위치할 수 있다는 가정하에 코드를 작성하였습니다.
 *      만약 LINK #2에 W5500 링크 설치가 가능하게 변경된다면 mRESET 핀 설정 부분을 수정해야 합니다.
 * 
 * @copyright Copyright (c) Edgecross Inc. 2025
 */




#include <esp32-hal-gpio.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Include/Converter.h"
#include "Include/Microchip24AA02E.h"
#include "JARVIS/Config/Network/Ethernet.h"
#include "Socket.h"
#include "W5500.h"

SPISettings muffin::W5500::mSpiConfig;



namespace muffin {

    
    W5500::W5500(const w5500::if_e idx)
        : mCS(static_cast<uint8_t>(idx))
        , mRESET(idx == w5500::if_e::EMBEDDED ? 48 : 38)
        , xSemaphore(NULL)
    {
        pinMode(mCS,     OUTPUT);
        pinMode(mMOSI,   OUTPUT);
        pinMode(mSCLK,   OUTPUT);
        pinMode(mRESET,  OUTPUT);
        pinMode(mMISO,   INPUT);
        
        digitalWrite(mCS,     HIGH);
        digitalWrite(mRESET,  HIGH);
        
        memset(&mCRB, 0, sizeof(mCRB));
    }


    /**
     * @todo MUTEX 적용이 필요한지, 범위는 어떻게 되어야 하는지 고민이 필요함
     *       예: 다른 SPI 디바이스와의 경쟁상황 발생 가능성 등
     */
    Status W5500::Init()
    {
        if (xSemaphore == NULL)
        {
            xSemaphore = xSemaphoreCreateMutex();
            if (xSemaphore == NULL)
            {
                LOG_ERROR(logger, "FAILED TO CREATE MUTEX");
                return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            }
        }

        resetW5500();
        initSPI(mMHz);

        Status ret = setMacAddress();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET MAC ADDRESS: %s", ret.c_str());
            return ret;
        }
        
        return ret;
    }


    Status W5500::Config(jvs::config::Base* config)
    {
        ASSERT((config != nullptr), "INPUT PARAM <jvs::config::Base* config> CANNOT BE NULL");
        ASSERT((config->GetCategory() == jvs::cfg_key_e::ETHERNET), "INVALID JARVIS CATEGORY");

        jvs::config::Ethernet* cin = static_cast<jvs::config::Ethernet*>(config);
        if (cin->GetDHCP().second == true)
        {
            memset(mCRB.IPv4, 0, sizeof(mCRB.IPv4));
            memset(mCRB.Gateway, 0, sizeof(mCRB.Gateway));
            memset(mCRB.Subnetmask, 0, sizeof(mCRB.Subnetmask));
        }
        else
        {
            mCRB.IPv4[0] = cin->GetStaticIPv4().second[0];
            mCRB.IPv4[1] = cin->GetStaticIPv4().second[1];
            mCRB.IPv4[2] = cin->GetStaticIPv4().second[2];
            mCRB.IPv4[3] = cin->GetStaticIPv4().second[3];

            mCRB.Gateway[0] = cin->GetGateway().second[0];
            mCRB.Gateway[1] = cin->GetGateway().second[1];
            mCRB.Gateway[2] = cin->GetGateway().second[2];
            mCRB.Gateway[3] = cin->GetGateway().second[3];

            mCRB.Subnetmask[0] = cin->GetSubnetmask().second[0];
            mCRB.Subnetmask[1] = cin->GetSubnetmask().second[1];
            mCRB.Subnetmask[2] = cin->GetSubnetmask().second[2];
            mCRB.Subnetmask[3] = cin->GetSubnetmask().second[3];

            mDNS1 = cin->GetDNS1().second;
            mDNS2 = cin->GetDNS2().second;
        }

        return Status(Status::Code::GOOD);
    }


    Status W5500::Connect()
    {
        Status ret(Status::Code::UNCERTAIN);

        if (mCRB.IPv4[0] == 0 && mCRB.IPv4[1] == 0 && mCRB.IPv4[2] == 0 && mCRB.IPv4[3] == 0)
        {
            if (mDHCP == nullptr)
            {
                w5500::Socket socket(*this, w5500::sock_id_e::SOCKET_7, w5500::sock_prtcl_e::UDP);
                mDHCP = new w5500::DHCP(socket);
            }
            
            ret = mDHCP->Init();
            if (ret != muffin::Status::Code::GOOD)
            {
                LOG_ERROR(muffin::logger, "FAILED TO INITIALIZE DHCP CLIENT: %s", ret.c_str());
                return ret;
            }

            /**
             * @todo Run 함수는 김주성 전임이 Task로 변환하는 작업 진행 중이고 그게 끝나면 DHCP.cpp 파일 교체해야 함
             *       실행 결과도 확인할 것
             */
            ret = mDHCP->Run();
        }
        

        /**
         * @todo 실행 결과 확인할 것
         */
        ret = setLocalIP(IPAddress(mCRB.IPv4[0], mCRB.IPv4[1], mCRB.IPv4[2], mCRB.IPv4[3]));
        ret = setGateway(IPAddress(mCRB.Gateway[0], mCRB.Gateway[1], mCRB.Gateway[2], mCRB.Gateway[3]));
        ret = setSubnetmask(IPAddress(mCRB.Subnetmask[0], mCRB.Subnetmask[1], mCRB.Subnetmask[2], mCRB.Subnetmask[3]));
    }


    Status W5500::Disconnect()
    {
        return Status(Status::Code::BAD_NOT_IMPLEMENTED);
    }


    Status W5500::Reconnect()
    {
        return Status(Status::Code::BAD_NOT_IMPLEMENTED);
    }

    
    bool W5500::getLinkStatus()
    {
        uint8_t retrievedPHY = 0;

        Status ret = retrieveCRB(w5500::crb_addr_e::PHY_CONFIGURATION, sizeof(retrievedPHY), &retrievedPHY);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE PHY REGISTER: %s", ret.c_str());
            return false;
        }
        
        return retrievedPHY & 0x01;
    }


    bool W5500::IsConnected()
    {
        const bool isUp = getLinkStatus();
        return isUp;
    }


    IPAddress W5500::GetIPv4() const
    {
        return INADDR_NONE;
    }


    Status W5500::SyncNTP()
    {
        return Status(Status::Code::BAD_NOT_IMPLEMENTED);
    }

    std::pair<Status, size_t> W5500::TakeMutex()
    {
        return std::make_pair(Status(Status::Code::GOOD), 1);
    }

    
    Status W5500::ReleaseMutex()
    {
        return Status(Status::Code::GOOD);
    }


    void W5500::resetW5500()
    {
        digitalWrite(mRESET, LOW);
        delay(140);
        digitalWrite(mRESET, HIGH);
        delay(100);
    }


    void W5500::initSPI(const uint8_t mhz)
    {
        SPI.begin(mSCLK, mMISO, mMOSI, mCS);
        mSpiConfig._clock     = mhz * MHz;
        mSpiConfig._bitOrder  = SPI_MSBFIRST;
        mSpiConfig._dataMode  = SPI_MODE0;
    }
    

    Status W5500::setMacAddress()
    {   
        Microchip24AA02E microchip;
        const bool isLink = w5500::if_e::EMBEDDED != static_cast<w5500::if_e>(mCS);

        if (microchip.Read(isLink, mCRB.MAC) == false)
        {
            LOG_ERROR(logger, "FAILED TO READ MAC ADDRESS");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
    
        Status ret = writeCRB(w5500::crb_addr_e::MAC, sizeof(mCRB.MAC), mCRB.MAC);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET MAC ADDRESS");
            return ret;
        }
        
        uint8_t retrievedMAC[6] = { 0 };
        ret = retrieveCRB(w5500::crb_addr_e::MAC, sizeof(retrievedMAC), retrievedMAC);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE MAC ADDRESS");
            return ret;
        }

        if (memcmp(mCRB.MAC, retrievedMAC, sizeof(mCRB.MAC)) != 0)
        {
            log_e("FAILED TO SET MAC ADDRESS");
            return Status(Status::Code::BAD_COMMUNICATION_ERROR);
        }
        
        LOG_INFO(logger, "MAC: %02X:%02X:%02X:%02X:%02X:%02X", 
            mCRB.MAC[0], mCRB.MAC[1], mCRB.MAC[2], mCRB.MAC[3], mCRB.MAC[4], mCRB.MAC[5]);
        
        mHasMacAddress = true;
        return ret;
    }


    Status W5500::setLocalIP(const IPAddress ipv4)
    {
        return setIPv4(w5500::ipv4_type_e::SOURCE_IPv4, ipv4);
    }


    Status W5500::setGateway(const IPAddress ipv4)
    {
        return setIPv4(w5500::ipv4_type_e::GATEWAY, ipv4);
    }


    Status W5500::setSubnetmask(const IPAddress ipv4)
    {
        return setIPv4(w5500::ipv4_type_e::SUBNETMASK, ipv4);
    }


    Status W5500::setDNS1(const IPAddress ipv4)
    {
        mDNS1 = ipv4;
        return Status(Status::Code::BAD_NOT_IMPLEMENTED);
    }


    Status W5500::setDNS2(const IPAddress ipv4)
    {
        mDNS2 = ipv4;
        return Status(Status::Code::BAD_NOT_IMPLEMENTED);
    }


    Status W5500::setIPv4(const w5500::ipv4_type_e type, const IPAddress ipv4)
    {
        const uint32_t decimalIPv4 = uint32_t(ipv4);
        Status ret(Status::Code::UNCERTAIN);
        
        uint8_t IPv4[4] = { 0 };
        uint8_t retrievedIPv4[4] = { 0 };
        memcpy(IPv4, &decimalIPv4, sizeof(IPv4));

        switch (type)
        {
        case w5500::ipv4_type_e::SOURCE_IPv4:
            ret = writeCRB(w5500::crb_addr_e::IPv4, sizeof(IPv4), IPv4);
            break;
        case w5500::ipv4_type_e::GATEWAY:
            ret = writeCRB(w5500::crb_addr_e::GATEWAY, sizeof(IPv4), IPv4);
            break;
        default:
            ret = writeCRB(w5500::crb_addr_e::SUBNETMASK, sizeof(IPv4), IPv4);
            break;
        }
        
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET IPv4 ADDRESS: %s", ret.c_str());
            return ret;
        }

        switch (type)
        {
        case w5500::ipv4_type_e::SOURCE_IPv4:
            ret = retrieveCRB(w5500::crb_addr_e::IPv4, sizeof(retrievedIPv4), retrievedIPv4);
            break;
        case w5500::ipv4_type_e::GATEWAY:
            ret = retrieveCRB(w5500::crb_addr_e::GATEWAY, sizeof(retrievedIPv4), retrievedIPv4);
            break;
        default:
            ret = retrieveCRB(w5500::crb_addr_e::SUBNETMASK, sizeof(retrievedIPv4), retrievedIPv4);
            break;
        }
        
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE IPv4 ADDRESS: %s", ret.c_str());
            return ret;
        }

        LOG_DEBUG(logger, "IPv4: %u.%u.%u.%u", IPv4[0], IPv4[1], IPv4[2], IPv4[3]);
        LOG_DEBUG(logger, "IPv4: %u.%u.%u.%u", retrievedIPv4[0], retrievedIPv4[1], retrievedIPv4[2], retrievedIPv4[3]);

        if (memcmp(IPv4, retrievedIPv4, sizeof(IPv4)) != 0)
        {
            LOG_ERROR(logger, "RETRIEVED IPv4 ADDRESS DOES NOT MATCH");
            return Status(Status::Code::BAD_COMMUNICATION_ERROR);
        }

        switch (type)
        {
        case w5500::ipv4_type_e::SOURCE_IPv4:
            memcpy(mCRB.IPv4, IPv4, sizeof(mCRB.IPv4));
            LOG_INFO(logger, "IPv4: %u.%u.%u.%u", mCRB.IPv4[0], mCRB.IPv4[1], mCRB.IPv4[2], mCRB.IPv4[3]);
            break;
        case w5500::ipv4_type_e::GATEWAY:
            memcpy(mCRB.Gateway, IPv4, sizeof(mCRB.Gateway));
            LOG_INFO(logger, "Gateway: %u.%u.%u.%u", mCRB.Gateway[0], mCRB.Gateway[1], mCRB.Gateway[2], mCRB.Gateway[3]);
            break;
        default:
            memcpy(mCRB.Subnetmask, IPv4, sizeof(mCRB.Subnetmask));
            LOG_INFO(logger, "Subnetmask: %u.%u.%u.%u", mCRB.Subnetmask[0], mCRB.Subnetmask[1], mCRB.Subnetmask[2], mCRB.Subnetmask[3]);
            break;
        }
        
        return Status(Status::Code::GOOD);
    }


    Status W5500::GetMacAddress(uint8_t mac[])
    {
        LOG_DEBUG(logger, "HAS MAC: %s", mHasMacAddress ? "true" : "false");
        if (mHasMacAddress == false)
        {
            memset(mac, 0, sizeof(mCRB.MAC));
            return Status(Status::Code::BAD);
        }
        else
        {
            memcpy(mac, mCRB.MAC, sizeof(mCRB.MAC));
            return Status(Status::Code::GOOD);
        }
    }


    Status W5500::GetMacAddress(char mac[])
    {
        if (mHasMacAddress == false)
        {
            mac[0] = '\0';
            return Status(Status::Code::BAD);
        }
        else
        {
            snprintf(mac, 13, "%02X%02X%02X%02X%02X%02X",
                mCRB.MAC[0], mCRB.MAC[1], mCRB.MAC[2],
                mCRB.MAC[3], mCRB.MAC[4], mCRB.MAC[5]
            );
            return Status(Status::Code::GOOD);
        }
    }


    uint16_t W5500::GetEphemeralPort()
    {
        if (mEphemeralPort == 0xEE47)
        {
            mEphemeralPort = DEFAULT_EPHEMERAL_PORT;
        }
        
        return mEphemeralPort++;
    }


    Status W5500::read(const uint16_t address, const uint8_t control, uint8_t* output)
    {
        ASSERT((output != nullptr), "OUTPUT PARAMETER CANNOT BE NULL");

		if (xSemaphoreTake(xSemaphore, 1000) != pdTRUE)
		{
			return Status(Status::Code::BAD_RESOURCE_UNAVAILABLE);
		}

        SPI.beginTransaction(mSpiConfig);
        digitalWrite(mCS, LOW);

        uint8_t result = 0x00;
        result |= SPI.transfer((address >> 8) & 0xFF)    ? 0 << 0 : 1 << 0;
        result |= SPI.transfer(address & 0xFF)           ? 0 << 1 : 1 << 1;
        result |= SPI.transfer(control)                  ? 0 << 2 : 1 << 2;
        *output = SPI.transfer(0x00);
        
        digitalWrite(mCS, HIGH);
        SPI.endTransaction();

		xSemaphoreGive(xSemaphore);

        if (result != 0)
        {
            LOG_ERROR(logger, "FAILED TO READ SPI: UNABLE TO READ DATA FROM 0x%02X", mCS);
            return Status(Status::Code::BAD_COMMUNICATION_ERROR);
        }
        else
        {
            return Status(Status::Code::GOOD);
        }
    }


    Status W5500::read(const uint16_t address, const uint8_t control, const size_t length, uint8_t* output)
    {
        ASSERT((length != 0), "OUTPUT LENGTH MUST BE GREATER THAN 0");
        ASSERT((output != nullptr), "OUTPUT PARAMETER CANNOT BE NULL");

		if (xSemaphoreTake(xSemaphore, 1000) != pdTRUE)
		{
			return Status(Status::Code::BAD_RESOURCE_UNAVAILABLE);
		}

        SPI.beginTransaction(mSpiConfig);
        digitalWrite(mCS, LOW);
        
        uint8_t result = 0x00;
        result |= SPI.transfer((address >> 8) & 0xFF)    ? 0 << 0 : 1 << 0;
        result |= SPI.transfer(address & 0xFF)           ? 0 << 1 : 1 << 1;
        result |= SPI.transfer(control)                  ? 0 << 2 : 1 << 2;
        SPI.transferBytes(nullptr, output, length);
        
        digitalWrite(mCS, HIGH);
        SPI.endTransaction();

		xSemaphoreGive(xSemaphore);

        if (result != 0)
        {
            LOG_ERROR(logger, "FAILED TO READ SPI: UNABLE TO READ DATA FROM 0x%02X", mCS);
            return Status(Status::Code::BAD_COMMUNICATION_ERROR);
        }
        else
        {
            return Status(Status::Code::GOOD);
        }
    }


    Status W5500::retrieveCRB(w5500::crb_addr_e address, uint8_t* output)
    {
        using namespace w5500;
        const uint16_t numericAddress   = static_cast<uint16_t>(address);
        const uint8_t  controlPhase     = Converter::ControlPhase(bsb_e::COMMON_REGISTER, am_e::READ);
        return read(numericAddress, controlPhase, output);
    }


    Status W5500::retrieveCRB(w5500::crb_addr_e address, uint16_t* output)
    {
        using namespace w5500;
        const uint16_t numericAddress   = static_cast<uint16_t>(address);
        const uint8_t  controlPhase     = Converter::ControlPhase(bsb_e::COMMON_REGISTER, am_e::READ);

        uint8_t buffer[2] = { 0 };
        Status ret = read(numericAddress, controlPhase, sizeof(buffer), buffer);
        if (ret != Status::Code::GOOD)
        {
            *output = 0;
            return ret;
        }
        
        *output = static_cast<uint16_t>(buffer[0]) << 8 | buffer[1];
        return ret;
    }


    Status W5500::retrieveCRB(w5500::crb_addr_e address, uint32_t* output)
    {
        using namespace w5500;
        const uint16_t numericAddress   = static_cast<uint16_t>(address);
        const uint8_t  controlPhase     = Converter::ControlPhase(bsb_e::COMMON_REGISTER, am_e::READ);

        uint8_t buffer[4] = { 0 };
        Status ret = read(numericAddress, controlPhase, sizeof(buffer), buffer);
        if (ret != Status::Code::GOOD)
        {
            *output = 0;
            return ret;
        }
        
        *output = static_cast<uint32_t>(buffer[0]) << 24 | 
                  static_cast<uint32_t>(buffer[1]) << 16 |
                  static_cast<uint32_t>(buffer[2]) <<  8 |
                  static_cast<uint32_t>(buffer[3]) <<  0;
        return ret;
    }


    Status W5500::retrieveCRB(const w5500::crb_addr_e address, const size_t length, uint8_t* output)
    {
        using namespace w5500;
        const uint16_t numericAddress   = static_cast<uint16_t>(address);
        const uint8_t  controlPhase     = Converter::ControlPhase(bsb_e::COMMON_REGISTER, am_e::READ);
        return read(numericAddress, controlPhase, length, output);
    }


    Status W5500::retrieveSRB(const w5500::sock_id_e idx, w5500::srb_addr_e address, uint8_t* output)
    {
        using namespace w5500;
        const uint16_t numericAddress   = static_cast<uint16_t>(address);
        const uint8_t  controlPhase     = Converter::ControlPhase(idx, am_e::READ);
        return read(numericAddress, controlPhase, output);
    }


    Status W5500::retrieveSRB(const w5500::sock_id_e idx, w5500::srb_addr_e address, uint16_t* output)
    {
        using namespace w5500;
        const uint16_t numericAddress   = static_cast<uint16_t>(address);
        const uint8_t  controlPhase     = Converter::ControlPhase(idx, am_e::READ);

        uint8_t buffer[2] = { 0 };
        Status ret = read(numericAddress, controlPhase, sizeof(buffer), buffer);
        if (ret != Status::Code::GOOD)
        {
            *output = 0;
            return ret;
        }
        
        *output = static_cast<uint16_t>(buffer[0]) << 8 | buffer[1];
        return ret;
    }


    Status W5500::retrieveSRB(const w5500::sock_id_e idx, w5500::srb_addr_e address, uint32_t* output)
    {
        using namespace w5500;
        const uint16_t numericAddress   = static_cast<uint16_t>(address);
        const uint8_t  controlPhase     = Converter::ControlPhase(idx, am_e::READ);

        uint8_t buffer[4] = { 0 };
        Status ret = read(numericAddress, controlPhase, sizeof(buffer), buffer);
        if (ret != Status::Code::GOOD)
        {
            *output = 0;
            return ret;
        }
        
        *output = static_cast<uint32_t>(buffer[0]) << 24 | 
                  static_cast<uint32_t>(buffer[1]) << 16 |
                  static_cast<uint32_t>(buffer[2]) <<  8 |
                  static_cast<uint32_t>(buffer[3]) <<  0;
        return ret;
    }


    Status W5500::retrieveSRB(const w5500::sock_id_e idx, const w5500::srb_addr_e address, const size_t length, uint8_t* output)
    {
        using namespace w5500;
        const uint16_t numericAddress   = static_cast<uint16_t>(address);
        const uint8_t  controlPhase     = Converter::ControlPhase(idx, am_e::READ);
        return read(numericAddress, controlPhase, length, output);
    }


    Status W5500::write(const uint16_t address, const uint8_t control, const uint8_t input)
    {
		if (xSemaphoreTake(xSemaphore, 1000) != pdTRUE)
		{
			return Status(Status::Code::BAD_RESOURCE_UNAVAILABLE);
		}

        SPI.beginTransaction(mSpiConfig);
        digitalWrite(mCS, LOW);

        SPI.transfer((address >> 8) & 0xFF);
        SPI.transfer(address & 0xFF);
        SPI.transfer(control);
        SPI.transfer(input);
        
        digitalWrite(mCS, HIGH);
        SPI.endTransaction();

		xSemaphoreGive(xSemaphore);
        return Status(Status::Code::GOOD);
    }


    Status W5500::write(const uint16_t address, const uint8_t control, const size_t length, const uint8_t input[])
    {
		if (xSemaphoreTake(xSemaphore, 1000) != pdTRUE)
		{
			return Status(Status::Code::BAD_RESOURCE_UNAVAILABLE);
		}

        SPI.beginTransaction(mSpiConfig);
        digitalWrite(mCS, LOW);

        SPI.transfer((address >> 8)   & 0xFF);
        SPI.transfer((address >> 0)   & 0xFF);
        SPI.transfer(control);
        SPI.transferBytes(input, nullptr, length);
        
        digitalWrite(mCS, HIGH);
        SPI.endTransaction();

		xSemaphoreGive(xSemaphore);
        return Status(Status::Code::GOOD);
    }


    Status W5500::writeCRB(w5500::crb_addr_e address, uint8_t data)
    {
        using namespace w5500;
        const uint16_t numericAddress   = static_cast<uint16_t>(address);
        const uint8_t  controlPhase     = Converter::ControlPhase(bsb_e::COMMON_REGISTER, am_e::WRITE);
        return write(numericAddress, controlPhase, data);
    }


    Status W5500::writeCRB(w5500::crb_addr_e address, uint16_t data)
    {
        using namespace w5500;
        const uint16_t numericAddress   = static_cast<uint16_t>(address);
        const uint8_t  controlPhase     = Converter::ControlPhase(bsb_e::COMMON_REGISTER, am_e::WRITE);
        
        uint8_t buffer[2] = { 0 };
        buffer[0] = data >> 8;
        buffer[1] = data & 0xFF;

        return write(numericAddress, controlPhase, sizeof(buffer), buffer);
    }


    Status W5500::writeCRB(w5500::crb_addr_e address, uint32_t data)
    {
        using namespace w5500;
        const uint16_t numericAddress   = static_cast<uint16_t>(address);
        const uint8_t  controlPhase     = Converter::ControlPhase(bsb_e::COMMON_REGISTER, am_e::WRITE);
        
        uint8_t buffer[4] = { 0 };
        buffer[0] = data >> 24;
        buffer[1] = data >> 16;
        buffer[2] = data >>  8;
        buffer[3] = data & 0xFF;
        
        return write(numericAddress, controlPhase, sizeof(buffer), buffer);
    }


    Status W5500::writeCRB(const w5500::crb_addr_e address, const size_t length, const uint8_t data[])
    {
        using namespace w5500;
        const uint16_t numericAddress   = static_cast<uint16_t>(address);
        const uint8_t  controlPhase     = Converter::ControlPhase(bsb_e::COMMON_REGISTER, am_e::WRITE);
        return write(numericAddress, controlPhase, length, data);
    }


    Status W5500::writeSRB(const w5500::sock_id_e idx, w5500::srb_addr_e address, uint8_t data)
    {
        using namespace w5500;
        const uint16_t numericAddress   = static_cast<uint16_t>(address);
        const uint8_t  controlPhase     = Converter::ControlPhase(idx, am_e::WRITE);
        return write(numericAddress, controlPhase, data);
    }


    Status W5500::writeSRB(const w5500::sock_id_e idx, w5500::srb_addr_e address, uint16_t data)
    {
        using namespace w5500;
        const uint16_t numericAddress   = static_cast<uint16_t>(address);
        const uint8_t  controlPhase     = Converter::ControlPhase(idx, am_e::WRITE);
        
        uint8_t buffer[2] = { 0 };
        buffer[0] = data >> 8;
        buffer[1] = data & 0xFF;

        return write(numericAddress, controlPhase, sizeof(buffer), buffer);
    }


    Status W5500::writeSRB(const w5500::sock_id_e idx, w5500::srb_addr_e address, uint32_t data)
    {
        using namespace w5500;
        const uint16_t numericAddress   = static_cast<uint16_t>(address);
        const uint8_t  controlPhase     = Converter::ControlPhase(idx, am_e::WRITE);
        
        uint8_t buffer[4] = { 0 };
        buffer[0] = data >> 24;
        buffer[1] = data >> 16;
        buffer[2] = data >>  8;
        buffer[3] = data & 0xFF;
        
        return write(numericAddress, controlPhase, sizeof(buffer), buffer);
    }


    Status W5500::writeSRB(const w5500::sock_id_e idx, const w5500::srb_addr_e address, const size_t length, const uint8_t data[])
    {
        using namespace w5500;
        const uint16_t numericAddress   = static_cast<uint16_t>(address);
        const uint8_t  controlPhase     = Converter::ControlPhase(idx, am_e::WRITE);
        return write(numericAddress, controlPhase, length, data);
    }

    W5500* ethernet   = nullptr;
    W5500* link1W5500 = nullptr;
    W5500* link2W5500 = nullptr;
}


#endif