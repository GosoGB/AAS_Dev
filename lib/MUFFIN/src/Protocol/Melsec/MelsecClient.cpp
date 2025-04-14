/**
 * @file MelsecClient.cpp
 * @author Kim, Joo-Sung (Joosung5732@edgecross.ai)
 * 
 * @brief Melsec Client 클래스를 정의합니다.
 * 
 * @date 2025-04-07
 * @version 1.4.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2025
 */



#include <string.h>
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"

#include "MelsecClient.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin
{
    MelsecClient::MelsecClient() 
    :   mPort(0), 
        mIP(nullptr), 
        mPlcSeries(jvs::ps_e::QL_SERIES), 
        mDataFormat(jvs::df_e::BINARY)
    {
        
    }


    MelsecClient::~MelsecClient()
    {   

    }

    void MelsecClient::SetHeader(uint8_t networkNo, uint8_t pcNo, uint16_t ioNo, uint8_t stationNo) 
    {
        mCommonHeader.NetworkNumber = networkNo;
        mCommonHeader.PcNumber = pcNo;
        mCommonHeader.IoNumber = ioNo;
        mCommonHeader.StationNumber = stationNo;
    }

    int MelsecClient::buildBinaryCommonHeader(uint8_t* frame)
    {
        int index = 0;

        // 1. Subheader
        frame[index++] = static_cast<uint8_t>((mCommonHeader.SubHeader >> 8) & 0xFF);
        frame[index++] = static_cast<uint8_t>(mCommonHeader.SubHeader & 0xFF);

        // 2. Network No
        frame[index++] = mCommonHeader.NetworkNumber;

        // 3. PC No
        frame[index++] = mCommonHeader.PcNumber;

        // 4. IO No (Little endian)
        frame[index++] = static_cast<uint8_t>(mCommonHeader.IoNumber & 0xFF);
        frame[index++] = static_cast<uint8_t>((mCommonHeader.IoNumber >> 8) & 0xFF);

        // 5. Station No
        frame[index++] = mCommonHeader.StationNumber;

        return index;
    }

    std::string MelsecClient::buildAsciiCommonHeader() 
    {
        char buf[15];
        sprintf(buf, "%.4X%02X%02X%.4X%02X", 
        mCommonHeader.SubHeader,
        mCommonHeader.NetworkNumber,
        mCommonHeader.PcNumber,
        mCommonHeader.IoNumber,
        mCommonHeader.StationNumber
        );
        return std::string(buf);
    }

    bool MelsecClient::Begin(const char *ip, uint16_t port, jvs::ps_e series) 
    {
        mIP = ip;
        mPort = port;
        mPlcSeries = series;
        mMelsecTCP.setIP(ip);
        mMelsecTCP.setPort(port);
        return mMelsecTCP.connectTCP();
    }

    bool MelsecClient::Connected() 
    {
        if (mMelsecTCP.connected == false)
        {
            return false;
        }

        // uint8_t test[1024] = {'\0'};
        // size_t index = 0;

        // index = mMelsecBuilder.BuildNopCommand(mDataFormat, mCommonHeader, test);

        // Serial.print("SEND : ");
        // for (size_t i = 0; i < index; i++)
        // {
        //     Serial.print((char)test[i]);
        // }
        // Serial.println();

        if (mDataFormat == jvs::df_e::ASCII)
        {
            std::string cmd = buildAsciiCommonHeader();

            // NOP 명령: 모니터링 타이머 + 명령(0C00) + 서브커맨드(0000)
            // 데이터 길이 = 2(타이머) + 2(커맨드) + 2(서브커맨드) = 6 = 0x0006
            cmd += "0006";              // Request data length (6 bytes = 12 ASCII chars)
            char buf[5];
            sprintf(buf, "%04X", mCommonHeader.MonitoringTimer);

            cmd += std::string(buf);    // Monitoring timer (0010 = 1000ms)
            cmd += "0C00";              // Command = 0x000C (NOP)
            cmd += "0000";              // Subcommand = 0x0000

            // LOG_INFO(logger,"req : %s",cmd.c_str());
            std::string response = sendAndReceive(cmd);
            
            // LOG_INFO(logger,"resp : %s",response.c_str());
            // 최소 응답 길이 확인 (헤더 + 응답코드까지 22)
            if (response == "ERROR" || response.length() < 22)
            {
                mMelsecTCP.closeConnection();
                return false;
            }

            std::string endCode = response.substr(response.length() - 4);
            
            if (strcasecmp(endCode.c_str(), "0000") == 0)
            {
                return true;
            }
            else
            {
                mMelsecTCP.closeConnection();
                return false;
            }
        }
        else
        {
            uint8_t frame[1024];

            // 1. 서두부 (Subheader ~ Request data length)
            int index = buildBinaryCommonHeader(frame);

            // 2. 데이터 길이 (Request Data Length)
            frame[index++] = 0x06;  // Length LSB
            frame[index++] = 0x00;  // Length MSB

            // 3. 모니터링 타이머 (1000ms)
            frame[index++] = static_cast<uint8_t>(mCommonHeader.MonitoringTimer & 0xFF);
            frame[index++] = static_cast<uint8_t>((mCommonHeader.MonitoringTimer >> 8) & 0xFF);

            // 4. 커맨드 (NOP = 0x000C)
            frame[index++] = 0x0C;  // Command LSB
            frame[index++] = 0x00;  // Command MSB

            // 5. 서브커맨드
            frame[index++] = 0x00;  // Subcommand LSB
            frame[index++] = 0x00;  // Subcommand MSB

            std::string request((char*)frame, index);
            std::string response = sendAndReceive(request);

            // 최소 응답 길이 확인 (헤더 + 응답코드까지 22)
            if (response == "ERROR" || response.length() < 22)
            {
                mMelsecTCP.closeConnection();
                return false;
            }

            std::string endCode = response.substr(response.length() - 4);
            if (strcasecmp(endCode.c_str(), "0000") == 0)
            {
                return true;
            }
            else
            {
                mMelsecTCP.closeConnection();
                return false;
            }
        }
    }

    std::string MelsecClient::sendAndReceive(const std::string &command) 
    {
        if (mDataFormat == jvs::df_e::ASCII)
        {
            return mMelsecTCP.sendAndReceive(command);
        }

        // BINARY 처리
        const uint8_t *data = (const uint8_t *)command.c_str();
        int length = command.length();
        uint8_t response[1024];
        int respLen = mMelsecTCP.sendAndReceiveBinary(data, length, response);

        if (respLen <= 0) return "ERROR";

        // 응답 바이트를 HEX 문자열로 변환
        std::string result = "";
        char buf[3];
        for (int i = 0; i < respLen; i++) 
        {
            sprintf(buf, "%02X", response[i]);
            result += std::string(buf);
        }
        log_d("result : %s",result.c_str());
        return result;
    }


    int MelsecClient::sendAndReceive(const std::string &command, uint16_t buffer[]) 
    {
        std::string resp = sendAndReceive(command);
        if (resp == "ERROR") return 0;
        return hexStringToWords(resp, buffer);
    }


    std::string MelsecClient::extractBinaryData(const std::string& response)
    {
        const int HEADER_SIZE = 11;  // Binary 응답 헤더 길이 (바이트 기준)
        size_t hexHeaderLen = HEADER_SIZE * 2;  // HEX 문자열이므로 * 2

        if (response.length() <= hexHeaderLen)
            return "";

        return response.substr(hexHeaderLen);
    }

    std::string MelsecClient::extractAsciiData(const std::string& response)
    {
        if (response.length() <= 22)
            return "";

        return response.substr(22);  // 22자리 이후가 실제 데이터
    }

    int MelsecClient::hexStringToWords(const std::string &hexStr, uint16_t buffer[]) 
    {
        int wordCount = hexStr.length() / 4;
        std::string fitted = fitStringToWords(hexStr, wordCount);
        for (int i = 0; i < wordCount; i++) 
        {
            uint16_t word = 0;
            for (int j = 0; j < 4; j++) 
            {
                char c = fitted[i * 4 + j];
                word <<= 4;
                if (c >= '0' && c <= '9')
                {
                    word |= (c - '0');
                }
                else if (c >= 'A' && c <= 'F')
                {
                    word |= (c - 'A' + 10);
                }
                else if (c >= 'a' && c <= 'f')
                {
                    word |= (c - 'a' + 10);
                }
            }
            buffer[i] = word;
        }
        return wordCount;
    }

    int MelsecClient::wordsToHexString(const uint16_t data[], int wordCount, std::string &hexStr) 
    {
        hexStr = "";
        char buf[5];
        for (int i = 0; i < wordCount; i++) 
        {
            sprintf(buf, "%.4X", data[i]);
            hexStr += std::string(buf);
        }
        return wordCount;
    }

    int MelsecClient::wordArrayToByteArray(const uint16_t words[], uint8_t bytes[], int wordCount) 
    {
        for (int i = 0; i < wordCount; i++) 
        {
        bytes[i * 2]     = (uint8_t)(words[i] >> 8);
        bytes[i * 2 + 1] = (uint8_t)(words[i] & 0xFF);
        }
        return wordCount * 2;
    }

    std::string MelsecClient::fitStringToWords(const std::string &input, int wordCount) 
    {
        int requiredLength = wordCount * 4;
        std::string output = input;

        int diff = requiredLength - static_cast<int>(input.length());

        if (diff > 0) 
        {
            output = std::string(diff, '0') + output;
        } 
        else if (diff < 0) 
        {
            output = output.substr(0, requiredLength);
        }
        return output;
    }

    std::string MelsecClient::stringToHexASCII(const std::string& input)
    {
        std::string result = "";

        for (size_t i = 0; i < input.length(); ++i)
        {
            char buf[3];
            snprintf(buf, sizeof(buf), "%02X", static_cast<unsigned char>(input[i]));
            result += buf;
        }
        
        while (result.length() % 4 != 0) 
        {
            result += "0";
        }

        std::string inverted;
        for (size_t i = 0; i < result.length(); i += 4)
        {
            inverted += result.substr(i + 2, 2);
            inverted += result.substr(i, 2);
        }
        
        return inverted;
    }

    bool MelsecClient::isHexMemory(const jvs::node_area_e type)
    {
        switch (type)
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

    std::string MelsecClient::buildAsciiRequestData(jvs::node_area_e area, uint32_t address, int count, melsec_command_e command, bool isBit)
    {
        std::string result = "";
        switch (command)
        {
        case melsec_command_e::BATCH_READ:
            result += "0401";
            break;
        case melsec_command_e::BATCH_WRITE:
            result += "1401";
            break;
        default:
            result += "0000";
            break;
        }

        if (mPlcSeries == jvs::ps_e::IQR_SERIES) 
        {
            result += (isBit ? "0003" : "0002");
            std::string deviceCode = getDeviceCodeASCII(area);
            while (deviceCode.length() < 4) 
            {
                deviceCode += '*';
            }
            result += deviceCode;
        } 
        else 
        {
            result += (isBit ? "0001" : "0000");
            result += getDeviceCodeASCII(area);
        }

        if (isHexMemory(area))
        {
            char hexStr[7];  // 6자리 + 널종료
            snprintf(hexStr, sizeof(hexStr), "%06X", address);
            result += hexStr;
        }
        else
        {
            std::string addrStr = std::to_string(address);
            while (addrStr.length() < 6)
            {
                addrStr = "0" + addrStr;
            }
            result += addrStr;
        }

        char buf[5];
        snprintf(buf, sizeof(buf), "%04X", count);
        result += buf;

        return result;
    }

    int MelsecClient::buildBinaryRequestData(uint8_t* frame, size_t index, jvs::node_area_e area, uint32_t address, int count, melsec_command_e command, bool isBit)
    {
        switch (command)
        {
        case melsec_command_e::BATCH_READ:
            frame[index++] = static_cast<uint8_t>(0x01);
            frame[index++] = static_cast<uint8_t>(0x04);
            break;
        case melsec_command_e::BATCH_WRITE:
            frame[index++] = static_cast<uint8_t>(0x01);
            frame[index++] = static_cast<uint8_t>(0x14);
            break;
        default:
            frame[index++] = static_cast<uint8_t>(0x00);
            frame[index++] = static_cast<uint8_t>(0x00);
            break;
        }

        // Subcommand
        if (mPlcSeries == jvs::ps_e::IQR_SERIES) 
        {
            frame[index++] = static_cast<uint8_t>(isBit ? 0x03 : 0x02);
            frame[index++] = 0x00;
        } 
        else 
        {
            frame[index++] = static_cast<uint8_t>(isBit ? 0x01 : 0x00);
            frame[index++] = 0x00;
        }

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
        if (mPlcSeries == jvs::ps_e::IQR_SERIES) 
        {
            frame[index++] = getDeviceCodeBinary(area);
            frame[index++] = 0x00;
        } 
        else 
        {
            frame[index++] = getDeviceCodeBinary(area); // 예: M=0x90, D=0xA8 등
        }
        

        // 5. 읽기/쓰기 개수
        frame[index++] = static_cast<uint8_t>(count & 0xFF);
        frame[index++] = static_cast<uint8_t>((count >> 8) & 0xFF);

        return index;
    }

    std::string MelsecClient::batchReadWrite(jvs::node_area_e area, uint32_t address, int count, melsec_command_e command, bool isBit, const std::string &dataToWrite) 
    {
        if (mDataFormat == jvs::df_e::ASCII) 
        {
            // 기존 ASCII 코드 유지
            std::string result = "";
            char buf[5];
            snprintf(buf, sizeof(buf), "%04X", mCommonHeader.MonitoringTimer);
            result += buf;
            result += buildAsciiRequestData(area,address,count,command,isBit);

            if (!(command == melsec_command_e::BATCH_READ) && !dataToWrite.empty())
            {
                result += dataToWrite;
            }

            snprintf(buf, sizeof(buf), "%04X", static_cast<int>(result.length()));
            std::string fullFrame = buildAsciiCommonHeader() + buf + result;
            return fullFrame;
        }

        // ✅ BINARY 프레임 생성
        uint8_t frame[1024];

        // 1. 서두부 (Subheader ~ Request data length)
        int index = buildBinaryCommonHeader(frame);

        // 임시로 나중에 채우는 요청 길이 위치 기억
        int lengthPos = index;
        frame[index++] = 0x00; // 데이터 길이 (2바이트, little-endian)
        frame[index++] = 0x00;

        // 3. 모니터링 타이머 (1000ms)
        frame[index++] = static_cast<uint8_t>(mCommonHeader.MonitoringTimer & 0xFF);
        frame[index++] = static_cast<uint8_t>((mCommonHeader.MonitoringTimer >> 8) & 0xFF);
        
        // Command
        index = buildBinaryRequestData(frame, index, area, address, count, command, isBit);

        // 6. 쓰기 데이터가 있다면 추가
        if (!(command == melsec_command_e::BATCH_READ) && !dataToWrite.empty())
        {
            for (size_t i = 0; i + 1 < dataToWrite.length(); i += 2)
            {
                std::string byteStr = dataToWrite.substr(i, 2);
                uint8_t b = static_cast<uint8_t>(strtoul(byteStr.c_str(), nullptr, 16));
                frame[index++] = b;
            }
        }

        // 요청 길이 계산하여 삽입
        uint16_t reqLen = static_cast<uint16_t>(index - 9);
        frame[lengthPos] = static_cast<uint8_t>(reqLen & 0xFF);
        frame[lengthPos + 1] = static_cast<uint8_t>((reqLen >> 8) & 0xFF);

        // Serial.print("[WRITE] SEND : ");
        // for (size_t i = 0; i < index; i++)
        // {
        //     Serial.printf("%02X ",frame[i]);
        // }
        // Serial.println();
        // 바이너리 전송 버퍼 준비 완료
        return std::string(reinterpret_cast<char*>(frame), index);
    }

    bool MelsecClient::WriteWords(jvs::node_area_e area, uint32_t address, int wordCount, const uint16_t data[])
    {
        uint8_t test[1024];
        size_t idx = 0;
        // idx = mMelsecBuilder.BuildWriteRequestDataASCII(mCommonHeader,mPlcSeries,false,area,address,wordCount,data,test);
        // Serial.print("[LAST] SEND : ");
        // for (size_t i = 0; i < idx; i++)
        // {
        //     Serial.print((char)test[i]);
        // }
        // Serial.println();

        idx = mMelsecBuilder.BuildWriteRequestDataBinary(mCommonHeader, mPlcSeries, false ,area, address, wordCount, data, test);
        uint8_t testResp[1024];
        size_t respSize =  mMelsecTCP.sendAndReceiveBinary(test,idx,testResp);

        Serial.print("[WRITE] RESP : ");
        for (size_t i = 0; i < respSize; i++)
        {
            Serial.printf("%02X ",testResp[i]);
        }
        Serial.println();

        std::string dataStr = "";

        if (mDataFormat == jvs::df_e::ASCII) 
        {
            // ASCII 모드: 워드를 HEX 문자열로 변환
            wordsToHexString(data, wordCount, dataStr);
        } 
        else 
        {
            // BINARY 모드: 워드를 바이트 배열로 변환 후 HEX로 인코딩
            char hex[5];
            for (int i = 0; i < wordCount; ++i)
            {
                uint8_t lsb = data[i] & 0xFF;
                uint8_t msb = (data[i] >> 8) & 0xFF;
                snprintf(hex, sizeof(hex), "%02X%02X", lsb, msb);
                dataStr += hex;
            }
        }

        std::string frame = batchReadWrite(area, address, wordCount, melsec_command_e::BATCH_WRITE, false, dataStr);

        std::string resp = sendAndReceive(frame);

        // log_d("writeWords response: %s", resp.c_str());

        return (resp != "ERROR");
    }


    bool MelsecClient::WriteWord(jvs::node_area_e area, uint32_t address, uint16_t word) 
    {
        return WriteWords(area, address, 1, &word);
    }

    bool MelsecClient::WriteBit(jvs::node_area_e area, uint32_t address, uint8_t value) 
    {
        bool data[1];
        data[0] = value;
        return WriteBits(area, address, 1, data);
    }

    bool MelsecClient::WriteBits(jvs::node_area_e area, uint32_t address, int count, const bool *values)
    {
        std::string data = "";

        if (mDataFormat == jvs::df_e::ASCII) 
        {
            for (int i = 0; i < count; i++) 
            {
                data += values[i] ? "1" : "0";
            }
        }
        else 
        {
            for (int i = 0; i < count; i += 2) 
            {
                uint8_t byte = 0x00;

                // 짝수 비트 → bit4
                if (values[i])
                {
                    byte |= (1 << 4);
                } 

                // 홀수 비트 → bit0
                if (i + 1 < count && values[i + 1])
                {
                    byte |= (1 << 0);
                } 

                char hex[3];
                snprintf(hex, sizeof(hex), "%02X", byte);
                data += hex;
            }
        }

        std::string frame = batchReadWrite(area, address, count, melsec_command_e::BATCH_WRITE, true, data);
        std::string resp = sendAndReceive(frame);
        // log_d("writeBits response : %s", resp.c_str());

        return (resp != "ERROR");
    }


    int MelsecClient::ReadWords(jvs::node_area_e area, uint32_t address, int wordCount, uint16_t buffer[]) 
    {
        uint8_t test[1024];
        size_t idx = 0;
        // idx = mMelsecBuilder.BuildReadRequestDataASCII(mCommonHeader, mPlcSeries, false, area, address, wordCount, test);
        

        // Serial.print("[LAST] SEND : ");
        // for (size_t i = 0; i < idx; i++)
        // {
        //     Serial.print((char)test[i]);
        // }
        // Serial.println();

        idx = mMelsecBuilder.BuildReadRequestDataBinary(mCommonHeader, mPlcSeries, false, area, address, wordCount, test);
        
        uint8_t testResp[1024];
        size_t respSize =  mMelsecTCP.sendAndReceiveBinary(test,idx,testResp);

        Serial.print("[WORD] RESP : ");
        for (size_t i = 0; i < respSize; i++)
        {
            Serial.printf("%02X ",testResp[i]);
        }
        Serial.println();
        
        Status ret = mMelsecParser.ParseReadResponseBinary(testResp, respSize, false, buffer);
        if (ret != Status(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "ERROR : %s",ret.c_str());
        }

        for (size_t i = 0; i < wordCount; i++)
        {
            LOG_DEBUG(logger,"DATA[%d] : %d",i, buffer[i]);
        }
        
        
        // Serial.print("[LAST] SEND : ");
        // for (size_t i = 0; i < idx; i++)
        // {
        //     Serial.printf("%02X ",test[i]);
        // }
        // Serial.println();
        


        if (mDataFormat == jvs::df_e::ASCII) 
        {
            // 기존 ASCII 코드 유지
            std::string result = "";
            char buf[5];
            snprintf(buf, sizeof(buf), "%04X", mCommonHeader.MonitoringTimer);
            result += buf;
            result += buildAsciiRequestData(area,address,wordCount,melsec_command_e::BATCH_READ,false);
            snprintf(buf, sizeof(buf), "%04X", static_cast<int>(result.length()));
            // log_d("SEND : %s",(buildAsciiCommonHeader() + buf + result).c_str());
            std::string resp = sendAndReceive((buildAsciiCommonHeader() + buf + result));

            std::string data = extractAsciiData(resp);
            if (data.empty() || data.length() < static_cast<size_t>(wordCount * 4)) 
            {
                return 0;
            }

            return hexStringToWords(data, buffer);
        }
        else
        {
            // ✅ BINARY 프레임 생성
            uint8_t frame[1024];

            // 1. 서두부 (Subheader ~ Request data length)
            int index = buildBinaryCommonHeader(frame);

            // 임시로 나중에 채우는 요청 길이 위치 기억
            int lengthPos = index;
            frame[index++] = 0x00; // 데이터 길이 (2바이트, little-endian)
            frame[index++] = 0x00;

            // 3. 모니터링 타이머 (1000ms)
            frame[index++] = static_cast<uint8_t>(mCommonHeader.MonitoringTimer & 0xFF);
            frame[index++] = static_cast<uint8_t>((mCommonHeader.MonitoringTimer >> 8) & 0xFF);
            
            // Command
            index = buildBinaryRequestData(frame, index, area, address, wordCount, melsec_command_e::BATCH_READ, false);

            // 요청 길이 계산하여 삽입
            uint16_t reqLen = static_cast<uint16_t>(index - 9);
            frame[lengthPos] = static_cast<uint8_t>(reqLen & 0xFF);
            frame[lengthPos + 1] = static_cast<uint8_t>((reqLen >> 8) & 0xFF);
            // Serial.print("[actual] SEND : ");
            // for (size_t i = 0; i < index; i++)
            // {
            //     Serial.printf("%02X ",frame[i]);
            // }
            // Serial.println();
            std::string resp = sendAndReceive(std::string(reinterpret_cast<char*>(frame), index));
            std::string data = extractBinaryData(resp); 

            if (data.length() < static_cast<size_t>(wordCount * 4))
            {
                return 0;
            }

            for (int i = 0; i < wordCount; i++) 
            {
                int byteIdx = i * 4; 
                uint8_t lsb = static_cast<uint8_t>(strtoul(data.substr(byteIdx, 2).c_str(), nullptr, 16));
                uint8_t msb = static_cast<uint8_t>(strtoul(data.substr(byteIdx + 2, 2).c_str(), nullptr, 16));
                buffer[i] = (msb << 8) | lsb;
            }

            return wordCount;
        }
    }


    int MelsecClient::ReadBits(jvs::node_area_e area, uint32_t address, int count, bool *buffer) 
    {
        uint8_t test[1024];
        size_t idx = 0;
        // idx = mMelsecBuilder.BuildReadRequestDataASCII(mCommonHeader, mPlcSeries, false, area, address, wordCount, test);
        

        // Serial.print("[LAST] SEND : ");
        // for (size_t i = 0; i < idx; i++)
        // {
        //     Serial.print((char)test[i]);
        // }
        // Serial.println();

        idx = mMelsecBuilder.BuildReadRequestDataBinary(mCommonHeader, mPlcSeries, true, area, address, count, test);
        
        uint8_t testResp[1024];
        size_t respSize =  mMelsecTCP.sendAndReceiveBinary(test,idx,testResp);

        Serial.print("[BIT] RESP : ");
        for (size_t i = 0; i < respSize; i++)
        {
            Serial.printf("%02X ",testResp[i]);
        }
        Serial.println();

        if (mDataFormat == jvs::df_e::ASCII) 
        {
            std::string result = "";
            char buf[5];
            snprintf(buf, sizeof(buf), "%04X", mCommonHeader.MonitoringTimer);
            result += buf;
            result += buildAsciiRequestData(area, address, count, melsec_command_e::BATCH_READ, true);
            snprintf(buf, sizeof(buf), "%04X", static_cast<int>(result.length()));
            std::string resp = sendAndReceive((buildAsciiCommonHeader() + buf + result));

            std::string data = extractAsciiData(resp);
            if (data.empty() || data.length() < static_cast<size_t>(count))
            {
                return 0;
            }

            for (int i = 0; i < count && i < static_cast<int>(data.length()); ++i)
            {
                buffer[i] = (data[i] == '1');
            }

            return count;
        }
        else
        {
            // ✅ BINARY 프레임 생성
            uint8_t frame[1024];

            // 1. 서두부 (Subheader ~ Request data length)
            int index = buildBinaryCommonHeader(frame);

            // 임시로 나중에 채우는 요청 길이 위치 기억
            int lengthPos = index;
            frame[index++] = 0x00; // 데이터 길이 (2바이트, little-endian)
            frame[index++] = 0x00;

            // 3. 모니터링 타이머 (1000ms)
            frame[index++] = static_cast<uint8_t>(mCommonHeader.MonitoringTimer & 0xFF);
            frame[index++] = static_cast<uint8_t>((mCommonHeader.MonitoringTimer >> 8) & 0xFF);
            
            // Command
            index = buildBinaryRequestData(frame, index, area, address, count, melsec_command_e::BATCH_READ, true);

            // 요청 길이 계산하여 삽입
            uint16_t reqLen = static_cast<uint16_t>(index - 9);
            frame[lengthPos] = static_cast<uint8_t>(reqLen & 0xFF);
            frame[lengthPos + 1] = static_cast<uint8_t>((reqLen >> 8) & 0xFF);


            std::string temp = std::string(reinterpret_cast<char*>(frame), index);
            const uint8_t* cmd = reinterpret_cast<const uint8_t*>(temp.data());

            uint8_t response[256];
            int len = mMelsecTCP.sendAndReceiveBinary(cmd, static_cast<int>(temp.length()), response);
            if (len <= 11)
            {
                return 0;
            }

            const int payloadStart = 11;  // Header size

            for (int i = 0; i < count; i++) 
            {
                int byteIndex = payloadStart + (i / 2);
                if (byteIndex >= len)
                {
                    break;
                }

                uint8_t byte = response[byteIndex];
                bool bitVal;

                if (i % 2 == 0) 
                {
                    bitVal = (byte & (1 << 4)) != 0;
                } 
                else 
                {
                    bitVal = (byte & (1 << 0)) != 0;
                }

                buffer[i] = bitVal;
            }

            return count; 
        }
    }

    uint8_t MelsecClient::getDeviceCodeBinary(jvs::node_area_e area)
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

    std::string MelsecClient::getDeviceCodeASCII(jvs::node_area_e area)
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
}
