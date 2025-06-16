/**
 * @file CatM1.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief LTE Cat.M1 통신을 사용하는데 필요한 기능을 제공하는 클래스를 선언합니다.
 * 
 * @date 2025-01-23
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <esp32-hal-gpio.h>
#include <iomanip>
#include <pins_arduino.h>
#include <sstream>

#include "CatM1.h"
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"



namespace muffin {

    CatM1::state_e CatM1::mState = CatM1::state_e::NOT_INITIALIZED_YET;
    std::bitset<8> CatM1::mInitFlags;
    std::bitset<6> CatM1::mConnFlags;
    uint32_t CatM1::mLastInterruptMillis = 0;


    CatM1::CatM1()
        : xSemaphore(NULL)
        , mConfig(std::make_pair(false, jvs::config::CatM1()))
    {
        mInitFlags.reset();
        mConnFlags.reset();
    }

    Status CatM1::Init()
    {
        xSemaphore = xSemaphoreCreateMutex();
        if (xSemaphore == NULL)
        {
            LOG_ERROR(logger, "FAILED TO CREATE SEMAPHORE");
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }

        if (mState == state_e::SUCCEDDED_TO_INITIALIZE)
        {
            LOG_WARNING(logger, "REINITIALIZATION IS NOT ALLOWED");
            return Status(Status::Code::GOOD);
        }

        if (mInitFlags.test(init_flags_e::URC_CALLBACK) == false)
        {
            mProcessor.RegisterCallbackRDY(std::bind(&CatM1::onEventRDY, this));
            mProcessor.RegisterCallbackCFUN(std::bind(&CatM1::onEventCFUN, this));
            mProcessor.RegisterCallbackCPIN(std::bind(&CatM1::onEventCPIN, this, std::placeholders::_1));
            mProcessor.RegisterCallbackQIND(std::bind(&CatM1::onEventQIND, this));
            mProcessor.RegisterCallbackAPPRDY(std::bind(&CatM1::onEventAPPRDY, this));
            mProcessor.RegisterCallbackQMTSTAT(std::bind(&CatM1::onEventQMTSTAT, this, std::placeholders::_1, std::placeholders::_2));
            mInitFlags.set(init_flags_e::URC_CALLBACK);
        }

        if (mInitFlags.test(init_flags_e::SERIAL_PORT) == false)
        {
            if (mProcessor.Init() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO INITIALIZE SERIAL PORT FOR CAT.M1 MODEM");
                return Status(Status::Code::BAD);
            }
            mInitFlags.set(init_flags_e::SERIAL_PORT);
        }

        if (mInitFlags.test(init_flags_e::DIGITAL_PIN) == false)
        {
            pinMode(mPinStatus, INPUT);
            pinMode(mPinReset, OUTPUT);
            attachInterrupt(digitalPinToInterrupt(mPinStatus), handlePinStatusISR, FALLING);
            mInitFlags.set(init_flags_e::DIGITAL_PIN);
            resetModule();
        }

        LOG_VERBOSE(logger, "Initialized the CatM1 instance");
        mState = state_e::SUCCEDDED_TO_INITIALIZE;
        return Status(Status::Code::GOOD);
    }

    Status CatM1::Config(jvs::config::Base* config)
    {
        assert(config != nullptr);
        assert(config->GetCategory() == jvs::cfg_key_e::LTE_CatM1);

        mConfig = std::make_pair(true, *static_cast<jvs::config::CatM1*>(config));

        if (mConfig.second.GetModel().second == jvs::md_e::LM5)
        {
            digitalWrite(mPinReset, HIGH);
        }
        else if (mConfig.second.GetModel().second == jvs::md_e::LCM300)
        {
            digitalWrite(mPinReset, LOW);
        }

        mState = (mState == state_e::SUCCEDDED_TO_GET_IP) ? 
            state_e::SUCCEDDED_TO_GET_IP : 
            state_e::SUCCEDDED_TO_CONFIGURE;
        return Status(Status::Code::GOOD);
    }

    Status CatM1::Connect()
    {
        constexpr uint8_t MAX_RETRY_COUNT = 5;
        constexpr uint16_t SECOND_IN_MILLIS = 1000;

        for (uint8_t i = 0; i < MAX_RETRY_COUNT; ++i)
        {
            if (isModemAvailable() == Status::Code::GOOD)
            {
                break;
            }
            else
            {
                LOG_WARNING(logger, "LTE Cat.M1 MODEM IS NOT AVAILABLE");
            }
            
            if ((i + 1) == MAX_RETRY_COUNT)
            {
                LOG_ERROR(logger, "FAILED TO COMMUNICATE WITH THE MODEM");
                return Status(Status::Code::BAD_NO_COMMUNICATION);
            }

            vTaskDelay(2 * SECOND_IN_MILLIS / portTICK_PERIOD_MS);
        }

        if (mConnFlags.test(conn_flags_e::PDP_CONFIGURED) == false &&
            configurePdpContext() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONFIGURE PDP CONTEXT");
            return Status(Status::Code::BAD);
        }
        mConnFlags.set(conn_flags_e::PDP_CONFIGURED);

        if (mConnFlags.test(conn_flags_e::PDP_ACTIVATED) == false &&
            activatePdpContext() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO ACTIVATE PDP CONTEXT");
            return Status(Status::Code::BAD);
        }
        mConnFlags.set(conn_flags_e::PDP_ACTIVATED);

        mState = state_e::SUCCEDDED_TO_CONNECT;

        if (mConnFlags.test(conn_flags_e::GOT_OPERATOR) == false &&
            checkOperator() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CHECK OPERATOR STATUS");
            return Status(Status::Code::BAD);
        }
        mConnFlags.set(conn_flags_e::GOT_OPERATOR);

        if (mConnFlags.test(conn_flags_e::GOT_REGISTERED) == false &&
            checkRegistration() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CHECK REGISTRATION STATUS");
            return Status(Status::Code::BAD);
        }
        mConnFlags.set(conn_flags_e::GOT_REGISTERED);

        mState = state_e::SUCCEDDED_TO_GET_IP;
        return Status(Status::Code::GOOD);
    }

    /**
     * @todo 현행과 동일한 방식을 유지할지 아니면 다른 방식을 적용할지 결정해야 함
     *       예시: PDP context를 모두 제거하거나 전원 종료 명령을 내릴지
     */
    Status CatM1::Disconnect()
    {
        if (mConfig.second.GetModel().second == jvs::md_e::LM5)
        {
            digitalWrite(mPinReset, LOW);
        }
        else
        {
            digitalWrite(mPinReset, HIGH);
        }

        LOG_INFO(logger, "Disconnected the LTE Cat.M1 module");
        mState = state_e::CatM1_DISCONNECTED;

        mInitFlags.reset(init_flags_e::APP_READY);
        mInitFlags.reset(init_flags_e::FUNCTIONS);
        mInitFlags.reset(init_flags_e::MODEM_BBP);
        mInitFlags.reset(init_flags_e::SMS_REPORT);
        mInitFlags.reset(init_flags_e::USIM_PIN);

        mConnFlags.reset(conn_flags_e::GOT_OPERATOR);
        mConnFlags.reset(conn_flags_e::GOT_REGISTERED);
        mConnFlags.reset(conn_flags_e::MODEM_AVAILABLE);
        mConnFlags.reset(conn_flags_e::PDP_ACTIVATED);
        mConnFlags.reset(conn_flags_e::PDP_CONFIGURED);

        return Status(Status::Code::GOOD);
    }

    Status CatM1::Reconnect()
    {
        resetModule();
        mState = state_e::CatM1_DISCONNECTED;
        return Status(Status::Code::GOOD);
    }

    bool CatM1::IsConnected()
    {
        if (mState == state_e::SUCCEDDED_TO_GET_IP)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    IPAddress CatM1::GetIPv4() const
    {
        return IPAddress(0,0,0,0);
    }

    CatM1::state_e CatM1::GetState() const
    {
        return mState;
    }

    Status CatM1::SyncNTP()
    {
        const std::string defaultTZ = "UTC";
        Status ret = SetTimezone(defaultTZ);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET TIMEZONE: %s", ret.c_str());
            return ret;
        }
        
        const std::string command = "AT+QLTS=1";
        const std::string expected = "+QLTS: ";
        const uint32_t timeoutMillis = 300;
        const uint32_t startMillis = millis();
        std::string rxd;

        ret = mProcessor.Write(command);
        if (ret == Status::Code::BAD_TOO_MANY_OPERATIONS)
        {
            LOG_WARNING(logger, "THE MODEM IS BUSY. TRY LATER");
            return ret;
        }

        while (uint32_t(millis() - startMillis) < timeoutMillis)
        {
            while (mProcessor.GetAvailableBytes() > 0)
            {
                rxd += mProcessor.Read();
            }
            
            if (rxd.find(expected) != std::string::npos)
            {
                goto FOUND_EXPECTED_RESPONSE;
            }
            else
            {
                continue;
            }
        }

        return Status(Status::Code::BAD_NO_COMMUNICATION);

    FOUND_EXPECTED_RESPONSE:
        rxd = rxd.substr(rxd.find("\"") + 1);
        rxd = rxd.substr(0, rxd.find("\""));
        rxd = rxd.substr(0, 19);
        std::replace(rxd.begin(), rxd.end(), ',', ' ');

        std::tm timeInfo = {};
        std::istringstream ss(rxd);
        ss >> std::get_time(&timeInfo, "%Y/%m/%d %H:%M:%S");
        if (ss.fail())
        {
            LOG_ERROR(logger, "FAILED TO PARSE TIME INFO: %s", rxd.c_str());
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        std::time_t epochTime = std::mktime(&timeInfo);
        ret = SetSystemTime(epochTime);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET SYSTEM TIME: %s", ret.c_str());
            return ret;
        }
        
        const std::string tz = "Asia/Seoul";
        ret = SetTimezone(tz);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO SET TIMEZONE: %s", ret.c_str());
            return ret;
        }

        LOG_INFO(logger, "Synchronized with NTP in timezone: %s", tz.c_str());
        return Status(Status::Code::GOOD);
    }

    void CatM1::KillUrcTask(bool forOTA)
    {
        mProcessor.StopUrcHandleTask(forOTA);
    }
    
    std::pair<Status, size_t> CatM1::TakeMutex()
    {
        if (xSemaphoreTake(xSemaphore, 2000)  != pdTRUE)
        {
            LOG_WARNING(logger, "FAILED TO TAKE MUTEX FOP LTE Cat.M1. TRY LATER.");
            return std::make_pair(Status(Status::Code::BAD_TOO_MANY_OPERATIONS), mMutexHandle);
        }

        ++mMutexHandle;
        return std::make_pair(Status(Status::Code::GOOD), mMutexHandle);
    }

    Status CatM1::ReleaseMutex()
    {
        xSemaphoreGive(xSemaphore);
        return Status(Status::Code::GOOD);
    }

    Status CatM1::Execute(const std::string& command, const size_t mutexHandle)
    {
        if (mutexHandle != mMutexHandle)
        {
            return Status(Status::Code::BAD_SEMPAHORE_FILE_MISSING);
        }

        if (mConnFlags.test(conn_flags_e::STATUS_PIN_GOOD) == false)
        {
            LOG_ERROR(logger, "FAILED TO EXECUTE DUE TO BAD MODEM STATUS. CHECK CONNECTION CABLE");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        else if (mState == state_e::CatM1_DISCONNECTED)
        {
            LOG_ERROR(logger, "FAILED TO EXECUTE DUE TO DISCONNECTED MODEM");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        
        return mProcessor.Write(command);
    }

    size_t CatM1::GetAvailableBytes()
    {
        return mProcessor.GetAvailableBytes();
    }

    int16_t CatM1::Read()
    {
        return mProcessor.Read();
    }

    std::string CatM1::ReadBetweenPatterns(const std::string& patternBegin, const std::string& patternEnd)
    {
        return mProcessor.ReadBetweenPatterns(patternBegin, patternEnd);
    }

    Status CatM1::isModemAvailable()
    {
        const std::string command = "AT";
        const std::string expected = "OK";
        const uint32_t timeoutMillis = 300;
        const uint32_t startMillis = millis();
        std::string rxd;

        Status ret = mProcessor.Write(command);
        if (ret == Status::Code::BAD_TOO_MANY_OPERATIONS)
        {
            LOG_WARNING(logger, "THE MODEM IS BUSY. TRY LATER");
            return ret;
        }

        while (uint32_t(millis() - startMillis) < timeoutMillis)
        {
            while (mProcessor.GetAvailableBytes() > 0)
            {
                rxd += mProcessor.Read();
            }
            
            if (rxd.find(expected) != std::string::npos)
            {
                return Status(Status::Code::GOOD);
            }
            else
            {
                continue;
            }
        }

        return Status(Status::Code::BAD_NO_COMMUNICATION);
    }

    /**
     * @todo lexing, tokenizing 적용하여 판단하도록 로직 및 로거를 변경해야 함
     *       이를 위해 해외향지의 오퍼레이터 정보를 확인하는 것이 필요함
     * @todo LTE 모뎀 통신이 안된다는 내용의 접수를 받을 때가 있습니다. 현재 발견한
     *       원인 중의 하나로 설정된 OPERATOR가 없는 경우가 있습니다. 설정이 안 된건지
     *       아니면 AUTOMATIC 설정이 실패한 것인지는 아직 알지 못하지만 이러한 경우에는 
     *       수동으로 OPERATOR를 설정하면 정상적으로 동작하는 것을 확인한 적이 있습니다.
     */
    Status CatM1::checkOperator()
    {
        const std::string command = "AT+COPS?";
        const std::string expected = "OK";
        const uint32_t timeoutMillis = 180 * 1000;
        const uint32_t startMillis = millis();
        std::string rxd;

        Status ret = mProcessor.Write(command);
        if (ret == Status::Code::BAD_TOO_MANY_OPERATIONS)
        {
            LOG_WARNING(logger, "THE MODEM IS BUSY. TRY LATER");
            return ret;
        }

        while (uint32_t(millis() - startMillis) < timeoutMillis)
        {
            while (mProcessor.GetAvailableBytes() > 0)
            {
                rxd += mProcessor.Read();
            }
            
            if (rxd.find(expected) != std::string::npos)
            {
                const size_t posMode              = 1 + rxd.find(" ");
                const size_t posFormat            = 1 + rxd.find(",", posMode + 1);
                const size_t posOperator          = 1 + rxd.find(",", posFormat + 1);
                const size_t posAccessTechnology  = 1 + rxd.find(",", posOperator + 1);

                if ((posMode == std::string::npos)           || 
                    (posFormat == std::string::npos)         || 
                    (posOperator == std::string::npos)       || 
                    (posAccessTechnology == std::string::npos))
                {
                    LOG_DEBUG(logger, "%s", rxd.c_str());
                    return Status(Status::Code::GOOD);
                }
                
                const std::string mode               = rxd.substr(posMode, posFormat - posMode - 1);
                const std::string format             = rxd.substr(posFormat, posOperator - posFormat - 1);
                const std::string operatorName       = rxd.substr(posOperator + 1, posAccessTechnology - posOperator - 3);
                const std::string accessTechnology   = rxd.substr(posAccessTechnology, 1);
                LOG_DEBUG(logger, "Mode: %s, Format: %s, Operator: %s, Access Technology: %s", 
                    mode.c_str(), format.c_str(), operatorName.c_str(), accessTechnology.c_str());
                
                return Status(Status::Code::GOOD);
            }
            else if (rxd.find("ERROR") != std::string::npos)
            {
                LOG_ERROR(logger, "FAILED TO QUERY OPERATOR: %s", rxd.c_str());
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }
            else
            {
                continue;
            }
        }

        return Status(Status::Code::BAD_NO_COMMUNICATION);
    }

    /**
     * @todo lexing, tokenizing 적용하여 판단하도록 로직 및 로거를 변경해야 함
     *       이를 위해 <urc><stat> 값을 해석하고 처리하는 코드의 구현이 필요함
     */
    Status CatM1::checkRegistration()
    {
        const std::string command = "AT+CREG?";
        const std::string expected = "OK";
        const uint32_t timeoutMillis = 300;
        const uint32_t startMillis = millis();
        std::string rxd;

        Status ret = mProcessor.Write(command);
        if (ret == Status::Code::BAD_TOO_MANY_OPERATIONS)
        {
            LOG_WARNING(logger, "THE MODEM IS BUSY. TRY LATER");
            return ret;
        }

        while (uint32_t(millis() - startMillis) < timeoutMillis)
        {
            while (mProcessor.GetAvailableBytes() > 0)
            {
                rxd += mProcessor.Read();
            }
            
            if (rxd.find(expected) != std::string::npos)
            {
                if (rxd.find("0,1") != std::string::npos)
                {
                    LOG_INFO(logger, "Registered, home network");
                    return Status(Status::Code::GOOD);
                }
                else
                {
                    LOG_WARNING(logger, "%s", rxd.c_str());
                    return Status(Status::Code::BAD_DEVICE_FAILURE);
                }
            }
            else if (rxd.find("ERROR") != std::string::npos)
            {
                LOG_ERROR(logger, "FAILED TO QUERY REGISTRATION STATUS: %s", rxd.c_str());
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }
            else
            {
                continue;
            }
        }

        return Status(Status::Code::BAD_TIMEOUT);
    }

    /**
     * @todo lexing, tokenizing 적용하여 판단하도록 로직 및 로거를 변경해야 함
     * @todo Context ID 등의 파라미터를 입력받아 처리할 수 있도록 해야 함
     */
    Status CatM1::configurePdpContext()
    {
        const std::string command = "AT+QICSGP=1,3,\"\",\"\",\"\",0";
        const std::string expected = "OK";
        const uint32_t timeoutMillis = 300;
        const uint32_t startMillis = millis();
        std::string rxd;

        Status ret = mProcessor.Write(command);
        if (ret == Status::Code::BAD_TOO_MANY_OPERATIONS)
        {
            LOG_WARNING(logger, "THE MODEM IS BUSY. TRY LATER");
            return ret;
        }

        while (uint32_t(millis() - startMillis) < timeoutMillis)
        {
            while (mProcessor.GetAvailableBytes() > 0)
            {
                rxd += mProcessor.Read();
            }
            
            if (rxd.find(expected) != std::string::npos)
            {
                LOG_INFO(logger, "ID: 1, Type: IPv4/IPv6 %s", rxd.c_str());
                return Status(Status::Code::GOOD);
            }
            else if (rxd.find("ERROR") != std::string::npos)
            {
                LOG_ERROR(logger, "FAILED TO QUERY REGISTRATION STATUS: %s", rxd.c_str());
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }
            else
            {
                continue;
            }
        }

        return Status(Status::Code::BAD_TIMEOUT);
    }

    /**
     * @todo lexing, tokenizing 적용하여 판단하도록 로직 및 로거를 변경해야 함
     * @todo Context ID 등의 파라미터를 입력받아 처리할 수 있도록 해야 함
     */
    Status CatM1::activatePdpContext()
    {
        const std::string command = "AT+QIACT=1";
        const std::string expected = "OK";
        const uint32_t timeoutMillis = 150 * 1000;
        const uint32_t startMillis = millis();
        std::string rxd;

        Status ret = mProcessor.Write(command);
        if (ret == Status::Code::BAD_TOO_MANY_OPERATIONS)
        {
            LOG_WARNING(logger, "THE MODEM IS BUSY. TRY LATER");
            return ret;
        }

        while (uint32_t(millis() - startMillis) < timeoutMillis)
        {
            while (mProcessor.GetAvailableBytes() > 0)
            {
                rxd += mProcessor.Read();
            }
            
            if (rxd.find(expected) != std::string::npos)
            {
                LOG_INFO(logger, "ID: 1 %s", rxd.c_str());
                return Status(Status::Code::GOOD);
            }
            else if (rxd.find("ERROR") != std::string::npos)
            {
                LOG_ERROR(logger, "FAILED TO ACTIVATE PDP CONTEXT: %s", rxd.c_str());
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }
            else
            {
                continue;
            }
        }

        return Status(Status::Code::BAD_TIMEOUT);
    }

    void CatM1::getPdpContext()
    {
        const std::string command = "AT+QIACT?";
        const std::string expected = "OK";
        const uint32_t timeoutMillis = 300;
        const uint32_t startMillis = millis();
        std::string rxd;

        Status ret = mProcessor.Write(command);
        if (ret == Status::Code::BAD_TOO_MANY_OPERATIONS)
        {
            LOG_WARNING(logger, "THE MODEM IS BUSY. TRY LATER");
            return ;
        }

        while (uint32_t(millis() - startMillis) < timeoutMillis)
        {
            while (mProcessor.GetAvailableBytes() > 0)
            {
                rxd += mProcessor.Read();
            }
            
            if (rxd.find(expected) != std::string::npos)
            {
                LOG_INFO(logger, "%s", rxd.c_str());
            }
        }
    }

    void CatM1::resetModule()
    {
        if (mConfig.second.GetModel().second == jvs::md_e::LM5)
        {
            digitalWrite(mPinReset, LOW);
            delay(110);
            digitalWrite(mPinReset, HIGH);
        }
        else
        {
            digitalWrite(mPinReset, HIGH);
            delay(110);
            digitalWrite(mPinReset, LOW);
        }

        LOG_INFO(logger, "Reset LTE Cat.M1 module");
        mInitFlags.reset(init_flags_e::APP_READY);
        mInitFlags.reset(init_flags_e::FUNCTIONS);
        mInitFlags.reset(init_flags_e::MODEM_BBP);
        mInitFlags.reset(init_flags_e::SMS_REPORT);
        mInitFlags.reset(init_flags_e::USIM_PIN);

        mConnFlags.reset(conn_flags_e::GOT_OPERATOR);
        mConnFlags.reset(conn_flags_e::GOT_REGISTERED);
        mConnFlags.reset(conn_flags_e::MODEM_AVAILABLE);
        mConnFlags.reset(conn_flags_e::PDP_ACTIVATED);
        mConnFlags.reset(conn_flags_e::PDP_CONFIGURED);
    }

    void CatM1::onEventPinStatusFalling(void* pvParameter, uint32_t ulParameter)
    {
        mConnFlags.reset(conn_flags_e::STATUS_PIN_GOOD);
        mInitFlags.reset(init_flags_e::MODEM_BBP);
        mInitFlags.reset(init_flags_e::FUNCTIONS);
        mInitFlags.reset(init_flags_e::USIM_PIN);
        mInitFlags.reset(init_flags_e::SMS_REPORT);
        mInitFlags.reset(init_flags_e::APP_READY);
        mState = state_e::CatM1_DISCONNECTED;

        LOG_WARNING(logger, "[ISR] MODEM STATUS BAD. CHECK CABLE CONNECTION");
    }

    /**
     * @todo 함수실행지연타이머 생성 실패 시 이에 대한 해결책이 필요함
     *       일단, 시간이 없으니 관련된 플래그만 리셋하게 땜빵만 함
     */
    void IRAM_ATTR CatM1::handlePinStatusISR()
    {
        if (millis() - mLastInterruptMillis > mDebounceMillis)
        {
            mLastInterruptMillis = millis();
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            BaseType_t pendingCreated = xTimerPendFunctionCallFromISR(
                                            onEventPinStatusFalling, 
                                            NULL, 
                                            0, 
                                            &xHigherPriorityTaskWoken
                                        );

            if (pendingCreated != pdPASS)
            {
                mConnFlags.reset(conn_flags_e::STATUS_PIN_GOOD);
                mInitFlags.reset(init_flags_e::MODEM_BBP);
                mInitFlags.reset(init_flags_e::FUNCTIONS);
                mInitFlags.reset(init_flags_e::USIM_PIN);
                mInitFlags.reset(init_flags_e::SMS_REPORT);
                mInitFlags.reset(init_flags_e::APP_READY);
                mState = state_e::CatM1_DISCONNECTED;
            }
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    void CatM1::onEventRDY()
    {
        LOG_INFO(logger, "Callback: LTE Cat.M1 modem is being initialized");
        mConnFlags.set(conn_flags_e::STATUS_PIN_GOOD);
        mInitFlags.set(init_flags_e::MODEM_BBP);
        mInitFlags.reset(init_flags_e::FUNCTIONS);
        mInitFlags.reset(init_flags_e::USIM_PIN);
        mInitFlags.reset(init_flags_e::SMS_REPORT);
        mInitFlags.reset(init_flags_e::APP_READY);
    }

    void CatM1::onEventCFUN()
    {
        LOG_INFO(logger, "Callback: All function of the ME is initialized");
        mInitFlags.set(init_flags_e::FUNCTIONS);
    }

    void CatM1::onEventCPIN(const std::string& state)
    {
        if (state == "READY")
        {
            LOG_INFO(logger, "Callback: MT can access USIM without PIN");
            mInitFlags.set(init_flags_e::USIM_PIN);
        }
        else
        {
            LOG_WARNING(logger, "Callback: MT IS PENDING FOR PIN TO ACCESS (U)SIM");
            // call method to set the pin numbers to activate the sim card
        }
    }

    void CatM1::onEventQMTSTAT(const uint8_t socketID, const uint8_t errorCode)
    {
        LOG_INFO(logger, "Callback: BROKER DISCONNECTED");
        mState = state_e::CatM1_DISCONNECTED;
    }

    void CatM1::onEventQIND()
    {
        LOG_INFO(logger, "Callback: SMS functionality is initialized");
        mInitFlags.set(init_flags_e::SMS_REPORT);
        checkCatM1Started();
        
    }

    void CatM1::onEventAPPRDY()
    {
        LOG_INFO(logger, "Callback: Modem's AP is initialized");
        mInitFlags.set(init_flags_e::APP_READY);
        checkCatM1Started();
    }

    void CatM1::checkCatM1Started()
    {
        if (mInitFlags.all() == true)
        {
            mState = state_e::SUCCEDDED_TO_START;
        }
    }

    Status CatM1::GetSignalQuality(catm1_report_t* _struct)
    {
        const std::string command = "AT+QCSQ";
        const std::string expected = "OK";
        const uint32_t timeoutMillis = 300;
        const uint32_t startMillis = millis();
        std::string rxd;

        Status ret = mProcessor.Write(command);
        if (ret == Status::Code::BAD_TOO_MANY_OPERATIONS)
        {
            LOG_WARNING(logger, "THE MODEM IS BUSY. TRY LATER");
            return ret;
        }

        while (uint32_t(millis() - startMillis) < timeoutMillis)
        {
            while (mProcessor.GetAvailableBytes() > 0)
            {
                rxd += mProcessor.Read();
            }
            
            if (rxd.find(expected) != std::string::npos)
            {
                goto FOUND_EXPECTED_RESPONSE;
            }
            else
            {
                continue;
            }
        }

        return Status(Status::Code::BAD_NO_COMMUNICATION);

    FOUND_EXPECTED_RESPONSE:

        size_t idxRSSI = rxd.find(",") + 1;
        size_t idxRSRP = rxd.find(",", idxRSSI) + 1;
        size_t idxSINR = rxd.find(",", idxRSRP) + 1;
        size_t idxRSRQ = rxd.find(",", idxSINR) + 1;

        if ( idxRSSI == 0 || idxRSRP == 0 || idxSINR == 0 || idxRSRQ == 0 )
        {
            LOG_ERROR(logger,"Failed to get signal quality report due to invalid response \n");
            return Status(Status::Code::BAD_INVALID_ARGUMENT);
        }

        _struct->Enabled = true;
        _struct->RSSI = std::stoi(rxd.substr(idxRSSI, idxRSRP - idxRSSI - 1));
        _struct->RSRP = std::stoi(rxd.substr(idxRSRP, idxSINR - idxRSRP - 1));
        _struct->SINR = std::stoi(rxd.substr(idxSINR, idxRSRQ - idxSINR - 1));
        _struct->SINR = 0.2 * _struct->SINR - 20;
        _struct->RSRQ = std::stoi(rxd.substr(idxRSRQ));

        return Status(Status::Code::GOOD);
    }


    CatM1* catM1 = nullptr;
}