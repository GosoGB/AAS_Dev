/**
 * @file Microchip24AA02E.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Microchip24AA02E 칩에 저장된 MAC 주소를 읽는 클래스를 정의합니다.
 * 
 * @date 2025-04-28
 * @version 1.0.0
 * 
 * @copyright Copyright (c) 2025
 */




#include <Wire.h>

#include "Common/Assert.h"
#include "Microchip24AA02E.h"



namespace muffin {


    bool Microchip24AA02E::Read(const bool isLINK, uint8_t mac[])
    {
        ASSERT((mac != nullptr), "OUTPUT PARAMETER CANNOT BE A NULL POINTER");

        if (Wire.setPins(mSDA, mSCL) == false)
        {
            log_e("FAILED TO SET PINS FOR I2C");
            return false;
        }

        if (Wire.begin() == false)
        {
            log_e("FAILED TO BEGIN I2C BUS");
            return false;
        }
        
        uint8_t idx = 0;
        bool hasRead = true;
        for (uint16_t offset = 0xFA; offset <= 0xFF; ++offset)
        {
            mac[idx] = readRegister(offset, isLINK);
            if (unlikely(mac[idx] == -1))
            {
                hasRead = false;
                break;
            }
            else
            {
                ++idx;
            }
        }

        if (unlikely(Wire.end() == false))
        {
            log_e("FAILED TO END I2C");
            return false;
        }

        if (unlikely(hasRead == false))
        {
            log_e("FAILED TO READ MAC ADDRESS");
            return hasRead;
        }

        if (verifyMacAddress(mac) == false)
        {
            log_e("MAC ADDRESS IS INVALID");
            return false;
        }
        
        return hasRead;
    }


    int8_t Microchip24AA02E::readRegister(const uint8_t offset, const uint8_t slaveID)
    {
        const uint8_t bytesToRead = 1;

        Wire.beginTransmission(slaveID);
        Wire.write(offset);
        Wire.endTransmission();
        Wire.requestFrom(slaveID, bytesToRead);
      
        const uint16_t timeout = 1000;
        const uint32_t startMillis = millis();
        while (millis() - startMillis < timeout)
        {
            if (Wire.available() > 0)
            {
                return static_cast<int8_t>(Wire.read());
            }
            vTaskDelay(50 / portTICK_PERIOD_MS);
        };

        log_e("FAILED TO READ DUE TO TIMEOUT ERROR");
        return -1;
    }


    int8_t Microchip24AA02E::readRegister(const uint8_t offset, const bool isLINK)
    {
        const uint8_t deviceAddress = isLINK ? 
            mDeviceAddressLINK :
            mDeviceAddressBOARD;

        const uint8_t bytesToRead = 1;

        Wire.beginTransmission(deviceAddress);
        Wire.write(offset);
        Wire.endTransmission();
        Wire.requestFrom(deviceAddress, bytesToRead);
      
        const uint16_t timeout = 1000;
        const uint32_t startMillis = millis();
        while (millis() - startMillis < timeout)
        {
            if (Wire.available() > 0)
            {
                return static_cast<int8_t>(Wire.read());
            }
            vTaskDelay(50 / portTICK_PERIOD_MS);
        };

        log_e("FAILED TO READ DUE TO TIMEOUT ERROR");
        return -1;
    }


    bool Microchip24AA02E::verifyMacAddress(const uint8_t baseMAC[])
    {
        const uint8_t MAX_ARRAY_SIZE = 6;
        bool isValid = false;

        for (uint8_t idx = 0; idx < MAX_ARRAY_SIZE; ++idx)
        {
            if (baseMAC[idx] != UINT8_MAX)
            {
                isValid = true;
                break;
            }
        }

        return isValid;
    }
}