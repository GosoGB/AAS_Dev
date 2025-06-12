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
    MelsecClient* embededMelsecClient = nullptr;
    MelsecClient* link1MelsecClient = nullptr;

    #if defined(MT11)
    MelsecClient::MelsecClient(W5500& interface, const w5500::sock_id_e sock_id)
    :   mPort(0), 
        mIP(0,0,0,0), 
        mPlcSeries(jvs::ps_e::QL_SERIES), 
        mDataFormat(jvs::df_e::BINARY)
    {
        mClient = new w5500::EthernetClient(interface, sock_id);
    }
#else
    MelsecClient::MelsecClient() 
    :   mPort(0), 
        mIP(0,0,0,0), 
        mPlcSeries(jvs::ps_e::QL_SERIES), 
        mDataFormat(jvs::df_e::BINARY)
    {
        mClient = new WiFiClient();
    }
#endif



    MelsecClient::~MelsecClient()
    {   

    }
    

    bool MelsecClient::Begin(IPAddress ip, uint16_t port, jvs::ps_e series) 
    {
        mIP = ip;
        mPort = port;
        mPlcSeries = series;

        if (mClient->connect(mIP, mPort)) 
        {
            mIsConnected = true;
        } 
        else 
        {
            LOG_ERROR(logger,"TCP CONNECTION ERROR");
            mIsConnected = false;
        }

        return mIsConnected;
    }

    bool MelsecClient::Connected() 
    {
        return mIsConnected;
    }

    void MelsecClient::Close()
    {
        mClient->stop();
        mIsConnected = false;
    }

    bool MelsecClient::WriteWords(jvs::node_area_e area, uint32_t address, size_t count, const uint16_t value[])
    {
        if (mIsConnected == false)
        {
            return 0;
        }
        
        memset(mReqFrame,0,sizeof(mReqFrame));
        memset(mRespFrame,0,sizeof(mRespFrame));
        
        size_t idx = 0;

        if (mDataFormat == jvs::df_e::ASCII) 
        {
            idx = mMelsecBuilder.BuildWriteRequestDataASCII(mCommonHeader, mPlcSeries, false, area, address, count, value, mReqFrame);
            uint8_t trialCount = 0;
            size_t respSize = 0;

            for (trialCount = 0; trialCount < MAX_TRIAL_COUNT; ++trialCount)
            {
                respSize =  sendAndReceive(mReqFrame, idx, mRespFrame);
                if (respSize != 0)
                {
                    break;
                }
                else
                {
                    LOG_WARNING(logger, "CONNECTION ERROR #%u",trialCount);
                    mClient->stop();
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                    mClient->connect(mIP, mPort);
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                }
            }
            
            if (trialCount == MAX_TRIAL_COUNT)
            {
                LOG_ERROR(logger, "CONNECTION ERROR #%u",trialCount);
                return 0;
            }

            Status ret = mMelsecParser.ParseWriteResponseASCII(mRespFrame, respSize, false);
            if (ret != Status(Status::Code::GOOD))
            {
                LOG_ERROR(logger, "ERROR : %s",ret.c_str());
                return false;
            }
            
            return true;
        } 
        else 
        {
            idx = mMelsecBuilder.BuildWriteRequestDataBinary(mCommonHeader, mPlcSeries, false, area, address, count, value, mReqFrame);
            uint8_t trialCount = 0;
            size_t respSize = 0;

            for (trialCount = 0; trialCount < MAX_TRIAL_COUNT; ++trialCount)
            {
                respSize =  sendAndReceive(mReqFrame, idx, mRespFrame);
                if (respSize != 0)
                {
                    break;
                }
                else
                {
                    LOG_WARNING(logger, "CONNECTION ERROR #%u",trialCount);
                    mClient->stop();
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                    mClient->connect(mIP, mPort);
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                }
            }
            
            if (trialCount == MAX_TRIAL_COUNT)
            {
                LOG_ERROR(logger, "CONNECTION ERROR #%u",trialCount);
                return 0;
            }

            Status ret = mMelsecParser.ParseWriteResponseBinary(mRespFrame, respSize, false);
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

        memset(mReqFrame,0,sizeof(mReqFrame));
        memset(mRespFrame,0,sizeof(mRespFrame));

        size_t idx = 0;

        if (mDataFormat == jvs::df_e::ASCII) 
        {
            idx = mMelsecBuilder.BuildWriteRequestDataASCII(mCommonHeader, mPlcSeries, true, area, address, count, value, mReqFrame);
            uint8_t trialCount = 0;
            size_t respSize = 0;

            for (trialCount = 0; trialCount < MAX_TRIAL_COUNT; ++trialCount)
            {
                respSize =  sendAndReceive(mReqFrame, idx, mRespFrame);
                if (respSize != 0)
                {
                    break;
                }
                else
                {
                    LOG_WARNING(logger, "CONNECTION ERROR #%u",trialCount);
                    mClient->stop();
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                    mClient->connect(mIP, mPort);
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                }
            }
            
            if (trialCount == MAX_TRIAL_COUNT)
            {
                LOG_ERROR(logger, "CONNECTION ERROR #%u",trialCount);
                return 0;
            }

            Status ret = mMelsecParser.ParseWriteResponseASCII(mRespFrame, respSize, true);
            if (ret != Status(Status::Code::GOOD))
            {
                LOG_ERROR(logger, "ERROR : %s",ret.c_str());
                return false;
            }
            
            return true;
        } 
        else 
        {
            idx = mMelsecBuilder.BuildWriteRequestDataBinary(mCommonHeader, mPlcSeries, true, area, address, count, value, mReqFrame);
            uint8_t trialCount = 0;
            size_t respSize = 0;

            for (trialCount = 0; trialCount < MAX_TRIAL_COUNT; ++trialCount)
            {
                respSize =  sendAndReceive(mReqFrame, idx, mRespFrame);
                if (respSize != 0)
                {
                    break;
                }
                else
                {
                    LOG_WARNING(logger, "CONNECTION ERROR #%u",trialCount);
                    mClient->stop();
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                    mClient->connect(mIP, mPort);
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                }
            }
            
            if (trialCount == MAX_TRIAL_COUNT)
            {
                LOG_ERROR(logger, "CONNECTION ERROR #%u",trialCount);
                return 0;
            }

            Status ret = mMelsecParser.ParseWriteResponseBinary(mRespFrame, respSize, false);
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

        memset(mReqFrame,0,sizeof(mReqFrame));
        memset(mRespFrame,0,sizeof(mRespFrame));
       
        size_t idx = 0;
    
        if (mDataFormat == jvs::df_e::ASCII) 
        {
            idx = mMelsecBuilder.BuildReadRequestDataASCII(mCommonHeader, mPlcSeries, false, area, address, count, mReqFrame);
            uint8_t trialCount = 0;
            size_t respSize = 0;

            for (trialCount = 0; trialCount < MAX_TRIAL_COUNT; ++trialCount)
            {
                respSize =  sendAndReceive(mReqFrame, idx, mRespFrame);
                if (respSize != 0)
                {
                    break;
                }
                else
                {
                    LOG_WARNING(logger, "CONNECTION ERROR #%u",trialCount);
                    mClient->stop();
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                    mClient->connect(mIP, mPort);
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                }
            }
            
            if (trialCount == MAX_TRIAL_COUNT)
            {
                LOG_ERROR(logger, "CONNECTION ERROR #%u",trialCount);
                return 0;
            }
            
            

            Status ret = mMelsecParser.ParseReadResponseASCII(mRespFrame, respSize, false, buffer);
            if (ret != Status(Status::Code::GOOD))
            {
                LOG_ERROR(logger, "ERROR : %s",ret.c_str());
                return 0;
            }
            
            return count;
        }
        else
        {
            idx = mMelsecBuilder.BuildReadRequestDataBinary(mCommonHeader, mPlcSeries, false, area, address, count, mReqFrame);
            uint8_t trialCount = 0;
            size_t respSize = 0;

            for (trialCount = 0; trialCount < MAX_TRIAL_COUNT; ++trialCount)
            {
                respSize = sendAndReceive(mReqFrame, idx, mRespFrame);
                if (respSize != 0)
                {
                    break;
                }
                else
                {
                    LOG_WARNING(logger, "CONNECTION ERROR #%u",trialCount);
                    mClient->stop();
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                    mClient->connect(mIP, mPort);
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                }
            }
            
            if (trialCount == MAX_TRIAL_COUNT)
            {
                LOG_ERROR(logger, "CONNECTION ERROR #%u",trialCount);
                return 0;
            }
            
            
            Status ret = mMelsecParser.ParseReadResponseBinary(mRespFrame, respSize, count, false, buffer);
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

        memset(mReqFrame,0,sizeof(mReqFrame));
        memset(mRespFrame,0,sizeof(mRespFrame));

        size_t idx = 0;
    
        if (mDataFormat == jvs::df_e::ASCII) 
        {
            idx = mMelsecBuilder.BuildReadRequestDataASCII(mCommonHeader, mPlcSeries, true, area, address, count, mReqFrame);
            uint8_t trialCount = 0;
            size_t respSize = 0;

            for (trialCount = 0; trialCount < MAX_TRIAL_COUNT; ++trialCount)
            {
                respSize =  sendAndReceive(mReqFrame, idx, mRespFrame);
                if (respSize != 0)
                {
                    break;
                }
                else
                {
                    LOG_WARNING(logger, "CONNECTION ERROR #%u",trialCount);
                    mClient->stop();
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                    mClient->connect(mIP, mPort);
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                }
            }
            
            if (trialCount == MAX_TRIAL_COUNT)
            {
                LOG_ERROR(logger, "CONNECTION ERROR #%u",trialCount);
                return 0;
            }

            Status ret = mMelsecParser.ParseReadResponseASCII(mRespFrame, respSize, true, buffer);
            if (ret != Status(Status::Code::GOOD))
            {
                LOG_ERROR(logger, "ERROR : %s",ret.c_str());
                return 0;
            }
            
            return count;
        }
        else
        {
            idx = mMelsecBuilder.BuildReadRequestDataBinary(mCommonHeader, mPlcSeries, true, area, address, count, mReqFrame);
            uint8_t trialCount = 0;
            size_t respSize = 0;

            for (trialCount = 0; trialCount < MAX_TRIAL_COUNT; ++trialCount)
            {
                respSize =  sendAndReceive(mReqFrame, idx, mRespFrame);
                if (respSize != 0)
                {
                    break;
                }
                else
                {
                    LOG_WARNING(logger, "CONNECTION ERROR #%u",trialCount);
                    mClient->stop();
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                    mClient->connect(mIP, mPort);
                    vTaskDelay(100/ portTICK_PERIOD_MS);
                }
            }
            
            if (trialCount == MAX_TRIAL_COUNT)
            {
                LOG_ERROR(logger, "CONNECTION ERROR #%u",trialCount);
                return 0;
            }

            Status ret = mMelsecParser.ParseReadResponseBinary(mRespFrame, respSize, count, true, buffer);
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
        if (!mClient->connected()) 
        {
            return 0;
        }
        mClient->write(cmd, length);
        
        // Serial.print("SEND DATA : ");
        // for (size_t i = 0; i < length; i++)
        // {
        //     Serial.print((char)cmd[i]);
        // }
        // Serial.print("\r\n\r\n");

        uint32_t startTS = millis();
        while (!mClient->available() && (millis() - startTS) < 2000) 
        {
            delay(1);
        }
        size_t idx = 0;
        while (mClient->available()>0)
        {
            responseBuf[idx] = mClient->read();
            idx++;
        }

        // Serial.print("RESPONSE DATA : ");
        // for (size_t i = 0; i < idx; i++)
        // {
        //     Serial.print((char)responseBuf[i]);
        // }
        
        // Serial.print("\r\n\r\n");
        return idx;
    }
}
