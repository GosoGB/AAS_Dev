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

    bool MelsecClient::Begin(const char *ip, uint16_t port, jvs::ps_e series) 
    {
        mIP = ip;
        mPort = port;
        mPlcSeries = series;

        if (mClient.connect(mIP,mPort)) 
        {
            mIsConnected = true;
        } 
        else 
        {
            mIsConnected = false;
            LOG_ERROR(logger,"TCP CONNECTION ERROR");
        }

        return mIsConnected;
    }

    bool MelsecClient::Connected() 
    {
        return mIsConnected;
    }

    void MelsecClient::Close()
    {
        mClient.stop();
        mIsConnected = false;
    }

    bool MelsecClient::WriteWords(jvs::node_area_e area, uint32_t address, size_t count, const uint16_t value[])
    {
        if (mIsConnected == false)
        {
            return 0;
        }
        
        uint8_t reqFrame[1024];
        uint8_t respFrame[1024];
        size_t idx = 0;

        if (mDataFormat == jvs::df_e::ASCII) 
        {
            idx = mMelsecBuilder.BuildWriteRequestDataASCII(mCommonHeader, mPlcSeries, false, area, address, count, value, reqFrame);
            size_t respSize =  sendAndReceive(reqFrame, idx, respFrame);
            if (respSize == 0)
            {
                LOG_ERROR(logger, "CONNECTION ERROR");
                return false;
            }

            Status ret = mMelsecParser.ParseWriteResponseASCII(respFrame, respSize, false);
            if (ret != Status(Status::Code::GOOD))
            {
                LOG_ERROR(logger, "ERROR : %s",ret.c_str());
                return false;
            }
            
            return true;
        } 
        else 
        {
            idx = mMelsecBuilder.BuildWriteRequestDataBinary(mCommonHeader, mPlcSeries, false, area, address, count, value, reqFrame);
            size_t respSize =  sendAndReceive(reqFrame, idx, respFrame);
            if (respSize == 0)
            {
                LOG_ERROR(logger, "CONNECTION ERROR");
                return false;
            }

            Status ret = mMelsecParser.ParseWriteResponseBinary(respFrame, respSize, false);
            if (ret != Status(Status::Code::GOOD))
            {
                LOG_ERROR(logger, "ERROR : %s",ret.c_str());
                return false;
            }
            
            return true;
        }
    }


    bool MelsecClient::WriteWord(jvs::node_area_e area, uint32_t address, uint16_t value) 
    {
        return WriteWords(area, address, 1, &value);
    }

    bool MelsecClient::WriteBit(jvs::node_area_e area, uint32_t address, uint16_t value) 
    {
        uint16_t data[1];
        data[0] = value;
        return WriteBits(area, address, 1, data);
    }

    bool MelsecClient::WriteBits(jvs::node_area_e area, uint32_t address, size_t count, const uint16_t value[])
    {
        if (mIsConnected == false)
        {
            return 0;
        }

        uint8_t reqFrame[1024];
        uint8_t respFrame[1024];
        size_t idx = 0;

        if (mDataFormat == jvs::df_e::ASCII) 
        {
            idx = mMelsecBuilder.BuildWriteRequestDataASCII(mCommonHeader, mPlcSeries, true, area, address, count, value, reqFrame);
            size_t respSize =  sendAndReceive(reqFrame, idx, respFrame);
            if (respSize == 0)
            {
                LOG_ERROR(logger, "CONNECTION ERROR");
                return false;
            }

            Status ret = mMelsecParser.ParseWriteResponseASCII(respFrame, respSize, true);
            if (ret != Status(Status::Code::GOOD))
            {
                LOG_ERROR(logger, "ERROR : %s",ret.c_str());
                return false;
            }
            
            return true;
        } 
        else 
        {
            idx = mMelsecBuilder.BuildWriteRequestDataBinary(mCommonHeader, mPlcSeries, true, area, address, count, value, reqFrame);
            size_t respSize =  sendAndReceive(reqFrame, idx, respFrame);
            if (respSize == 0)
            {
                LOG_ERROR(logger, "CONNECTION ERROR");
                return false;
            }

            Status ret = mMelsecParser.ParseWriteResponseBinary(respFrame, respSize, false);
            if (ret != Status(Status::Code::GOOD))
            {
                LOG_ERROR(logger, "ERROR : %s",ret.c_str());
                return false;
            }
            
            return true;
        }
    }


    int MelsecClient::ReadWords(jvs::node_area_e area, uint32_t address, size_t count, uint16_t buffer[]) 
    {
        if (mIsConnected == false)
        {
            return 0;
        }
        
        uint8_t reqFrame[1024];
        uint8_t respFrame[1024];
        size_t idx = 0;
    
        if (mDataFormat == jvs::df_e::ASCII) 
        {
            idx = mMelsecBuilder.BuildReadRequestDataASCII(mCommonHeader, mPlcSeries, false, area, address, count, reqFrame);
            size_t respSize =  sendAndReceive(reqFrame, idx, respFrame);
            if (respSize == 0)
            {
                LOG_ERROR(logger, "CONNECTION ERROR");
                Close();
                return 0;
            }

            Status ret = mMelsecParser.ParseReadResponseASCII(respFrame, respSize, false, buffer);
            if (ret != Status(Status::Code::GOOD))
            {
                LOG_ERROR(logger, "ERROR : %s",ret.c_str());
                return 0;
            }
            
            return count;
        }
        else
        {
            idx = mMelsecBuilder.BuildReadRequestDataBinary(mCommonHeader, mPlcSeries, false, area, address, count, reqFrame);
            size_t respSize =  sendAndReceive(reqFrame, idx, respFrame);
            if (respSize == 0)
            {
                LOG_ERROR(logger, "CONNECTION ERROR");
                Close();
                return 0;
            }

            Status ret = mMelsecParser.ParseReadResponseBinary(respFrame, respSize, false, buffer);
            if (ret != Status(Status::Code::GOOD))
            {
                LOG_ERROR(logger, "ERROR : %s",ret.c_str());
                return 0;
            }

            return count;
        }
    }


    int MelsecClient::ReadBits(jvs::node_area_e area, uint32_t address, size_t count, uint16_t buffer[]) 
    {
        if (mIsConnected == false)
        {
            return 0;
        }

        uint8_t reqFrame[1024];
        uint8_t respFrame[1024];
        size_t idx = 0;
    
        if (mDataFormat == jvs::df_e::ASCII) 
        {
            idx = mMelsecBuilder.BuildReadRequestDataASCII(mCommonHeader, mPlcSeries, true, area, address, count, reqFrame);
            size_t respSize =  sendAndReceive(reqFrame, idx, respFrame);
            if (respSize == 0)
            {
                LOG_ERROR(logger, "CONNECTION ERROR");
                Close();
                return 0;
            }

            Status ret = mMelsecParser.ParseReadResponseASCII(respFrame, respSize, true, buffer);
            if (ret != Status(Status::Code::GOOD))
            {
                LOG_ERROR(logger, "ERROR : %s",ret.c_str());
                return 0;
            }
            
            return count;
        }
        else
        {
            idx = mMelsecBuilder.BuildReadRequestDataBinary(mCommonHeader, mPlcSeries, true, area, address, count, reqFrame);
            size_t respSize = sendAndReceive(reqFrame, idx, respFrame);
            if (respSize == 0)
            {
                LOG_ERROR(logger, "CONNECTION ERROR");
                Close();
                return 0;
            }
            Status ret = mMelsecParser.ParseReadResponseBinary(respFrame, respSize, true, buffer);
            if (ret != Status(Status::Code::GOOD))
            {
                LOG_ERROR(logger, "ERROR : %s",ret.c_str());
                return 0;
            }

            return count;
        }
    }

    int MelsecClient::sendAndReceive(const uint8_t *cmd, int length, uint8_t *responseBuf) 
    {
        if (!mClient.connected()) 
        {
            return 0;
        }

        mClient.write(cmd, length);

        unsigned long startTS = millis();
        while (!mClient.available() && (millis() - startTS) < 1000) 
        {
            delay(1);
        }
        size_t idx = 0;
        
        while (mClient.available()>0)
        {
            responseBuf[idx] = mClient.read();
            idx++;
        }

        mClient.flush();
        
        return idx;
    }
}
