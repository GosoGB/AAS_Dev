/**
 * @file CatMQTT.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 모듈의 MQTT 프로토콜 클래스를 정의합니다.
 * 
 * @date 2024-10-30
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "CatMQTT.h"
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Convert/ConvertClass.h"
#include "Network/Helper.h"
#include "Protocol/MQTT/Include/Helper.h"



namespace muffin { namespace mqtt {

    CatMQTT* CatMQTT::CreateInstanceOrNULL(CatM1& catM1, BrokerInfo& broker, Message& lwt)
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) CatMQTT(catM1, broker, lwt);
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMROY FOR CatMQTT");
                return mInstance;
            }
        }
        
        return mInstance;
    }

    CatMQTT* CatMQTT::CreateInstanceOrNULL(CatM1& catM1, BrokerInfo& broker)
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) CatMQTT(catM1, broker);
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMROY FOR CatMQTT");
                return mInstance;
            }
        }
        
        return mInstance;
    }

    CatMQTT& CatMQTT::GetInstance()
    {
        ASSERT((mInstance != nullptr), "NO INSTANCE CREATED: CALL FUNCTION \"CreateInstanceOrNULL\" IN ADVANCE");
        return *mInstance;
    }

    CatMQTT::CatMQTT(CatM1& catM1, BrokerInfo& broker, Message& lwt)
        : mCatM1(catM1)
        , mBrokerInfo(std::move(broker))
        , mMessageLWT(std::move(lwt))
    {
        mInitFlags.reset();
        mInitFlags.set(init_flag_e::ENABLE_LWT_MSG);
        mState = state_e::CONSTRUCTED;
    }
    
    CatMQTT::CatMQTT(CatM1& catM1, BrokerInfo& broker)
        : mCatM1(catM1)
        , mBrokerInfo(std::move(broker))
        , mMessageLWT(Message(topic_e::LAST_WILL, std::string()))
    {
        mInitFlags.reset();
        mInitFlags.reset(init_flag_e::ENABLE_LWT_MSG);
        mState = state_e::CONSTRUCTED;
        LOG_WARNING(logger, "LWT FEATURE IS TURNED OFF");
    }

    CatMQTT::~CatMQTT()
    {
    }

    void CatMQTT::OnEventReset()
    {
        this->mInitFlags.reset();
    }
    
    INetwork* CatMQTT::RetrieveNIC()
    {
        return static_cast<INetwork*>(&mCatM1);
    }

    Status CatMQTT::Init(const size_t mutexHandle, const network::lte::pdp_ctx_e pdp, const network::lte::ssl_ctx_e ssl)
    {
        /**
         * @todo 연결 끊어졌을 때 상태를 다시 초기화해야 합니다. 그 다음 아래 assert를 다시 활성화시켜야 합니다.
         */
        // ASSERT((mState != state_e::INITIALIZED), "REINITIALIZATION IS FORBIDDEN");
        mState = state_e::DISCONNECTED;

        Status ret = Status(Status::Code::UNCERTAIN);

        if (mInitFlags.test(init_flag_e::INITIALIZED_PDP) == false)
        {
            ret = setPdpContext(mutexHandle, pdp);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAIL TO SET PDP CONTEXT: %s", ret.c_str());
                goto INIT_FAILED;
            }
            mInitFlags.set(init_flag_e::INITIALIZED_PDP);
        }
        
        if (mInitFlags.test(init_flag_e::INITIALIZED_SSL) == false)
        {
            ret = setSslContext(mutexHandle, ssl);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAIL TO SET SSL CONTEXT: %s", ret.c_str());
                goto INIT_FAILED;
            }
            mInitFlags.set(init_flag_e::INITIALIZED_SSL);
        }

        if (mInitFlags.test(init_flag_e::INITIALIZED_VSN) == false)
        {
            ret = setVersion(mutexHandle);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAIL TO SET VERSION: %s", ret.c_str());
                goto INIT_FAILED;
            }

            ret = checkVersion(mutexHandle);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "VERSION NOT APPLIED PROPERLY: %s", ret.c_str());
                goto INIT_FAILED;
            }
            mInitFlags.set(init_flag_e::INITIALIZED_VSN);
        }

        // if (mInitFlags.test(init_flag_e::INITIALIZED_LWT) == false)
        if (true)
        {
            ret = setLastWill(mutexHandle);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAIL TO SET LWT MESSAGE: %s", ret.c_str());
                goto INIT_FAILED;
            }

            // ret = checkLastWill(mutexHandle);
            // if (ret != Status::Code::GOOD)
            // {
            //     LOG_ERROR(logger, "LWT NOT APPLIED PROPERLY: %s", ret.c_str());
            //     goto INIT_FAILED;
            // }
            mInitFlags.set(init_flag_e::INITIALIZED_LWT);
        }

        if (mInitFlags.test(init_flag_e::INITIALIZED_KAT) == false)
        {
            ret = setKeepAlive(mutexHandle);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAIL TO SET KEEP ALIVE: %s", ret.c_str());
                goto INIT_FAILED;
            }
            mInitFlags.set(init_flag_e::INITIALIZED_KAT);
        }        

        LOG_INFO(logger, "Initialized successfully");
        mInitFlags.set(init_flag_e::INITIALIZED_ALL);
        mState = (mState == state_e::CONNECTED) ? state_e::CONNECTED : state_e::INITIALIZED;
        return Status(Status::Code::GOOD);

    INIT_FAILED:
        mState = state_e::INIT_FAILED;
        return ret;
    }

    Status CatMQTT::Connect(const size_t mutexHandle)
    {
        /**
         * @todo 연결 끊어졌을 때 상태를 다시 초기화해야 합니다. 그 다음 아래 assert를 다시 활성화시켜야 합니다.
         */
        // ASSERT((mState == state_e::INITIALIZED), "MUST BE INITIALIZED PRIOR TO \"Connect()\"");

        if (mState == state_e::CONNECTED)
        {
            LOG_WARNING(logger, "ALREADY CONNECTED. NOTHING TO DO");
            return Status(Status::Code::GOOD);
        }

        Status ret = openSession(mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO OPEN MQTT SESSION: %s", ret.c_str());
            return ret;
        }
        
        ret = connectBroker(mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT TO MQTT BROKER: %s", ret.c_str());
            return ret;
        }
        
        LOG_INFO(logger, "Connected to broker: %s", ret.c_str());
        mState = state_e::CONNECTED;
        return ret;
    }

    Status CatMQTT::Disconnect(const size_t mutexHandle)
    {
    /*  ASSERT((mState == state_e::INITIALIZED), "MUST BE INITIALIZED PRIOR TO \"Disconnect()\"");

        Status ret = IsConnected();
        if (ret != Status::Code::GOOD)
        {
            LOG_WARNING(logger, "NO CONNECTION OR SESSION TO DISCONNECT");
            return Status(Status::Code::GOOD);
        }*/

        Status ret = disconnectBroker(mutexHandle);
        // if (ret != Status::Code::GOOD)
        // {
        //     LOG_ERROR(logger, "FAILED TO DISCONNECT FROM THE MQTT BROKER: %s", ret.c_str());
        //     return ret;
        // }
        
        ret = closeSession(mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CLOSE MQTT SESSION: %s", ret.c_str());
            return ret;
        }
        
        LOG_INFO(logger, "Disconnected from broker: %s", ret.c_str());
        mState = state_e::DISCONNECTED;
        return ret;
    }

    Status CatMQTT::IsConnected()
    {
        ASSERT((mState >= state_e::INITIALIZED), "MUST BE INITIALIZED PRIOR TO \"IsConnected()\"");

        switch (mState)
        {
        case state_e::CONNECT_FAILED:
        case state_e::INIT_FAILED:
        case state_e::INITIALIZED:
            LOG_WARNING(logger, "CONNECTION FAILED OR NOT INITIALIZED");
            return Status(Status::Code::BAD_REQUEST_NOT_COMPLETE);
        case state_e::CONNECTED:
            // LOG_VERBOSE(logger, "Connected to the broker");
            return Status(Status::Code::GOOD);
        case state_e::DISCONNECTED:
            LOG_WARNING(logger, "DISCONNECTED FROM THE BROKER");
            return Status(Status::Code::BAD_DISCONNECT);
        default:
            ASSERT(false, "UNDEFINED STATE: %d", static_cast<int8_t>(mState));
            return Status(Status::Code::BAD_INVALID_STATE);
        }
    }

    Status CatMQTT::Subscribe(const size_t mutexHandle, const std::vector<Message>& messages)
    {
        ASSERT((mState == state_e::CONNECTED), "MUST BE CONNECTED TO THE BROKER PRIOR TO \"Subscribe()\"");
        ASSERT((messages.size() != 0), "NUMBER OF TOPICS TO SUBSCRIBE CANNOT BE 0");

        if (mState != state_e::CONNECTED)
        {
            LOG_ERROR(logger, "REQUEST FAILED: NOT CONNECTED TO THE BROKER");
            return Status(Status::Code::BAD_REQUEST_CANCELLED_BY_CLIENT);
        }

        const uint8_t brokerSocketID = static_cast<uint8_t>(mBrokerInfo.GetSocketID());
        constexpr uint8_t MESSAGE_ID = 1;

        std::string command = "AT+QMTSUB="
            + std::to_string(brokerSocketID) + ","
            + std::to_string(MESSAGE_ID);
            
        constexpr uint8_t BUFFER_SIZE = 64;
        char buffer[BUFFER_SIZE];
        for (const Message& message : messages)
        {
            memset(buffer, '\0', sizeof(buffer));
            sprintf(buffer, ",\"%s\",%u", message.GetTopicString(), static_cast<uint8_t>(message.GetQoS()));
            ASSERT((strlen(buffer) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");
            command.append(buffer);
        }

        LOG_INFO(logger,"command : %s", command.c_str());

        const uint32_t timeoutMillis = 15 * 1000;
        const uint32_t startedMillis = millis();
        std::string rxd;

        Status ret = mCatM1.Execute(command, mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SUBSCRIBE: %s", ret.c_str());
            return ret;
        }
        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SUBSCRIBE: %s: %s", ret.c_str(), processCmeErrorCode(rxd).c_str());
            return ret;
        }

        const std::string patternBegin = "\r\n+QMTSUB: ";
        const std::string patternEnd   = "\r\n";
        rxd.clear();

        while (millis() - startedMillis < timeoutMillis)
        {
            const std::string betweenPatterns = mCatM1.ReadBetweenPatterns(patternBegin, patternEnd);
            if (betweenPatterns.length() == 0)
            {
                vTaskDelay(50 / portTICK_PERIOD_MS);
                continue;
            }
            rxd.append(betweenPatterns);
            break;
        }

        std::vector<size_t> vecDelimiter;
        vecDelimiter.emplace_back(rxd.find(' '));
        if (vecDelimiter.front() == std::string::npos)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        size_t position = std::string::npos;
        do
        {
            position = rxd.find(',', vecDelimiter.back() + 1);
            if (position != std::string::npos && position != rxd.length())
            {
                vecDelimiter.emplace_back(position);
            }
        } while (position != std::string::npos);
        vecDelimiter.emplace_back(rxd.find('\r', vecDelimiter.back() + 1));

        constexpr uint8_t DEFAULT_DELIMITER_COUNT = 4;
        if (DEFAULT_DELIMITER_COUNT + messages.size() != vecDelimiter.size())
        {
            LOG_ERROR(logger, "INVALID NUMBER OF DELIMITER: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        
        std::vector<size_t>::iterator it = vecDelimiter.begin();
        const std::string strSocketID  = rxd.substr(*it + 1, *(it + 1) - *it - 1);
        const std::string strMessageID = rxd.substr(*(++it) + 1, *(it + 1) - *(it) - 1);
        const std::string strResult    = rxd.substr(*(++it) + 1, *(it + 1) - *(it) - 1);
        const int32_t rxdSocketID  = Convert.ToInt32(strSocketID.c_str());
        const int32_t rxdMessageID = Convert.ToInt32(strMessageID.c_str());
        const int32_t rxdResult    = Convert.ToInt32(strResult.c_str());

        if (rxdSocketID == INT32_MAX || rxdMessageID == INT32_MAX || rxdResult == INT32_MAX)
        {
            //LOG_DEBUG(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        else if (rxdSocketID != brokerSocketID || rxdMessageID != MESSAGE_ID)
        {
            //LOG_DEBUG(logger, "INVALID SOCKET OR MESSAGE ID: %u, %u", rxdSocketID, rxdMessageID);
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        else
        {
            if (rxdResult == 2)
            {
                LOG_ERROR(logger, "FAILED TO SEND PACKET");
                return Status(Status::Code::BAD_REQUEST_NOT_COMPLETE);
            }

            const uint8_t arraySize = vecDelimiter.size() - DEFAULT_DELIMITER_COUNT;
            uint8_t mArrayValue[arraySize];
            for (size_t i = 0; i < arraySize; i++)
            {
                const std::string str = rxd.substr(*(++it) + 1, *(it + 1) - *it - 1);
                const int32_t val = Convert.ToInt32(str.c_str());
                if (val == INT32_MAX)
                {
                    //LOG_DEBUG(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
                    return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
                }
                mArrayValue[i] = val;
            }

            std::string strValueVector = "{ ";
            for (size_t i = 0; i < arraySize; i++)
            {
                if (i < arraySize - 1)
                {
                    strValueVector.append(std::to_string(mArrayValue[i]) + ", ");
                }
                else
                {
                    strValueVector.append(std::to_string(mArrayValue[i]) + " }");
                }
            }

            if (rxdResult == 0)
            {
                LOG_INFO(logger, "Subscribed with QoS: %s", strValueVector.c_str());
                return Status(Status::Code::GOOD);
            }
            else
            {
                LOG_WARNING(logger, "RETRANSMITTED: %s", strValueVector.c_str());
                return Status(Status::Code::UNCERTAIN);
            }
        }
    }

    Status CatMQTT::Unsubscribe(const size_t mutexHandle, const std::vector<Message>& messages)
    {
        ASSERT((mState == state_e::CONNECTED), "MUST BE CONNECTED TO THE BROKER PRIOR TO \"Unsubscribe()\"");
        ASSERT((messages.size() != 0), "NUMBER OF TOPICS TO UNSUBSCRIBE CANNOT BE 0");

        if (mState != state_e::CONNECTED)
        {
            LOG_ERROR(logger, "REQUEST FAILED: NOT CONNECTED TO THE BROKER");
            return Status(Status::Code::BAD_REQUEST_CANCELLED_BY_CLIENT);
        }

        const uint8_t brokerSocketID = static_cast<uint8_t>(mBrokerInfo.GetSocketID());
        constexpr uint8_t MESSAGE_ID = 1;

        std::string command = "AT+QMTUNS="
            + std::to_string(brokerSocketID) + ","
            + std::to_string(MESSAGE_ID);
            
        constexpr uint8_t BUFFER_SIZE = 64;
        char buffer[BUFFER_SIZE];
        for (const Message& message : messages)
        {
            memset(buffer, '\0', sizeof(buffer));
            sprintf(buffer, ",\"%s\"", message.GetTopicString());
            ASSERT((strlen(buffer) < (BUFFER_SIZE - 1)), "BUFFER OVERFLOW ERROR");
            command.append(buffer);
        }

        const uint32_t timeoutMillis = 15 * 1000;
        const uint32_t startedMillis = millis();
        std::string rxd;

        Status ret = mCatM1.Execute(command, mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO UNSUBSCRIBE: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO UNSUBSCRIBE: %s: %s", ret.c_str(), processCmeErrorCode(rxd).c_str());
            return ret;
        }

        const std::string patternBegin = "\r\n+QMTUNS: ";
        const std::string patternEnd   = "\r\n";
        rxd.clear();

        while (millis() - startedMillis < timeoutMillis)
        {
            const std::string betweenPatterns = mCatM1.ReadBetweenPatterns(patternBegin, patternEnd);
            if (betweenPatterns.length() == 0)
            {
                vTaskDelay(50 / portTICK_PERIOD_MS);
                continue;
            }
            rxd.append(betweenPatterns);
            break;
        }

        std::vector<size_t> vecDelimiter;
        vecDelimiter.emplace_back(rxd.find(' '));
        if (vecDelimiter.front() == std::string::npos)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        size_t position = std::string::npos;
        do
        {
            position = rxd.find(',', vecDelimiter.back() + 1);
            if (position != std::string::npos && position != rxd.length())
            {
                vecDelimiter.emplace_back(position);
            }
        } while (position != std::string::npos);
        vecDelimiter.emplace_back(rxd.find('\r', vecDelimiter.back() + 1));

        constexpr uint8_t TARGET_DELIMITER_COUNT = 4;
        if (TARGET_DELIMITER_COUNT != vecDelimiter.size())
        {
            LOG_ERROR(logger, "INVALID NUMBER OF DELIMITER: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        std::vector<size_t>::iterator it = vecDelimiter.begin();
        const std::string strSocketID  = rxd.substr(*it + 1, *(it + 1) - *it - 1);
        const std::string strMessageID = rxd.substr(*(++it) + 1, *(it + 1) - *(it) - 1);
        const std::string strResult    = rxd.substr(*(++it) + 1, *(it + 1) - *(it) - 1);
        const int32_t rxdSocketID  = Convert.ToInt32(strSocketID.c_str());
        const int32_t rxdMessageID = Convert.ToInt32(strMessageID.c_str());
        const int32_t rxdResult    = Convert.ToInt32(strResult.c_str());

        if (rxdSocketID == INT32_MAX || rxdMessageID == INT32_MAX || rxdResult == INT32_MAX)
        {
            //LOG_DEBUG(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        else if (rxdSocketID != brokerSocketID || rxdMessageID != MESSAGE_ID)
        {
            //LOG_DEBUG(logger, "INVALID SOCKET OR MESSAGE ID: %u, %u", rxdSocketID, rxdMessageID);
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        else
        {
            switch (rxdResult)
            {
            case 0:
                LOG_INFO(logger, "Packet sent successfully and received ACK from server: %u", rxdSocketID);
                return Status(Status::Code::GOOD);
            case 1:
                LOG_ERROR(logger, "RETRANSMITTING UNSUBSCRIBE PACKETS: %u", rxdSocketID);
                return Status(Status::Code::UNCERTAIN);
            case 2:
                LOG_ERROR(logger, "FAILED TO SEND UNSUBSCRIBE PACKETS: %u", rxdSocketID);
                return Status(Status::Code::BAD_REQUEST_NOT_COMPLETE);
            default:
                LOG_ERROR(logger, "UNDEFINED RESULT: %u", rxdResult);
                return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
            }
        }
    }

    Status CatMQTT::Publish(const size_t mutexHandle, const Message& message)
    {
        ASSERT((mState == state_e::CONNECTED), "MUST BE CONNECTED TO THE BROKER PRIOR TO \"Unsubscribe()\"");
        ASSERT((strlen(message.GetPayload()) < 4097), "PAYLOAD SIZE CANNOT EXCEED 4,096 BYTES");
        ASSERT((message.GetSocketID() == mBrokerInfo.GetSocketID()), 
            "INVALID SOCKET ID: \"Broker\": %u,  \"Message\": %u",
            static_cast<uint8_t>(mBrokerInfo.GetSocketID()), 
            static_cast<uint8_t>(message.GetSocketID())
        );

        if (mState != state_e::CONNECTED)
        {
            LOG_ERROR(logger, "REQUEST FAILED: NOT CONNECTED TO THE BROKER");
            return Status(Status::Code::BAD_REQUEST_CANCELLED_BY_CLIENT);
        }

        const uint8_t msgSocketID   = static_cast<uint8_t>(message.GetSocketID());
        const uint16_t msgMessageID = message.GetMessageID();
        const uint8_t msgQoS        = static_cast<uint8_t>(message.GetQoS());
        const uint8_t msgRetain     = static_cast<uint8_t>(message.IsRetain());
        const size_t msgLength      = strlen(message.GetPayload());

        const std::string command = "AT+QMTPUB="
            + std::to_string(msgSocketID)   + ","
            + std::to_string(msgMessageID)  + ","
            + std::to_string(msgQoS)        + ","
            + std::to_string(msgRetain)     + ",\""
            + message.GetTopicString()      + "\","
            + std::to_string(msgLength);

        const uint32_t timeoutMillis = 15 * 1000;
        const uint32_t startedMillis = millis();
        std::string rxd;

        Status ret = mCatM1.Execute(command, mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO PUBLISH: %s", ret.c_str());
            return ret;
        }

        bool hasReadyToSendSignal = false;
        while (uint32_t(millis() - startedMillis) < timeoutMillis)
        {
            while (mCatM1.GetAvailableBytes() > 0)
            {
                if (mCatM1.Read() == static_cast<int16_t>('>'))
                {
                    hasReadyToSendSignal = true;
                    goto HAS_RTS_SIGNAL;
                }
            }
        }
        if (hasReadyToSendSignal == false)
        {
            LOG_ERROR(logger, "FAILED TO PUBLISH DUE TO NO RTS"); // ready to send
            return Status(Status::Code::BAD_TIMEOUT);
        }

HAS_RTS_SIGNAL:
        ret = mCatM1.Execute(message.GetPayload(), mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO PUBLISH: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        //LOG_DEBUG(logger, "RxD: %s", rxd.c_str());
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO PUBLISH: %s: %s", ret.c_str(),
                processCmeErrorCode(rxd).c_str());
            return ret;
        }

        const std::string patternBegin = "\r\n+QMTPUB: ";
        const std::string patternEnd   = "\r\n";
        
        if (rxd.find(patternBegin) == std::string::npos)
        {
            rxd.clear();
        }
        else if (rxd.find(patternBegin) != std::string::npos && 
                 rxd.find(patternEnd)   == std::string::npos)
        {
            const size_t startPos = rxd.find(patternBegin);
            rxd.erase(0, startPos);
        }
        else if (rxd.find(patternBegin) != std::string::npos && 
                 rxd.find(patternEnd)   != std::string::npos)
        {
            const size_t startPos = rxd.find(patternBegin);
            rxd.erase(0, startPos);
            //LOG_DEBUG(logger, "goto the label \"PATTERN_FOUND\"");
            goto PATTERN_FOUND;
        }

        while (millis() - startedMillis < timeoutMillis)
        {
            const std::string betweenPatterns = mCatM1.ReadBetweenPatterns(patternBegin, patternEnd);
            if (betweenPatterns.length() == 0)
            {
                vTaskDelay(50 / portTICK_PERIOD_MS);
                continue;
            }
            //LOG_DEBUG(logger, "RxD: %s", rxd.c_str());
            rxd.append(betweenPatterns);
            //LOG_DEBUG(logger, "RxD: %s", rxd.c_str());
            break;
        }

PATTERN_FOUND:
        //LOG_DEBUG(logger, "RxD: %s", rxd.c_str());
        std::vector<size_t> vecDelimiter;
        vecDelimiter.emplace_back(rxd.find(' '));
        if (vecDelimiter.front() == std::string::npos)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        size_t position = std::string::npos;
        do
        {
            position = rxd.find(',', vecDelimiter.back() + 1);
            if (position != std::string::npos && position != rxd.length())
            {
                vecDelimiter.emplace_back(position);
            }
        } while (position != std::string::npos);
        vecDelimiter.emplace_back(rxd.find('\r', vecDelimiter.back() + 1));

        constexpr uint8_t MINIMUM_DELIMITER_COUNT = 4;
        constexpr uint8_t MAXIMUM_DELIMITER_COUNT = 5;
        if (vecDelimiter.size() != MINIMUM_DELIMITER_COUNT && MAXIMUM_DELIMITER_COUNT != vecDelimiter.size())
        {
            LOG_ERROR(logger, "INVALID NUMBER OF DELIMITER: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        std::vector<size_t>::iterator it = vecDelimiter.begin();
        const std::string strSocketID  = rxd.substr(*it + 1, *(it + 1) - *it - 1);
        const std::string strMessageID = rxd.substr(*(++it) + 1, *(it + 1) - *(it) - 1);
        const std::string strResult    = rxd.substr(*(++it) + 1, *(it + 1) - *(it) - 1);
        const std::string strRetrans   = (it + 1) == vecDelimiter.end() ?
            "" : rxd.substr(*(++it) + 1, *(it + 1) - *(it) - 1);
        const int32_t rxdSocketID  = Convert.ToInt32(strSocketID.c_str());
        const int32_t rxdMessageID = Convert.ToInt32(strMessageID.c_str());
        const int32_t rxdResult    = Convert.ToInt32(strResult.c_str());
        const int32_t rxdRetrans   = strRetrans == "" ? 0 : Convert.ToInt32(strResult.c_str());

        if (rxdSocketID == INT32_MAX || rxdMessageID == INT32_MAX || rxdResult == INT32_MAX || rxdRetrans == INT32_MAX)
        {
            //LOG_DEBUG(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        else if (rxdSocketID != msgSocketID || rxdMessageID != msgMessageID)
        {
            //LOG_DEBUG(logger, "INVALID SOCKET OR MESSAGE ID: %u, %u", rxdSocketID, rxdMessageID);
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        else
        {
            switch (rxdResult)
            {
            case 0:
                LOG_INFO(logger, "Packet sent successfully and received ACK from server: %u", rxdSocketID);
                return Status(Status::Code::GOOD);
            case 1:
                LOG_WARNING(logger, "RETRANSMITTING PUBLISH PACKETS: \"SocketID\": %u, \"Retrans\": %u", rxdSocketID, rxdRetrans);
                return Status(Status::Code::UNCERTAIN);
            case 2:
                LOG_ERROR(logger, "FAILED TO SEND PUBLISH PACKETS: %u", rxdSocketID);
                return Status(Status::Code::BAD_REQUEST_NOT_COMPLETE);
            default:
                LOG_ERROR(logger, "UNDEFINED RESULT: %u", rxdResult);
                return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
            }
        }
    }

    Status CatMQTT::setPdpContext(const size_t mutexHandle, const network::lte::pdp_ctx_e pdp)
    {
        char command[32];
        memset(command, '\0', sizeof(command));

        const uint8_t socketID  = static_cast<uint8_t>(mBrokerInfo.GetSocketID());
        const uint8_t contextID = static_cast<uint8_t>(pdp);

        sprintf(command, "AT+QMTCFG=\"pdpcid\",%u,%u", socketID, contextID);
        const uint32_t timeoutMillis = 300;
        std::string rxd;

        Status ret = mCatM1.Execute(command, mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET PDP CONTEXT: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret == Status::Code::GOOD)
        {
            mContextPDP = pdp;
            LOG_INFO(logger, "PDP context: %s", ConvertPdpToString(mContextPDP));
            return ret;
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO SET PDP CONTEXT: %s", ret.c_str());
            return ret;
        }
    }

    Status CatMQTT::setSslContext(const size_t mutexHandle, const network::lte::ssl_ctx_e ssl)
    {
        char command[32];
        memset(command, '\0', sizeof(command));

        const uint8_t socketID  = static_cast<uint8_t>(mBrokerInfo.GetSocketID());
        const bool enableSSL = mBrokerInfo.IsSslEnabled();
        if (enableSSL == false)
        {
            sprintf(command, "AT+QMTCFG=\"ssl\",%u,%u", socketID, enableSSL);
        }
        else
        {
            const uint8_t contextID = static_cast<uint8_t>(ssl);
            sprintf(command, "AT+QMTCFG=\"ssl\",%u,%u,%u", socketID, enableSSL, contextID);
        }
        
        const uint32_t timeoutMillis = 300;
        std::string rxd;

        Status ret = mCatM1.Execute(command, mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET SSL CONTEXT: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret == Status::Code::GOOD)
        {
            mContextSSL = ssl;
            LOG_INFO(logger, "SSL context: %s", ConvertSslToString(mContextSSL));
            return ret;
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO SET SSL CONTEXT: %s", ret.c_str());
            return ret;
        }
    }

    Status CatMQTT::setVersion(const size_t mutexHandle)
    {
        char command[32];
        memset(command, '\0', sizeof(command));

        const uint8_t socketID  = static_cast<uint8_t>(mBrokerInfo.GetSocketID());
        const uint8_t version   = static_cast<uint8_t>(mBrokerInfo.GetVersion());

        sprintf(command, "AT+QMTCFG=\"version\",%u,%u", socketID, version);
        const uint32_t timeoutMillis = 300;
        std::string rxd;

        Status ret = mCatM1.Execute(command, mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET VERSION: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret == Status::Code::GOOD)
        {
            LOG_INFO(logger, "MQTT version: %s", ConvertVersionToString(mBrokerInfo.GetVersion()));
            return ret;
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO SET VERSION: %s", ret.c_str());
            return ret;
        }
    }

    Status CatMQTT::setLastWill(const size_t mutexHandle)
    {
        constexpr uint8_t COMMAND_BUFFER_SIZE = UINT8_MAX;
        char command[COMMAND_BUFFER_SIZE];
        memset(command, '\0', sizeof(command));

        const uint8_t socketID    = static_cast<uint8_t>(mBrokerInfo.GetSocketID());
        // const uint8_t enableFlag  = static_cast<uint8_t>(mInitFlags.test(init_flag_e::ENABLE_LWT_MSG));
        const uint8_t qosLevel    = static_cast<uint8_t>(mMessageLWT.GetQoS());
        const uint8_t retainFlag  = static_cast<uint8_t>(mMessageLWT.IsRetain());

        LOG_WARNING(logger, "ENABLE_LWT_MSG: %s", mInitFlags.test(init_flag_e::ENABLE_LWT_MSG) ? "true" : "false");
        // if (mInitFlags.test(init_flag_e::ENABLE_LWT_MSG) == true)
        // {
            sprintf(command, "AT+QMTCFG=\"will\",%u,%u,%u,%u,\"%s\",\"%s\"", 
                // socketID, enableFlag, qosLevel, retainFlag,
                socketID, 1, qosLevel, retainFlag,
                mMessageLWT.GetTopicString(), mMessageLWT.GetPayload()
            );
            
            ASSERT(
                (strlen(command) < (COMMAND_BUFFER_SIZE - 1)), 
                "COMMAND SIZE CANNOT EXCEED %u", COMMAND_BUFFER_SIZE
            );
        // }
        // else
        // {
        //     sprintf(command, "AT+QMTCFG=\"will\",%u,%u", socketID, enableFlag);
        // }
        
        const uint32_t timeoutMillis = 300;
        std::string rxd;

        Status ret = mCatM1.Execute(command, mutexHandle);
        LOG_WARNING(logger, "LWT: %s", command);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET LWT: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret == Status::Code::GOOD)
        {
            LOG_INFO(logger, "LWT message: Set");
            mInitFlags.set(init_flag_e::INITIALIZED_LWT);
            return ret;
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO SET LWT: %s", ret.c_str());
            return ret;
        }
    }

    Status CatMQTT::setKeepAlive(const size_t mutexHandle)
    {
        char command[32];
        memset(command, '\0', sizeof(command));

        const uint8_t socketID   = static_cast<uint8_t>(mBrokerInfo.GetSocketID());
        const uint16_t keepAlive = static_cast<uint16_t>(mBrokerInfo.GetKeepAlive());

        sprintf(command, "AT+QMTCFG=\"keepalive\",%u,%u", socketID, keepAlive);
        const std::string expected = "OK";
        const uint32_t timeoutMillis = 300;
        std::string rxd;

        Status ret = mCatM1.Execute(command, mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET KEEPALIVE: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret == Status::Code::GOOD)
        {
            LOG_INFO(logger, "Keep alive: %u", keepAlive);
            return ret;
        }
        else
        {
            LOG_ERROR(logger, "FAILED TO SET KEEPALIVE: %s", ret.c_str());
            return ret;
        }
    }

/*    Status CatMQTT::checkPdpContext()
    {
        ASSERT(false, "THE FUNCTION IS NOT IMPLEMENTED!");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }*/

/*    Status CatMQTT::checkSslContext()
    {
        ASSERT(false, "THE FUNCTION IS NOT IMPLEMENTED!");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }*/

    Status CatMQTT::checkVersion(const size_t mutexHandle)
    {
        const std::string command = "AT+QMTCFG=\"version\"," + std::to_string(static_cast<uint8_t>(mBrokerInfo.GetSocketID()));
        const std::string expected = "OK";
        const uint32_t timeoutMillis = 300;
        std::string rxd;

        Status ret = mCatM1.Execute(command, mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CHECK PROTOCOL VERSION: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "MODEM REPORTED ERROR: %s", ret.c_str());
            return ret;
        }

        const size_t delimiter = rxd.find(",", rxd.find("+QMTCFG:") + 1);
        if (delimiter == std::string::npos)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: \n%s\n\n", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        const char* strVersion = rxd.substr(delimiter + 1, 1).c_str();
        const uint32_t intVersion = Convert.ToUInt32(strVersion);
        version_e convertedVersion = ConvertUInt32ToVersion(intVersion);
        if (convertedVersion == version_e::UNDEFINED)
        {
            LOG_ERROR(logger, "RECEIVED VERSION STRING: \n%s\n\n", strVersion);
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        if (convertedVersion == mBrokerInfo.GetVersion())
        {
            LOG_INFO(logger, "Check result: OK");
            return Status(Status::Code::GOOD);
        }
        else
        {
            LOG_ERROR(logger, "BROKER REQUIRES: %s,  MODEM CONFIG: %s", 
                ConvertVersionToString(mBrokerInfo.GetVersion()), 
                ConvertVersionToString(convertedVersion));
            return Status(Status::Code::BAD);
        }
    }

/*    Status CatMQTT::checkLastWill(const size_t mutexHandle)
    {
        if (strlen(mMessageLWT.GetPayload()) == 0)
        {
            return Status(Status::Code::GOOD);
        }
        
        char command[32];
        memset(command, '\0', sizeof(command));

        const uint8_t socketID = static_cast<uint8_t>(mBrokerInfo.GetSocketID());
        sprintf(command, "AT+QMTCFG=\"will\",%u", socketID);
        const uint32_t timeoutMillis = 300;
        std::string rxd;

        Status ret = mCatM1.Execute(command, mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CHECK PROTOCOL VERSION: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "MODEM REPORTED ERROR: %s", ret.c_str());
            return ret;
        }

        const size_t delimiter1 = rxd.find(",", rxd.find("+QMTCFG: \"will\""));
        const size_t delimiter2 = rxd.find(",", delimiter1 + 1);
        const size_t delimiter3 = rxd.find(",", delimiter2 + 1);
        const size_t delimiter4 = rxd.find(",", delimiter3 + 1);
        const size_t delimiter5 = rxd.find(",", delimiter4 + 1);
        const size_t delimiter6 = rxd.find_last_of("\"");

        if (delimiter1 == std::string::npos || delimiter2 == std::string::npos ||
            delimiter3 == std::string::npos || delimiter4 == std::string::npos ||
            delimiter5 == std::string::npos || delimiter6 == std::string::npos)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s\n\n", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        const std::string strEnableFlag = rxd.substr(delimiter1 + 1, delimiter2 - delimiter1 - 1);
        if (static_cast<uint8_t>(mInitFlags.test(ENABLE_LWT_MSG)) != Convert.ToUInt32(strEnableFlag.c_str()))
        {
            LOG_ERROR(logger, "[ENABLE FLAG] CONFIG: %s  QUERY: %s", mInitFlags.test(ENABLE_LWT_MSG) ? "true" : "false", strEnableFlag.c_str());
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        const std::string strQoS = rxd.substr(delimiter2 + 1, delimiter3 - delimiter2 - 1);
        if (static_cast<uint8_t>(mMessageLWT.GetQoS()) != Convert.ToUInt32(strQoS.c_str()))
        {
            LOG_ERROR(logger, "[QoS LEVEL] CONFIG: %u  QUERY: %s", static_cast<uint8_t>(mMessageLWT.GetQoS()), strQoS.c_str());
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        const std::string strRetainFlag = rxd.substr(delimiter3 + 1, delimiter4 - delimiter3 - 1);
        if (static_cast<uint8_t>(mMessageLWT.IsRetain()) != Convert.ToUInt32(strRetainFlag.c_str()))
        {
            LOG_ERROR(logger, "[RETAIN FLAG] CONFIG: %s  QUERY: %s", mMessageLWT.IsRetain() ? "true" : "false", strRetainFlag.c_str());
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }
        
        const std::string strTopic = rxd.substr(delimiter4 + 2, delimiter5 - delimiter4 - 3);
        if (strcmp(mMessageLWT.GetTopicString(), strTopic.c_str()) != 0)
        {
            LOG_ERROR(logger, "[TOPIC] CONFIG: %s  QUERY: %s", mMessageLWT.GetTopicString(), strTopic.c_str());
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        const std::string strPayload = rxd.substr(delimiter5 + 2, delimiter6 - delimiter5 - 2);
        if (strcmp(mMessageLWT.GetPayload(), strPayload.c_str()) != 0)
        {
            LOG_ERROR(logger, "[PAYLOAD] CONFIG: %s  QUERY: %s", mMessageLWT.GetPayload(), strPayload.c_str());
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        LOG_INFO(logger, "Check result: OK");
        return Status(Status::Code::GOOD);
    }*/ 

/*    Status CatMQTT::checkKeepAlive()
    {
        ASSERT(false, "THE FUNCTION IS NOT IMPLEMENTED!");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }*/

    Status CatMQTT::openSession(const size_t mutexHandle)
    {
        const uint8_t brokerSocketID = static_cast<uint8_t>(mBrokerInfo.GetSocketID());

        const std::string command = "AT+QMTOPEN="
            + std::to_string(brokerSocketID) + ",\""
            + mBrokerInfo.GetHost()          + "\","
            + std::to_string(mBrokerInfo.GetPort());
        const uint32_t timeoutMillis = 75 * 1000;
        const uint32_t startedMillis = millis();
        std::string rxd;

        Status ret = mCatM1.Execute(command, mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO OPEN SESSION: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO OPEN SESSION: %s: %s", ret.c_str(), processCmeErrorCode(rxd).c_str());
            return ret;
        }

        const std::string patternBegin = "\r\n+QMTOPEN: ";
        const std::string patternEnd   = "\r\n";
        rxd.clear();

        while (millis() - startedMillis < timeoutMillis)
        {
            const std::string betweenPatterns = mCatM1.ReadBetweenPatterns(patternBegin, patternEnd);
            if (betweenPatterns.length() == 0)
            {
                vTaskDelay(50 / portTICK_PERIOD_MS);
                continue;
            }
            rxd.append(betweenPatterns);
            break;
        }

        const size_t delimiter1 = rxd.find(' ');
        const size_t delimiter2 = rxd.find(',',  delimiter1);
        const size_t delimiter3 = rxd.find('\r', delimiter2);
        if (delimiter1 == std::string::npos || delimiter2 == std::string::npos || delimiter3 == std::string::npos)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        const std::string strSocketID = rxd.substr(delimiter1 + 1, delimiter2 - delimiter1 - 1);
        const std::string strResult   = rxd.substr(delimiter2 + 1, delimiter3 - delimiter2 - 1);
        const int32_t rxdSocketID = Convert.ToInt32(strSocketID.c_str());
        const int32_t rxdResult   = Convert.ToInt32(strResult.c_str());
        if (rxdSocketID == INT32_MAX || rxdResult == INT32_MAX)
        {
            //LOG_DEBUG(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        else if (rxdSocketID != brokerSocketID)
        {
            //LOG_DEBUG(logger, "INVALID SOCKET ID: %u", rxdSocketID);
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        else
        {
            switch (rxdResult)
            {
            case -1:
                LOG_ERROR(logger, "FAILED TO OPEN A NETWORK FOR A NEW SESSION");
                return Status(Status::Code::BAD_SESSION_NOT_ACTIVATED);
            case 0:
                LOG_INFO(logger, "Opened a session with socket ID: %u", rxdSocketID);
                return Status(Status::Code::GOOD);
            case 1:
                LOG_ERROR(logger, "BROKER CONNECTION PARAMETER IS NOT VALID");
                return Status(Status::Code::BAD_INVALID_ARGUMENT);
            case 2:
                LOG_ERROR(logger, "ALREADY OPENED WITH SOCKET ID: %u", rxdSocketID);
                return Status(Status::Code::BAD_RESOURCE_UNAVAILABLE);
            case 3:
                LOG_ERROR(logger, "FAILED TO ACTIVATE PDP CONTEXT FOR A SESSION");
                return Status(Status::Code::BAD_SESSION_NOT_ACTIVATED);
            case 4:
                LOG_ERROR(logger, "FAILED TO PARSE DOMAIN TO OPEN A SESSION");
                return Status(Status::Code::BAD_SERVER_URI_INVALID);
            case 5:
                LOG_ERROR(logger, "FAILED TO OPEN DUE TO NETWORK DISCONNECTION");
                return Status(Status::Code::BAD_DISCONNECT);
            default:
                LOG_ERROR(logger, "UNDEFINED RESULT: %u", rxdResult);
                return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
            }
        }
    }

    Status CatMQTT::connectBroker(const size_t mutexHandle)
    {
        const uint8_t brokerSocketID = static_cast<uint8_t>(mBrokerInfo.GetSocketID());
        
        const std::string command = "AT+QMTCONN="
            + std::to_string(brokerSocketID) + ",\""
            + mBrokerInfo.GetClientID()      + "\",\""
            + mBrokerInfo.GetUsername()      + "\",\""
            + mBrokerInfo.GetPassword()      + "\"";
        const uint32_t timeoutMillis = 60 * 1000;
        const uint32_t startedMillis = millis();
        std::string rxd;

        Status ret = mCatM1.Execute(command, mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT TO BROKER: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT TO BROKER: %s: %s", ret.c_str(), processCmeErrorCode(rxd).c_str());
            return ret;
        }

        const std::string patternBegin = "\r\n+QMTCONN: ";
        const std::string patternEnd   = "\r\n";
        rxd.clear();

        while (millis() - startedMillis < timeoutMillis)
        {
            const std::string betweenPatterns = mCatM1.ReadBetweenPatterns(patternBegin, patternEnd);
            if (betweenPatterns.length() == 0)
            {
                vTaskDelay(50 / portTICK_PERIOD_MS);
                continue;
            }
            rxd.append(betweenPatterns);
            break;
        }

        const size_t delimiter1 = rxd.find(' ');
        const size_t delimiter2 = rxd.find(',',  delimiter1 + 1);
        const size_t delimiter3 = rxd.find(',',  delimiter2 + 1);
        const size_t delimiter4 = delimiter3 == std::string::npos ?
            rxd.find('\r', delimiter2 + 1) : 
            rxd.find('\r', delimiter3 + 1);

        if (delimiter1 == std::string::npos || delimiter2 == std::string::npos || delimiter4 == std::string::npos)
        {
            LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        const std::string strSocketID = rxd.substr(delimiter1 + 1, delimiter2 - delimiter1 - 1);
        const std::string strResult   = delimiter3 == std::string::npos ?
            rxd.substr(delimiter2 + 1, delimiter4 - delimiter2 - 1) :
            rxd.substr(delimiter2 + 1, delimiter3 - delimiter2 - 1);
        const std::string strReturn   = delimiter3 == std::string::npos ?
            "" : rxd.substr(delimiter3 + 1, delimiter4 - delimiter3 - 1);
        
        const int32_t rxdSocketID = Convert.ToInt32(strSocketID.c_str());
        const int32_t rxdResult   = Convert.ToInt32(strResult.c_str());
        const int32_t rxdReturn   = strReturn == "" ? -1 : Convert.ToInt32(strResult.c_str());

        if (rxdSocketID == INT32_MAX || rxdResult == INT32_MAX)
        {
            //LOG_DEBUG(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        else if (rxdSocketID != brokerSocketID)
        {
            //LOG_DEBUG(logger, "INVALID SOCKET ID: %u", rxdSocketID);
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
        else
        {
            if (rxdReturn != -1)
            {
                switch (rxdReturn)
                {
                case 0:
                    LOG_INFO(logger, "Connection accepted: %u", rxdSocketID);
                    break;
                case 1:
                    LOG_ERROR(logger, "CONNECTION REFUSED: UNACCEPTABLE PROTOCOL VERSION: %u", rxdSocketID);
                    break;
                case 2:
                    LOG_ERROR(logger, "CONNECTION REFUSED: IDENTIFIER REJECTED: %u", rxdSocketID);
                    break;
                case 3:
                    LOG_ERROR(logger, "CONNECTION REFUSED: BROKER UNAVAILABLE: %u", rxdSocketID);
                    break;
                case 4:
                    LOG_ERROR(logger, "CONNECTION REFUSED: BAD USER NAME OR PASSWORD: %u", rxdSocketID);
                    break;
                case 5:
                    LOG_ERROR(logger, "CONNECTION REFUSED: NOT AUTHORIZED: %u", rxdSocketID);
                    break;
                default:
                    LOG_ERROR(logger, "PROCESSOR ERROR: UNDEFINED RESULT: %d", rxdReturn);
                    break;
                }
            }

            switch (rxdResult)
            {
            case 0:
                LOG_INFO(logger, "Packet sent successfully and received ACK from server: %u", rxdSocketID);
                return Status(Status::Code::GOOD);
            case 1:
                LOG_ERROR(logger, "RETRANSMITTING CONN-REQ PACKETS: %u", rxdSocketID);
                return Status(Status::Code::UNCERTAIN);
            case 2:
                LOG_ERROR(logger, "FAILED TO SEND CONN-REQ: %u", rxdSocketID);
                return Status(Status::Code::BAD_REQUEST_NOT_COMPLETE);
            default:
                LOG_ERROR(logger, "UNDEFINED RESULT: %u", rxdResult);
                return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
            }
        }
    }

    Status CatMQTT::disconnectBroker(const size_t mutexHandle)
    {
        const uint8_t brokerSocketID = static_cast<uint8_t>(mBrokerInfo.GetSocketID());
        
        const std::string command = "AT+QMTDISC=" + std::to_string(brokerSocketID);
        const uint32_t timeoutMillis = 500;
        // const uint32_t startedMillis = millis();
        std::string rxd;

        Status ret = mCatM1.Execute(command, mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DISCONNECT: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DISCONNECT: %s: %s", ret.c_str(), processCmeErrorCode(rxd).c_str());
            return ret;
        }
        return ret;
        
        // const std::string patternBegin = "\r\n+QMTDISC: ";
        // const std::string patternEnd   = "\r\n";
        // LOG_DEBUG(logger, "RxD: %s", rxd.c_str());
        // rxd.clear();
        
        // while (millis() - startedMillis < timeoutMillis)
        // {
        //     const std::string betweenPatterns = mCatM1.ReadBetweenPatterns(patternBegin, patternEnd);
        //     if (betweenPatterns.length() == 0)
        //     {
        //         vTaskDelay(50 / portTICK_PERIOD_MS);
        //         continue;
        //     }
        //     rxd.append(betweenPatterns);
        //     break;
        // }
        // LOG_DEBUG(logger, "RxD: %s", rxd.c_str());

        // const size_t delimiter1 = rxd.find(' ');
        // const size_t delimiter2 = rxd.find(',',  delimiter1 + 1);
        // if (delimiter1 == std::string::npos || delimiter2 == std::string::npos)
        // {
        //     LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
        //     return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        // }

        // const std::string strSocketID = rxd.substr(delimiter1 + 1, delimiter2 - delimiter1 - 1);
        // const std::string strResult   = rxd.substr(delimiter2 + 1);
        // const int32_t rxdSocketID = Convert.ToInt32(strSocketID.c_str());
        // const int32_t rxdResult   = Convert.ToInt32(strResult.c_str());
        // LOG_DEBUG(logger, "Socket ID: %d", rxdSocketID);
        // LOG_DEBUG(logger, "Result: %d", rxdResult);

        // if (rxdResult == 0)
        // {
        //     return Status(Status::Code::GOOD);
        // }
        // else
        // {
        //     return Status(Status::Code::BAD);
        // }
    }

    Status CatMQTT::closeSession(const size_t mutexHandle)
    {
        const uint8_t brokerSocketID = static_cast<uint8_t>(mBrokerInfo.GetSocketID());
        
        const std::string command = "AT+QMTCLOSE=" + std::to_string(brokerSocketID);
        const uint32_t timeoutMillis = 300;
        // const uint32_t startedMillis = millis();
        std::string rxd;

        Status ret = mCatM1.Execute(command, mutexHandle);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CLOSE: %s", ret.c_str());
            return ret;
        }

        ret = readUntilOKorERROR(timeoutMillis, &rxd);
        LOG_DEBUG(logger, "RxD: %s", rxd.c_str());
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CLOSE: %s: %s", ret.c_str(), processCmeErrorCode(rxd).c_str());
            return ret;
        }
        return ret;
        
        // const std::string patternBegin = "\r\n+QMTCLOSE: ";
        // const std::string patternEnd   = "\r\n";
        // rxd.clear();
        
        // while (millis() - startedMillis < timeoutMillis)
        // {
        //     const std::string betweenPatterns = mCatM1.ReadBetweenPatterns(patternBegin, patternEnd);
        //     if (betweenPatterns.length() == 0)
        //     {
        //         vTaskDelay(50 / portTICK_PERIOD_MS);
        //         continue;
        //     }
        //     rxd.append(betweenPatterns);
        //     break;
        // }

        // const size_t delimiter1 = rxd.find(' ');
        // const size_t delimiter2 = rxd.find(',',  delimiter1 + 1);
        // if (delimiter1 == std::string::npos || delimiter2 == std::string::npos)
        // {
        //     LOG_ERROR(logger, "UNKNOWN RESPONSE: %s", rxd.c_str());
        //     return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        // }

        // const std::string strSocketID = rxd.substr(delimiter1 + 1, delimiter2 - delimiter1 - 1);
        // const std::string strResult   = rxd.substr(delimiter2 + 1);
        // const int32_t rxdSocketID = Convert.ToInt32(strSocketID.c_str());
        // const int32_t rxdResult   = Convert.ToInt32(strResult.c_str());
        // LOG_DEBUG(logger, "Socket ID: %d", rxdSocketID);
        // LOG_DEBUG(logger, "Result: %d", rxdResult);

        // if (rxdResult == 0)
        // {
        //     return Status(Status::Code::GOOD);
        // }
        // else
        // {
        //     return Status(Status::Code::BAD);
        // }
    }

/*    void CatMQTT::onEventQMTSTAT(const uint8_t socketID, const uint8_t errorCode)
    {
        ASSERT(false, "THE FUNCTION IS NOT IMPLEMENTED!");

        LOG_WARNING(logger, "DISCONNECTED FROM THE BROKER DUE TO CHANGE IN LINK LAYER");
        mState = state_e::DISCONNECTED;

        return ;
    }*/

    Status CatMQTT::readUntilOKorERROR(const uint32_t timeoutMillis, std::string* rxd)
    {
        ASSERT((rxd != nullptr), "OUTPUT PARAMETER MUST NOT BE A NULLPTR");
        
        const uint32_t startMillis = millis();

        while (uint32_t(millis() - startMillis) < timeoutMillis)
        {
            while (mCatM1.GetAvailableBytes() > 0)
            {
                int16_t value = mCatM1.Read();
                if (value == -1)
                {
                    LOG_WARNING(logger, "FAILED TO TAKE MUTEX OR NO DATA AVAILABLE");
                    continue;
                }
                *rxd += value;
            }

            if (rxd->find("OK") != std::string::npos)
            {
                return Status(Status::Code::GOOD);
            }
            else if (rxd->find("ERROR") != std::string::npos)
            {
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }
            else
            {
                continue;
            }
        }

        return Status(Status::Code::BAD_TIMEOUT);
    }

    Status CatMQTT::processCmeErrorCode(const std::string& rxd)
    {
        const std::string cmeErrorIndicator = "+CME ERROR: ";
        const size_t cmeStartPosition  = rxd.find(cmeErrorIndicator) + cmeErrorIndicator.length();
        const size_t cmeFinishPosition = rxd.find("\r", cmeStartPosition);
        if (cmeStartPosition == std::string::npos || cmeFinishPosition == std::string::npos)
        {
            //LOG_DEBUG(logger, "INVALID CME ERROR CODE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }

        const size_t cmeErrorCodeLength = cmeFinishPosition - cmeStartPosition;
        const std::string cmeErrorCodeString = rxd.substr(cmeStartPosition, cmeErrorCodeLength);
        const uint32_t cmeErrorCode = Convert.ToUInt32(cmeErrorCodeString.c_str());
        switch (cmeErrorCode)
        {
        case 0:
            LOG_ERROR(logger, "UE FAILURE");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        case 1:
            LOG_ERROR(logger, "NO CONNECTION TO UE");
            return Status(Status::Code::BAD_NOT_CONNECTED);
        case 2:
            LOG_ERROR(logger, "UE ADAPTER LINK RESERVED");
            return Status(Status::Code::BAD_RESOURCE_UNAVAILABLE);
        case 3:
            LOG_ERROR(logger, "OPERATION NOT ALLOWED");
            return Status(Status::Code::BAD_REQUEST_NOT_ALLOWED);
        case 4:
            LOG_ERROR(logger, "OPERATION NOT SUPPORTED");
            return Status(Status::Code::BAD_NOT_SUPPORTED);
        case 5:
            LOG_ERROR(logger, "PH-SIM PIN REQUIRED");
            return Status(Status::Code::BAD_TICKET_REQUIRED);
        case 6:
            LOG_ERROR(logger, "PH-FSIM PIN REQUIRED");
            return Status(Status::Code::BAD_TICKET_REQUIRED);
        case 7:
            LOG_ERROR(logger, "PH FSIM PUK REQUIRED");
            return Status(Status::Code::BAD_TICKET_REQUIRED);
        case 10:
            LOG_ERROR(logger, "(U)SIM NOT INSERTED");
            return Status(Status::Code::BAD_TICKET_REQUIRED);
        case 11:
            LOG_ERROR(logger, "(U)SIM PIN REQUIRED");
            return Status(Status::Code::BAD_TICKET_REQUIRED);
        case 12:
            LOG_ERROR(logger, "(U)SIM PUK REQUIRED");
            return Status(Status::Code::BAD_TICKET_REQUIRED);
        case 13:
            LOG_ERROR(logger, "(U)SIM FAILURE");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        case 14:
            LOG_ERROR(logger, "(U)SIM BUSY");
            return Status(Status::Code::BAD_RESOURCE_UNAVAILABLE);
        case 15:
            LOG_ERROR(logger, "(U)SIM WRONG");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        case 16:
            LOG_ERROR(logger, "INCORRECT PASSWORD");
            return Status(Status::Code::BAD_TICKET_INVALID);
        case 17:
            LOG_ERROR(logger, "SIM PIN2 REQUIRED");
            return Status(Status::Code::BAD_TICKET_REQUIRED);
        case 18:
            LOG_ERROR(logger, "SIM PUK2 REQUIRED");
            return Status(Status::Code::BAD_TICKET_REQUIRED);
        case 20:
            LOG_ERROR(logger, "MEMORY FULL");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        case 21:
            LOG_ERROR(logger, "INVALID INDEX");
            return Status(Status::Code::BAD_INDEX_RANGE_INVALID);
        case 22:
            LOG_ERROR(logger, "NOT FOUND");
            return Status(Status::Code::BAD_NOT_FOUND);
        case 23:
            LOG_ERROR(logger, "MEMORY FAILURE");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        case 24:
            LOG_ERROR(logger, "TEXT STRING TOO LONG");
            return Status(Status::Code::BAD_REQUEST_TOO_LARGE);
        case 25:
            LOG_ERROR(logger, "INVALID CHARACTERS IN TEXT STRING");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        case 26:
            LOG_ERROR(logger, "DIAL STRING TOO LONG");
            return Status(Status::Code::BAD_REQUEST_TOO_LARGE);
        case 27:
            LOG_ERROR(logger, "INVALID CHARACTERS IN DIAL STRING");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        case 30:
            LOG_ERROR(logger, "NO NETWORK SERVICE");
            return Status(Status::Code::BAD_NO_COMMUNICATION);
        case 31:
            LOG_ERROR(logger, "NETWORK TIMEOUT");
            return Status(Status::Code::BAD_TIMEOUT);
        case 32:
            LOG_ERROR(logger, "NETWORK NOT ALLOWED - EMERGENCY CALLS ONLY");
            return Status(Status::Code::BAD_NO_COMMUNICATION);
        case 40:
            LOG_ERROR(logger, "NETWORK PERSONALIZATION PIN REQUIRED");
            return Status(Status::Code::BAD_TICKET_REQUIRED);
        case 41:
            LOG_ERROR(logger, "NETWORK PERSONALIZATION PUK REQUIRED");
            return Status(Status::Code::BAD_TICKET_REQUIRED);
        case 42:
            LOG_ERROR(logger, "NETWORK SUBNET PERSONALIZATION PIN REQUIRED");
            return Status(Status::Code::BAD_TICKET_REQUIRED);
        case 43:
            LOG_ERROR(logger, "NETWORK SUBNET PERSONALIZATION PUK REQUIRED");
            return Status(Status::Code::BAD_TICKET_REQUIRED);
        case 44:
            LOG_ERROR(logger, "SERVICE PROVIDER PERSONALIZATION PIN REQUIRED");
            return Status(Status::Code::BAD_TICKET_REQUIRED);
        case 45:
            LOG_ERROR(logger, "SERVICE PROVIDER PERSONALIZATION PUK REQUIRED");
            return Status(Status::Code::BAD_TICKET_REQUIRED);
        case 46:
            LOG_ERROR(logger, "CORPORATE PERSONALIZATION PIN REQUIRED");
            return Status(Status::Code::BAD_TICKET_REQUIRED);
        case 47:
            LOG_ERROR(logger, "CORPORATE PERSONALIZATION PUK REQUIRED");
            return Status(Status::Code::BAD_TICKET_REQUIRED);
        default:
            //LOG_DEBUG(logger, "INVALID CME ERROR CODE: %s", rxd.c_str());
            return Status(Status::Code::BAD_UNKNOWN_RESPONSE);
        }
    }


    CatMQTT* CatMQTT::mInstance = nullptr;
}}