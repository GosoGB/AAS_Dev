/**
 * @file MelsecBuilder.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-04-11
 * 
 * @copyright Copyright (c) 2025
 * 
 */



#include <string.h>
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"

#include "MelsecBuilder.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin
{
    MelsecBuilder::MelsecBuilder()
    {

    }
    
    MelsecBuilder::~MelsecBuilder()
    {

    }

    size_t MelsecBuilder::BuildReadRequestDataBinary(MelsecCommonHeader commonHeader, jvs::ps_e plcSeries, const bool isBit, const jvs::node_area_e area, const uint32_t address, const int count, uint8_t* frame)
    {
        size_t index = 0;
        index = buildBinaryCommonHeader(commonHeader, frame);

        size_t tempPos = index;
        frame[index++] = 0x00;
        frame[index++] = 0x00;

        // 3. 모니터링 타이머 (1000ms)
        frame[index++] = static_cast<uint8_t>(commonHeader.MonitoringTimer & 0xFF);
        frame[index++] = static_cast<uint8_t>((commonHeader.MonitoringTimer >> 8) & 0xFF);

        index += buildBinaryRequestData(plcSeries,isBit,area,address,count,melsec_command_e::BATCH_READ, &frame[index]);

        uint16_t reqLen = static_cast<uint16_t>(index - 9);
        frame[tempPos] = static_cast<uint8_t>(reqLen & 0xFF);
        frame[tempPos + 1] = static_cast<uint8_t>((reqLen >> 8) & 0xFF);

        return index;
    }

    size_t MelsecBuilder::BuildReadRequestDataASCII(MelsecCommonHeader commonHeader, jvs::ps_e plcSeries, const bool isBit, const jvs::node_area_e area, const uint32_t address, const int count, uint8_t* frame)
    {
        size_t index = 0;
        index = buildAsciiCommonHeader(commonHeader, frame);
        size_t tempPos = index;
        frame[index++] = 0x00;
        frame[index++] = 0x00;
        frame[index++] = 0x00;
        frame[index++] = 0x00;
        char buf[5];
        sprintf(buf, "%04X", commonHeader.MonitoringTimer); 
        for (char c : buf)
        {
            frame[index++] = static_cast<uint8_t>(c);
        }
        // NULL 삭제
        index--;
        index += buildAsciiRequestData(plcSeries,isBit,area,address,count,melsec_command_e::BATCH_READ, &frame[index]);
        
        size_t startDataPos = tempPos + 4;
        size_t actualLength = (index ) - startDataPos;
        char lenBuf[5];
        snprintf(lenBuf, sizeof(lenBuf), "%04X", static_cast<unsigned int>(actualLength));
        for (int i = 0; i < 4; ++i)
        {
            frame[tempPos + i] = static_cast<uint8_t>(lenBuf[i]);
        }

        return index;
    }

    size_t MelsecBuilder::BuildWriteRequestDataASCII(MelsecCommonHeader commonHeader, jvs::ps_e plcSeries, const bool isBit, const jvs::node_area_e area, const uint32_t address, const int count, const uint16_t data[], uint8_t* frame)
    {
        size_t index = 0;
        index = buildAsciiCommonHeader(commonHeader, frame);
        size_t tempPos = index;
        frame[index++] = 0x00;
        frame[index++] = 0x00;
        frame[index++] = 0x00;
        frame[index++] = 0x00;
        char buf[5];
        sprintf(buf, "%04X", commonHeader.MonitoringTimer); 
        for (char c : buf)
        {
            frame[index++] = static_cast<uint8_t>(c);
        }
        // NULL 삭제
        index--;
        index += buildAsciiRequestData(plcSeries, isBit, area, address, count, melsec_command_e::BATCH_WRITE, &frame[index]);
        for (int i = 0; i < count; ++i)
        {
            char dataBuf[5];
            sprintf(dataBuf, "%04X", data[i]);  // 16bit -> 4자리 HEX
            for (char c : dataBuf)
            {
                frame[index++] = static_cast<uint8_t>(c);
            }
            // NULL 삭제
            index--;
        }

        // Length 계산 및 삽입
        size_t startDataPos = tempPos + 4;
        size_t actualLength = index - startDataPos;
        char lenBuf[5];
        snprintf(lenBuf, sizeof(lenBuf), "%04X", static_cast<unsigned int>(actualLength));
        for (int i = 0; i < 4; ++i)
        {
            frame[tempPos + i] = static_cast<uint8_t>(lenBuf[i]);
        }

        return index;
    }

    size_t MelsecBuilder::BuildWriteRequestDataBinary(MelsecCommonHeader commonHeader, jvs::ps_e plcSeries, const bool isBit, const jvs::node_area_e area, const uint32_t address, const int count, const uint16_t data[], uint8_t* frame)
    {
        size_t index = 0;
        index = buildBinaryCommonHeader(commonHeader, frame);

        size_t tempPos = index;
        frame[index++] = 0x00;
        frame[index++] = 0x00;

        // 3. 모니터링 타이머 (1000ms)
        frame[index++] = static_cast<uint8_t>(commonHeader.MonitoringTimer & 0xFF);
        frame[index++] = static_cast<uint8_t>((commonHeader.MonitoringTimer >> 8) & 0xFF);

        index += buildBinaryRequestData(plcSeries,isBit,area,address,count,melsec_command_e::BATCH_WRITE, &frame[index]);

        for (int i = 0; i < count; ++i)
        {
            frame[index++] = static_cast<uint8_t>(data[i] & 0xFF);         // LSB
            frame[index++] = static_cast<uint8_t>((data[i] >> 8) & 0xFF);  // MSB
        }

        uint16_t reqLen = static_cast<uint16_t>(index - 9);
        frame[tempPos] = static_cast<uint8_t>(reqLen & 0xFF);
        frame[tempPos + 1] = static_cast<uint8_t>((reqLen >> 8) & 0xFF);
        
        return index;
    }

    size_t MelsecBuilder::BuildNopCommand(jvs::df_e dataFormat, MelsecCommonHeader commonHeader, uint8_t* frame)
    {
        size_t index = BuildCommonHeader(dataFormat, commonHeader, frame);

        if (dataFormat == jvs::df_e::ASCII)
        {
            char buf[5];
            sprintf(buf, "%04X", commonHeader.MonitoringTimer); 
            std::string cmd = "0006" + std::string(buf) + "0C000000";
            for (char c : cmd)
            {
                frame[index++] = static_cast<uint8_t>(c);
            }

            return index;
        }
        else
        {
            // 2. 데이터 길이 (Request Data Length)
            frame[index++] = 0x06;  // Length LSB
            frame[index++] = 0x00;  // Length MSB

            // 3. 모니터링 타이머 (1000ms)
            frame[index++] = static_cast<uint8_t>(commonHeader.MonitoringTimer & 0xFF);
            frame[index++] = static_cast<uint8_t>((commonHeader.MonitoringTimer >> 8) & 0xFF);

            // 4. 커맨드 (NOP = 0x000C)
            frame[index++] = 0x0C;  // Command LSB
            frame[index++] = 0x00;  // Command MSB

            // 5. 서브커맨드
            frame[index++] = 0x00;  // Subcommand LSB
            frame[index++] = 0x00;  // Subcommand MSB

            return index;
        }
        

    }

    size_t MelsecBuilder::BuildCommonHeader(jvs::df_e dataFormat, MelsecCommonHeader commonHeader, uint8_t* frame)
    {
        switch (dataFormat)
        {
        case jvs::df_e::BINARY:
            return (buildBinaryCommonHeader(commonHeader, frame));
        case jvs::df_e::ASCII:
            return (buildAsciiCommonHeader(commonHeader, frame)); 
        default:
            ASSERT(false, "UNSUPPORTED DATA FRAME");
            return 0;
        }
    }

    size_t MelsecBuilder::buildAsciiCommonHeader(MelsecCommonHeader commonHeader, uint8_t* frame)
    {
        size_t index = 0;

        char asciiHeader[15];
        sprintf(
            asciiHeader, "%.4X%02X%02X%.4X%02X", 
            commonHeader.SubHeader, 
            commonHeader.NetworkNumber, 
            commonHeader.PcNumber, 
            commonHeader.IoNumber, 
            commonHeader.StationNumber
        );
        
        for (size_t i = 0; i < 14; ++i) 
        {
            frame[index++] = static_cast<uint8_t>(asciiHeader[i]);
        }

        return index;
    }

    size_t MelsecBuilder::buildBinaryCommonHeader(MelsecCommonHeader commonHeader, uint8_t* frame)
    {
        size_t index = 0;

        // 1. Subheader
        frame[index++] = static_cast<uint8_t>((commonHeader.SubHeader >> 8) & 0xFF);
        frame[index++] = static_cast<uint8_t>(commonHeader.SubHeader & 0xFF);

        // 2. Network No
        frame[index++] = commonHeader.NetworkNumber;

        // 3. PC No
        frame[index++] = commonHeader.PcNumber;

        // 4. IO No (Little endian)
        frame[index++] = static_cast<uint8_t>(commonHeader.IoNumber & 0xFF);
        frame[index++] = static_cast<uint8_t>((commonHeader.IoNumber >> 8) & 0xFF);

        // 5. Station No
        frame[index++] = commonHeader.StationNumber;

        return index;
    }

    size_t MelsecBuilder::buildAsciiRequestCommand(melsec_command_e command, uint8_t* frame)
    {
        size_t index = 0;

        switch (command)
        {
        case melsec_command_e::BATCH_READ:
            frame[index++] = '0'; 
            frame[index++] = '4';
            frame[index++] = '0'; 
            frame[index++] = '1';
            return index;

        case melsec_command_e::BATCH_WRITE:
            frame[index++] = '1'; 
            frame[index++] = '4';
            frame[index++] = '0'; 
            frame[index++] = '1';
            return index;

        default:
            frame[index++] = '0'; 
            frame[index++] = '0';
            frame[index++] = '0'; 
            frame[index++] = '0';
            return index;
        }
    }

    size_t MelsecBuilder::buildAsciiRequestSubCommand(melsec_command_e command, jvs::ps_e plcSeries, const bool isBit, uint8_t* frame)
    {
        size_t index = 0;
        
        if (plcSeries == jvs::ps_e::IQR_SERIES)
        {
            if (isBit) 
            {
                frame[index++] = '0'; 
                frame[index++] = '0';
                frame[index++] = '0'; 
                frame[index++] = '3';
            } 
            else 
            {
                frame[index++] = '0';
                frame[index++] = '0';
                frame[index++] = '0'; 
                frame[index++] = '2';
            }
        }
        else
        {
            if (isBit) 
            {
                frame[index++] = '0'; 
                frame[index++] = '0';
                frame[index++] = '0'; 
                frame[index++] = '1';
            } 
            else 
            {
                frame[index++] = '0'; 
                frame[index++] = '0';
                frame[index++] = '0'; 
                frame[index++] = '0';
            }
        }
        
        return index;
    }

    size_t MelsecBuilder::buildAsciiRequestData(jvs::ps_e plcSeries, const bool isBit, const jvs::node_area_e area, const uint32_t address, const int count, const melsec_command_e command, uint8_t* frame)
    {
        size_t index = 0;
        index = buildAsciiRequestCommand(command, frame);
        index += buildAsciiRequestSubCommand(command, plcSeries, isBit, &frame[index]);

        if (plcSeries == jvs::ps_e::IQR_SERIES)
        {
            std::string deviceCode = getDeviceCodeASCII(area);
            while (deviceCode.length() < 4)
            {
                deviceCode += '*';
            }

            for (char c : deviceCode)
            {
                frame[index++] = static_cast<uint8_t>(c);
            }
        }
        else
        {
            std::string deviceCode = getDeviceCodeASCII(area);
            for (char c : deviceCode)
            {
                frame[index++] = static_cast<uint8_t>(c);
            }
        }

        if (isHexMemory(area))
        {
            char hexStr[7];  // 6자리 + NULL
            snprintf(hexStr, sizeof(hexStr), "%06X", address);

            for (int i = 0; i < 6; ++i) 
            {
                frame[index++] = static_cast<uint8_t>(hexStr[i]);
            }
        }
        else
        {
            // 10진수 주소: 6자리, 예: "000123"
            std::string addrStr = std::to_string(address);
            while (addrStr.length() < 6)
            {
                addrStr = "0" + addrStr;
            }

            for (int i = 0; i < 6; ++i) 
            {
                frame[index++] = static_cast<uint8_t>(addrStr[i]);
            }
        }

        char buf[5];
        snprintf(buf, sizeof(buf), "%04X", count);
        for (int i = 0; i < 4; ++i) 
        {
            frame[index++] = static_cast<uint8_t>(buf[i]);  // ASCII 문자 복사
        }

        return index;
    }

    size_t MelsecBuilder::buildBinaryRequestData(jvs::ps_e plcSeries, const bool isBit, const jvs::node_area_e area, const uint32_t address, const int count, const melsec_command_e command, uint8_t* frame)
    {

        size_t index = 0;
        index = buildBinaryRequestCommand(command, frame);
        index += buildBinaryRequestSubCommand(command, plcSeries, isBit, &frame[index]);

        if (isHexMemory(area))
        {
            char buf[16];
            snprintf(buf, sizeof(buf), "%X", address);

            uint32_t convAddr = strtoul(buf, nullptr, 16);

            frame[index++] = static_cast<uint8_t>(convAddr & 0xFF);
            frame[index++] = static_cast<uint8_t>((convAddr >> 8) & 0xFF);
            frame[index++] = static_cast<uint8_t>((convAddr >> 16) & 0xFF);
        }
        else
        {
            frame[index++] = static_cast<uint8_t>(address & 0xFF);
            frame[index++] = static_cast<uint8_t>((address >> 8) & 0xFF);
            frame[index++] = static_cast<uint8_t>((address >> 16) & 0xFF);
        }


        // 4. 디바이스 코드
        if (plcSeries == jvs::ps_e::IQR_SERIES) 
        {
            frame[index++] = getDeviceCodeBinary(area);
            frame[index++] = 0x00;
        } 
        else 
        {
            frame[index++] = getDeviceCodeBinary(area);
        }
        

        // 5. 읽기/쓰기 개수
        frame[index++] = static_cast<uint8_t>(count & 0xFF);
        frame[index++] = static_cast<uint8_t>((count >> 8) & 0xFF);

        return index;
    }

    size_t MelsecBuilder::buildBinaryRequestCommand(melsec_command_e command, uint8_t* frame)
    {
        size_t index = 0;

        switch (command)
        {
        case melsec_command_e::BATCH_READ:
            frame[index++] = static_cast<uint8_t>(0x01);
            frame[index++] = static_cast<uint8_t>(0x04);
            return index;
        case melsec_command_e::BATCH_WRITE:
            frame[index++] = static_cast<uint8_t>(0x01);
            frame[index++] = static_cast<uint8_t>(0x14);
            return index;
        default:
            frame[index++] = static_cast<uint8_t>(0x00);
            frame[index++] = static_cast<uint8_t>(0x00);
            return index;
        }
    }

    size_t MelsecBuilder::buildBinaryRequestSubCommand(melsec_command_e command, jvs::ps_e plcSeries, const bool isBit, uint8_t* frame)
    {
        size_t index = 0;

        if (plcSeries == jvs::ps_e::IQR_SERIES) 
        {
            frame[index++] = static_cast<uint8_t>(isBit ? 0x03 : 0x02);
            frame[index++] = 0x00;
        } 
        else 
        {
            frame[index++] = static_cast<uint8_t>(isBit ? 0x01 : 0x00);
            frame[index++] = 0x00;
        }

        return index;
    }

    std::string MelsecBuilder::getDeviceCodeASCII(jvs::node_area_e area)
    {
        switch (area)
        {
            case jvs::node_area_e::SM: return "SM";
            case jvs::node_area_e::SD: return "SD";
            case jvs::node_area_e::X:  return "X*";
            case jvs::node_area_e::Y:  return "Y*";
            case jvs::node_area_e::M:  return "M*";
            case jvs::node_area_e::L:  return "L*";
            case jvs::node_area_e::F:  return "F*";
            case jvs::node_area_e::V:  return "V*";
            case jvs::node_area_e::B:  return "B*";
            case jvs::node_area_e::D:  return "D*";
            case jvs::node_area_e::W:  return "W*";
            case jvs::node_area_e::TS: return "TS";
            case jvs::node_area_e::TC: return "TC";
            case jvs::node_area_e::TN: return "TN";
            case jvs::node_area_e::LTS: return "LTS";
            case jvs::node_area_e::LTC: return "LTC";
            case jvs::node_area_e::LTN: return "LTN";
            case jvs::node_area_e::STS: return "STS";
            case jvs::node_area_e::STC: return "STC";
            case jvs::node_area_e::STN: return "STN";
            case jvs::node_area_e::LSTS: return "LSTS";
            case jvs::node_area_e::LSTC: return "LSTC";
            case jvs::node_area_e::LSTN: return "LSTN";
            case jvs::node_area_e::CS: return "CS";
            case jvs::node_area_e::CC: return "CC";
            case jvs::node_area_e::CN: return "CN";
            case jvs::node_area_e::LCS: return "LCS";
            case jvs::node_area_e::LCC: return "LCC";
            case jvs::node_area_e::LCN: return "LCN";
            case jvs::node_area_e::SB: return "SB";
            case jvs::node_area_e::SW: return "SW";
            case jvs::node_area_e::S:  return "S*";
            case jvs::node_area_e::DX: return "DX";
            case jvs::node_area_e::DY: return "DY";
            case jvs::node_area_e::Z:  return "Z*";
            case jvs::node_area_e::LZ: return "LZ";
            default: return ""; // 알 수 없는 디바이스
        }
    }

    uint8_t MelsecBuilder::getDeviceCodeBinary(jvs::node_area_e area)
    {
        switch (area)
        {
            case jvs::node_area_e::SM: return 0x91;  // Special relay
            case jvs::node_area_e::SD: return 0xA9;  // Special register
            case jvs::node_area_e::X:  return 0x9C;  // Input
            case jvs::node_area_e::Y:  return 0x9D;  // Output
            case jvs::node_area_e::M:  return 0x90;  // Internal relay
            case jvs::node_area_e::L:  return 0x92;  // Latch relay
            case jvs::node_area_e::F:  return 0x93;  // Annunciator
            case jvs::node_area_e::V:  return 0x94;  // Edge relay
            case jvs::node_area_e::B:  return 0xA0;  // Link relay
            case jvs::node_area_e::D:  return 0xA8;  // Data register
            case jvs::node_area_e::W:  return 0xB4;  // Link register
            case jvs::node_area_e::TS: return 0xC1;  // Timer (contact)
            case jvs::node_area_e::TC: return 0xC0;  // Timer (coil)
            case jvs::node_area_e::TN: return 0xC2;  // Timer (current value)
            case jvs::node_area_e::LTS: return 0x51;
            case jvs::node_area_e::LTC: return 0x50;
            case jvs::node_area_e::LTN: return 0x52;
            case jvs::node_area_e::STS: return 0xC7; // Retentive timer (contact)
            case jvs::node_area_e::STC: return 0xC6; // Retentive timer (coil)
            case jvs::node_area_e::STN: return 0xC8; // Retentive timer (current value)
            case jvs::node_area_e::LSTS: return 0x59;
            case jvs::node_area_e::LSTC: return 0x58;
            case jvs::node_area_e::LSTN: return 0x5A;
            case jvs::node_area_e::CS: return 0xC4;  // Counter (contact)
            case jvs::node_area_e::CC: return 0xC3;  // Counter (coil)
            case jvs::node_area_e::CN: return 0xC5;  // Counter (current value)
            case jvs::node_area_e::LCS: return 0x55;
            case jvs::node_area_e::LCC: return 0x54;
            case jvs::node_area_e::LCN: return 0x56;
            case jvs::node_area_e::SB: return 0xA1;  // Link special relay
            case jvs::node_area_e::SW: return 0xB5;  // Link special register
            case jvs::node_area_e::S:  return 0x98;  // Step relay
            case jvs::node_area_e::DX: return 0xA2;  // Direct access input
            case jvs::node_area_e::DY: return 0xA3;  // Direct access output
            case jvs::node_area_e::Z:  return 0xCC;  // Index register
            case jvs::node_area_e::LZ: return 0x62;
            default: return 0x00;  // Unknown or unsupported
        }
    }

    bool MelsecBuilder::isHexMemory(const jvs::node_area_e area)
    {
        switch (area)
        {
        case jvs::node_area_e::X:
        case jvs::node_area_e::Y:
        case jvs::node_area_e::B:
        case jvs::node_area_e::W:
        case jvs::node_area_e::SB:
        case jvs::node_area_e::SW:
        case jvs::node_area_e::DX:
        case jvs::node_area_e::DY:
        return true;
        case jvs::node_area_e::SM:
        case jvs::node_area_e::M:
        case jvs::node_area_e::L:
        case jvs::node_area_e::F:
        case jvs::node_area_e::V:
        case jvs::node_area_e::TS:
        case jvs::node_area_e::TC:
        case jvs::node_area_e::LTS:
        case jvs::node_area_e::LTC:
        case jvs::node_area_e::STS:
        case jvs::node_area_e::STC:
        case jvs::node_area_e::LSTS:
        case jvs::node_area_e::LSTC:
        case jvs::node_area_e::CS:
        case jvs::node_area_e::CC:
        case jvs::node_area_e::LCS:
        case jvs::node_area_e::LCC:
        case jvs::node_area_e::SD:
        case jvs::node_area_e::D:
        case jvs::node_area_e::TN:
        case jvs::node_area_e::CN:
        case jvs::node_area_e::Z:
        case jvs::node_area_e::LTN:
        case jvs::node_area_e::STN:
        case jvs::node_area_e::LSTN:
        case jvs::node_area_e::LCN:
        case jvs::node_area_e::LZ:
        return false;
        default:
            return false;
        }
    }
}