/**
 * @file W5500.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#if defined(MT11)




#include <esp32-hal-gpio.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Include/Converter.h"
#include "Include/Microchip24AA02E.h"
#include "W5500.h"

SPISettings muffin::W5500::mSpiConfig;



namespace muffin {

    
    W5500::W5500(const w5500::if_e idx)
        : mCS(static_cast<uint8_t>(idx))
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
    Status W5500::Init(const uint8_t mhz)
    {
        ASSERT((mhz < 71), "CLOCK MUST BE LESS THAN OR EQUAL TO 70MHz");

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
        initSPI(mhz);

        Status ret = setMacAddress();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET MAC ADDRESS: %s", ret.c_str());
            return ret;
        }
        
        return ret;
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
        if (microchip.Read(false, mCRB.MAC) == false)
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


    Status W5500::SetIPv4(const IPAddress ipv4)
    {
        return setIPv4(w5500::ipv4_type_e::SOURCE_IPv4, ipv4);
    }


    Status W5500::SetGateway(const IPAddress ipv4)
    {
        return setIPv4(w5500::ipv4_type_e::GATEWAY, ipv4);
    }


    Status W5500::SetSubnetmask(const IPAddress ipv4)
    {
        return setIPv4(w5500::ipv4_type_e::SUBNETMASK, ipv4);
    }


    Status W5500::SetDNS1(const IPAddress ipv4)
    {
        mDNS1 = ipv4;
        return Status(Status::Code::BAD_NOT_IMPLEMENTED);
    }


    Status W5500::SetDNS2(const IPAddress ipv4)
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
}

#endif