/**
 * @file CLI.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @brief 
 * @version 1.3.1
 * @date 2025-01-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */



#include <regex>
#include <HardwareSerial.h>

#include "CLI.h"
#include "Common/Logger/Logger.h"
#include "IM/Custom/Constants.h"
#include "JARVIS/Include/TypeDefinitions.h"
#include "Storage/ESP32FS/ESP32FS.h"

namespace muffin {

    Status CommandLineInterface::Init()
    {
        unsigned long previousTime = millis();
        int countdown = 5;  // 5 카운트다운

        while (true) 
        { 	
            if (millis() - previousTime >= 1000) 
            {  
                previousTime = millis();
                Serial.print("\rPress the Enter key to configure the device settings [");
                Serial.print(" ");
                Serial.print(countdown);
                Serial.print(" ] Seconds left");

                countdown--;
                
                if (countdown < 0) 
                {
                    Serial.print("\r\nTime has expired. Proceeding with device execution\r\n");
                    delay(1000);
                    return Status(Status::Code::GOOD_NO_DATA);
                }
            }

            if (Serial.available()) 
            {
                Serial.read(); 
                while (Serial.available()) 
                {
                    Serial.read();
                }
                Serial.print("\r\n\r\n");
                return startCLI();
            }
        }
    }

    Status CommandLineInterface::startCLI()
    {
        logger.SetLevel(muffin::log_level_e::LOG_LEVEL_ERROR);
        esp32FS.Begin(false);

        while (true)
        {
            Status ret = displaySettingsMenu();
            if (ret == Status::Code::GOOD_DEPENDENT_VALUE_CHANGED)
            {
                return Status(Status::Code::GOOD);
            }
            else if (ret != Status::Code::GOOD)
            {
                return ret;
            }  
        }
    }

    Status CommandLineInterface::saveServiceUrlJson()
    {
        File file = esp32FS.Open(SERVICE_URL_PATH, "w", true);
        serializeJson(mServiceUrlJson, file);
        file.close();

        return Status(Status::Code::GOOD);
    }

    Status CommandLineInterface::loadServiceUrlJson()
    {
        Status ret = esp32FS.DoesExist(SERVICE_URL_PATH);
        if (ret == Status::Code::BAD_NOT_FOUND)
        {
            JsonDocument doc;
            doc["ver"] = "v4";
            JsonObject mqtt = doc["mqtt"].to<JsonObject>();

            mqtt["host"] = "mmm.broker.edgecross.ai";
            mqtt["port"] = 8883;
            mqtt["scheme"] = 2;
            mqtt["checkCert"] = true;
            mqtt["id"]   = "edgeaiot";
            mqtt["pw"]   = "!edge1@1159";

            
            JsonObject mfm = doc["mfm"].to<JsonObject>();
            mfm["host"] = "api.mfm.edgecross.ai";
            mfm["port"] = 443;
            mfm["scheme"] = 2;
            mfm["checkCert"] = true;
            doc["ntp"] = "time.google.com";

            mServiceUrlJson = doc;
        }
        else
        {
            if (mServiceUrlJson.isNull() == true)
            {
                mServiceUrlJson.clear();
                mServiceUrlJson.shrinkToFit();
                
                JSON json;
                File file = esp32FS.Open(SERVICE_URL_PATH, "r", false);
                ret = json.Deserialize(file, &mServiceUrlJson);
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO DESERIALIZE: %s", ret.c_str());
                    return ret;
                }
                LOG_INFO(logger, "Deserialized Service URL file");
            }
        }

        return Status(Status::Code::GOOD);
    }

    Status CommandLineInterface::configureMqttURL()
    {
        Status ret = loadServiceUrlJson();
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }
        std::string settingStr;

        uint16_t _port        = mServiceUrlJson["mqtt"]["port"].as<uint16_t>();
        uint8_t _scheme       = mServiceUrlJson["mqtt"]["scheme"].as<uint8_t>();
        std::string _host     = mServiceUrlJson["mqtt"]["host"].as<std::string>();
        bool _checkCert       = mServiceUrlJson["mqtt"]["checkCert"].as<bool>();
        std::string _userName = mServiceUrlJson["mqtt"]["id"].as<std::string>();
        std::string _password = mServiceUrlJson["mqtt"]["pw"].as<std::string>();
        
        while (true)
        {
            std::string maskedPassword(_password.length(), '*');

            Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
            Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
            Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
            Serial.print("+--------------------------------------------------------------+\r\n");
            Serial.print("|             MQTT Broker Information Settings                 |\r\n");
            Serial.print("+----------------------+---------------------------------------+\r\n");
            Serial.print("|  [1] MQTT host       | ");                      
            printLeftAlignedText(_host,38);
            Serial.print(" |\r\n");
            Serial.print("+----------------------+---------------------------------------+\r\n");
            Serial.print("|  [2] MQTT port       | ");
            printLeftAlignedText(std::to_string(_port),38);
            Serial.print(" |\r\n");
            Serial.print("+----------------------+---------------------------------------+\r\n");
            Serial.print("|  [3] MQTT scheme     | ");
            printLeftAlignedText(_scheme == 1 ? "MQTT" : "MQTTS",38);
            Serial.print(" |\r\n");
            Serial.print("+----------------------+---------------------------------------+\r\n");
            Serial.print("|  [4] MQTT user name  | ");
            printLeftAlignedText(_userName,38);
            Serial.print(" |\r\n");
            Serial.print("+----------------------+---------------------------------------+\r\n");
            Serial.print("|  [5] MQTT password   | ");
            printLeftAlignedText(maskedPassword,38);
            Serial.print(" |\r\n");
            Serial.print("+----------------------------------+---------------------------+\r\n");
            Serial.print("|  [6] TLS Certification Check     | ");
            printLeftAlignedText(_checkCert == true ? "Enabled" : "Disabled",26);
            Serial.print(" |\r\n");
            Serial.print("+--------------------------------------------------------------+\r\n");
            Serial.print("|  [7] >>>  Save and Exit  <<<                                 |\r\n");
            Serial.print("|  [9] Exit                                                    |\r\n");
            Serial.print("+--------------------------------------------------------------+\r\n");
            
            Serial.print("\r\nSelect an option [1 - 9]\r\n");
            
            delay(80);
            settingStr = getSerialInput();

            uint16_t inputCommand = 0;
            try 
            {
                inputCommand = std::stoul(settingStr);
            } 
            catch (const std::invalid_argument& e) 
            {
                Serial.print("\r\nInvalid input (non-numeric). Enter a number between [1 - 9]\r\n");
                continue;
            } 
            catch (const std::out_of_range& e) 
            {
                Serial.print("\r\nInput is out of range. Enter a number between [1 - 9]\r\n");
                continue;
            }

            switch (inputCommand)
            {
            case 1:
            {    
                Serial.print("\r\nEnter a MQTT broker host\r\n");
                _host = getSerialInput();
                
                break;
            }
            case 2:
            {
                Serial.print("\r\nEnter a MQTT broker port\r\n");
                settingStr = getSerialInput();

                try
                {
                    _port = static_cast<uint16_t>(std::stoul(settingStr));
                }
                catch (const std::invalid_argument& e)
                {
                    Serial.print("\r\nInvalid input (non-numeric). Enter a number between [0 - 65535]\r\n");
                }
                catch (const std::out_of_range& e)
                {
                    Serial.print("\r\nInput is out of range. Enter a number between [0 - 65535]\r\n");
                }

                break;
            }
            case 3:
            {    
                Serial.print("\r\nEnter a MQTT scheme");
                Serial.print("\r\n[1] MQTT");
                Serial.print("\r\n[2] MQTTS\r\n");
                settingStr = getSerialInput();

                if (settingStr == "1")
                {
                    _scheme = 1;
                    _checkCert = false;
                }
                else if (settingStr == "2")
                {
                    _scheme = 2;
                }
                else
                {
                    Serial.print("\r\nInput is out of range. Enter a number between [1 or 2]\r\n");
                }

                break;
            }
            case 4:
            {
                Serial.print("\r\nEnter a MQTT broker user name\r\n");
                _userName = getSerialInput();
                break;
            }
            case 5:
            {
                while (true)
                {
                    Serial.print("\r\nEnter a MQTT broker password\r\n");
                    std::string firstInputPassword = getSerialInput();
                    Serial.print("\r\nRe-enter the password to confirm\r\n");
                    std::string secondInputPassword = getSerialInput();
                    if (firstInputPassword == secondInputPassword)
                    {
                        Serial.println("\r\nPassword confirmed and saved");
                        _password = firstInputPassword;
                        break;
                    }
                    else
                    {
                        Serial.println("\r\nPasswords do not match. Try again\r\n\r\n\r\n");
                    }
                }
                break;
            }
            case 6:
            {
                Serial.print("\r\nChoose whether to enable TLS certificate validation for MQTT connection");
                Serial.print("\r\n\r\n[1] Enabled");
                Serial.print("\r\n[2] Disabled (Skip certificate check)\r\n");
                settingStr = getSerialInput();

                if (settingStr == "1")
                {
                    _checkCert = true;
                }
                else if (settingStr == "2")
                {
                    _checkCert = false;
                }
                else
                {
                    Serial.print("\r\nInput is out of range. Enter a number between [1 or 2]\r\n");
                }

                break;
            }
            case 7:
                mServiceUrlJson["mqtt"]["host"]        = _host;
                mServiceUrlJson["mqtt"]["port"]        = _port;
                mServiceUrlJson["mqtt"]["scheme"]      = _scheme;
                mServiceUrlJson["mqtt"]["id"]          = _userName;
                mServiceUrlJson["mqtt"]["pw"]          = _password;
                mServiceUrlJson["mqtt"]["checkCert"]   = _checkCert;

                Serial.print("\r\nMqtt broker settings have been saved \r\n\r\n\r\n");
                return saveServiceUrlJson();
            case 9:
                Serial.print("\r\nMqtt broker settings has not been changed. Exiting settings\r\n\r\n");
                return Status(Status::Code::GOOD);
            default:
                Serial.print("\r\nInvalid input. Enter a number between [1 - 9]\r\n");
                break;
            }
        }
    }

    Status CommandLineInterface::configureNtpURL()
    {
        Status ret = loadServiceUrlJson();
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }
        std::string settingStr;


        std::string _ntpURL = mServiceUrlJson["ntp"].as<std::string>();

        while (true)
        {
            Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
            Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
            Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
            Serial.print("+--------------------------------------------------------------+\r\n");
            Serial.print("|                  NTP URL Information Settings                |\r\n");
            Serial.print("+------------------+-------------------------------------------+\r\n");
            Serial.print("|  [1] NTP URL     | ");
            printLeftAlignedText(_ntpURL,40);
            Serial.print("   |\r\n");
            Serial.print("+--------------------------------------------------------------+\r\n");
            Serial.print("|  [2] >>>  Save and Exit  <<<                                 |\r\n");
            Serial.print("|  [9] Exit                                                    |\r\n");
            Serial.print("+--------------------------------------------------------------+\r\n");
            
            Serial.print("\r\nSelect an option [1 - 9]\r\n");
            
            delay(80);
            settingStr = getSerialInput();

            uint16_t inputCommand = 0;
            try 
            {
                inputCommand = std::stoul(settingStr);
            } 
            catch (const std::invalid_argument& e) 
            {
                Serial.print("\r\nInvalid input (non-numeric). Enter a number between [1 - 9]\r\n");
                continue;
            } 
            catch (const std::out_of_range& e) 
            {
                Serial.print("\r\nInput is out of range. Enter a number between [1 - 9]\r\n");
                continue;
            }

            switch (inputCommand)
            {
            case 1:
            {
                Serial.print("\r\n\r\n\r\nEnter a NTP server URL\r\n");
                _ntpURL = getSerialInput();

                break;
            }
            case 2:
            {
                mServiceUrlJson["ntp"] = _ntpURL;
                Serial.print("\r\nNTP server URL settings have been saved \r\n\r\n\r\n");
                return saveServiceUrlJson();
                break;
            }
            case 9:
                Serial.print("\r\nnNTP server URL settings has not been changed. Exiting settings\r\n\r\n");
                return Status(Status::Code::GOOD);
            default:
                Serial.print("\r\nInvalid input. Enter a number between [1 - 9]\r\n");
                break;
            }

        }
    }

    Status CommandLineInterface::configureMfmURL()
    {
        Status ret = loadServiceUrlJson();
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }

        std::string settingStr;

        uint16_t _port     = mServiceUrlJson["mfm"]["port"].as<uint16_t>();
        std::string _host  = mServiceUrlJson["mfm"]["host"].as<std::string>();
        uint16_t _scheme   = mServiceUrlJson["mfm"]["scheme"].as<uint16_t>();
        bool _checkCert    = mServiceUrlJson["mfm"]["checkCert"].as<bool>();

        while (true)
        {
            Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
            Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
            Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
            Serial.print("+--------------------------------------------------------------+\r\n");
            Serial.print("|                    MFM Information Settings                  |\r\n");
            Serial.print("+------------------+-------------------------------------------+\r\n");
            Serial.print("|  [1] MFM host    | ");
            printLeftAlignedText(_host,40);
            Serial.print("   |\r\n");
            Serial.print("+------------------+-------------------------------------------+\r\n");
            Serial.print("|  [2] MFM port    | ");
            printLeftAlignedText(std::to_string(_port),40);
            Serial.print("   |\r\n");
            Serial.print("+------------------+-------------------------------------------+\r\n");
            Serial.print("|  [3] MFM scheme  | ");
            printLeftAlignedText(_scheme == 1 ? "HTTP" : "HTTPS",40);
            Serial.print("   |\r\n");
            Serial.print("+---------------------------------+----------------------------+\r\n");
            Serial.print("|  [4] TLS Certification Check    | ");
            printLeftAlignedText(_checkCert == true ? "Enabled" : "Disabled",25);
            Serial.print("   |\r\n");
            Serial.print("+--------------------------------------------------------------+\r\n");
            Serial.print("|  [5] >>>  Save and Exit  <<<                                 |\r\n");
            Serial.print("|  [9] Exit                                                    |\r\n");
            Serial.print("+--------------------------------------------------------------+\r\n");
            
            Serial.print("\r\n\r\nSelect an option [1 - 9]\r\n\r\n");
        
            delay(80);
            settingStr = getSerialInput();
        
            uint16_t inputCommand = 0;
            try 
            {
                inputCommand = std::stoul(settingStr);
            } 
            catch (const std::invalid_argument& e) 
            {
                Serial.print("\r\nInvalid input (non-numeric). Enter a number between [1 - 9]\r\n");
                continue;
            } 
            catch (const std::out_of_range& e) 
            {
                Serial.print("\r\nInput is out of range. Enter a number between [1 - 9]\r\n");
                continue;
            }

            switch (inputCommand)
            {
            case 1:
            {    
                Serial.print("\r\n\r\n\r\nEnter a MFM Server host\r\n");
                _host = getSerialInput();
                
                break;
            }
            case 2:
            {
                Serial.print("\r\nEnter a MFM Server port\r\n");
                settingStr = getSerialInput();

                try
                {
                    _port = static_cast<uint16_t>(std::stoul(settingStr));
                }
                catch (const std::invalid_argument& e)
                {
                    Serial.print("\r\nInvalid input (non-numeric). Enter a number between [0 - 65535]\r\n");
                }
                catch (const std::out_of_range& e)
                {
                    Serial.print("\r\nInput is out of range. Enter a number between [0 - 65535]\r\n");
                }
                break;
            }
            case 3:
            {
                Serial.print("\r\nEnter a MFM Server HTTP scheme");
                Serial.print("\r\n[1] HTTP");
                Serial.print("\r\n[2] HTTPS\r\n");
                settingStr = getSerialInput();

                if (settingStr == "1")
                {
                    _scheme = 1;
                    _checkCert = false;
                }
                else if (settingStr == "2")
                {
                    _scheme = 2;
                }
                else
                {
                    Serial.print("\r\nInput is out of range Enter a number between [1 or 2]\r\n");
                }
                break;            
            }
            case 4:
            {
                Serial.print("\r\nChoose whether to enable TLS certificate validation for HTTPS requests");
                Serial.print("\r\n\r\n[1] Enabled");
                Serial.print("\r\n[2] Disabled (Skip certificate check)\r\n");
                settingStr = getSerialInput();

                if (settingStr == "1")
                {
                    _checkCert = true;
                }
                else if (settingStr == "2")
                {
                    _checkCert = false;
                }
                else
                {
                    Serial.print("\r\nInput is out of range. Enter a number between [1 or 2]\r\n");
                }
                break;
            }
            case 5:
                mServiceUrlJson["mfm"]["host"]      = _host;
                mServiceUrlJson["mfm"]["port"]      = _port;
                mServiceUrlJson["mfm"]["scheme"]    = _scheme;
                mServiceUrlJson["mfm"]["checkCert"] = _checkCert;
                Serial.print("\r\nMFM Server settings have been saved \r\n\r\n\r\n");
                return saveServiceUrlJson();
            case 9:
                Serial.print("\r\nMFM Server settings has not been changed. Exiting settings\r\n\r\n");
                return Status(Status::Code::GOOD);
            default:
                Serial.print("\r\nInvalid input Enter a number between [1 - 9]\r\n");
                break;
            }
        
        }
        
    }

    Status CommandLineInterface::displaySettingsMenu()
    {
        Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
        Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
        Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
        Serial.print("+--------------------------------------------------------------+\r\n");
        Serial.print("|                        Settings Menu                         |\r\n");
        Serial.print("+--------------------------------------------------------------+\r\n");
        Serial.print("|  [1] Service Network Settings                                |\r\n");
        Serial.print("|  [2] MQTT Broker Information Settings                        |\r\n");
        Serial.print("|  [3] NTP URL Information Settings                            |\r\n");
        Serial.print("|  [4] MFM Information Settings                                |\r\n");
        Serial.print("+--------------------------------------------------------------+\r\n");
        Serial.print("|  [9] Exit                                                    |\r\n");
        Serial.print("+--------------------------------------------------------------+\r\n");
        
        std::string settingStr;
        while (true)
        {
            Serial.print("\r\nSelect an option [1 - 9]\r\n");
            delay(80);
            settingStr = getSerialInput();

            uint16_t inputCommand = 0;
            try 
            {
                inputCommand = std::stoul(settingStr);
            } 
            catch (const std::invalid_argument& e) 
            {
                Serial.print("\r\nInvalid input (non-numeric). Enter a number between [1 - 5]\r\n");
                continue;
            } 
            catch (const std::out_of_range& e) 
            {
                Serial.print("\r\nInput is out of range. Enter a number between [1 - 5]\r\n");
                continue;
            }

            
            switch (inputCommand)
            {
            case 1:
                return configureNetworkInterface();

            case 2:
                return configureMqttURL();

            case 3:
                return configureNtpURL();

            case 4:
                return configureMfmURL();

            case 9:
                Serial.print("\r\nAll settings have been saved. The device will now reboot \r\n\r\n\r\n");
                return Status(Status::Code::GOOD_DEPENDENT_VALUE_CHANGED);

            default:
                Serial.print("\r\nInvalid input Enter a number between [1 - 9]\r\n");
                break;
            }
        }


        
    }

    Status CommandLineInterface::configureNetworkInterface()
    {
        Status ret = esp32FS.DoesExist(JARVIS_PATH);
        if (ret == Status::Code::BAD_NOT_FOUND)
        {
            JsonDocument doc;
            doc["ver"] = "v4";

            JsonObject cnt = doc["cnt"].to<JsonObject>();
            cnt["rs232"].to<JsonArray>();
            cnt["rs485"].to<JsonArray>();
            cnt["wifi"].to<JsonArray>();
            cnt["eth"].to<JsonArray>();
            cnt["mbrtu"].to<JsonArray>();
            cnt["mbtcp"].to<JsonArray>();
            cnt["node"].to<JsonArray>();
            cnt["alarm"].to<JsonArray>();
            cnt["optime"].to<JsonArray>();
            cnt["prod"].to<JsonArray>();
            cnt["mc"].to<JsonArray>();

            JsonArray catm1 = cnt["catm1"].to<JsonArray>();
            JsonObject _catm1 = catm1.add<JsonObject>();
            _catm1["md"]    = "LM5";
            _catm1["ctry"]  = "KR";

            JsonArray op    = cnt["op"].to<JsonArray>();
            JsonObject _op  = op.add<JsonObject>();
            _op["snic"]      = "lte";
            _op["exp"]       = true;
            _op["intvPoll"]  = 1;
            _op["intvSrv"]   = 60;
            _op["rst"]       = false;

            mJarvisJson = doc;
            Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
            Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
            Serial.print("+---------------------+------------+---------------------------+\r\n");
            Serial.print("|                    Service Network Settings                  |\r\n");
            Serial.print("+---------------------+------------+---------------------------+\r\n");
            Serial.print("|  Network Interface  |   Country  |   Modem                   |\r\n");
            Serial.print("+---------------------+------------+---------------------------+\r\n");
            Serial.print("|         LTE         |     KR     |    LM5                    |\r\n");
            Serial.print("+---------------------+------------+---------------------------+\r\n");
        }
        else
        {
            mJarvisJson.clear();
            mJarvisJson.shrinkToFit();
            
            JSON json;
            File file = esp32FS.Open(JARVIS_PATH, "r", false);
            ret = json.Deserialize(file, &mJarvisJson);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO DESERIALIZE: %s", ret.c_str());
                file.close();
                return ret;
            }

            file.close();

            JsonObject op = mJarvisJson["cnt"]["op"][0].as<JsonObject>();
            mEthernetArray = mJarvisJson["cnt"]["eth"].as<JsonArray>();
            std::string serviceNetwork = op["snic"].as<std::string>();

            if (serviceNetwork == "lte")
            {
                JsonObject lte = mJarvisJson["cnt"]["catm1"][0].as<JsonObject>();
                std::string model = lte["md"].as<std::string>();
                std::string ctry = lte["ctry"].as<std::string>();
                Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
                Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
                Serial.print("+--------------------------------------------------------------+\r\n");
                Serial.print("|                    Service Network Settings                  |\r\n");
                Serial.print("+---------------------+------------+---------------------------+\r\n");
                Serial.print("|  Network Interface  |   Country  |   Modem                   |\r\n");
                Serial.print("+---------------------+------------+---------------------------+\r\n");
                Serial.print("|         LTE         | ");
                printCenteredText(ctry,12);
                Serial.print("| ");
                printCenteredText(model,27);
                Serial.print("|\r\n");
                Serial.print("+---------------------+------------+---------------------------+\r\n");

            }
            else if(serviceNetwork == "eth")
            {
                for (auto eth : mEthernetArray)
                {
                    JsonObject _eth = eth.as<JsonObject>();
                    bool snic = true;
                    /**
                     * @todo 1.4.0 이전 버전에는 eths 키가 없기 때문에 해당 로직을 추가해 두었습니다. 
                     *       키가 있다면 eth 인터페이스 위치를 확인하고 없다면 바로 사용합니다.
                     * 
                     */
                    if (_eth.containsKey("eths") == true)
                    {
                        snic = _eth["eths"] == static_cast<uint8_t>(jvs::if_e::EMBEDDED) ? true : false;
                    }
                    
                    
                    if (snic)
                    {
                        bool dhcp = eth["dhcp"].as<bool>();
                        Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
                        Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
                        Serial.print("+--------------------------------------------------------------+\r\n");
                        Serial.print("|                  Service Network Settings                    |\r\n");
                        Serial.print("+-----------------------+--------------------------------------+\r\n");

                        Serial.print("| Network Interface     | ");
                        Serial.print("Ethernet                             |\r\n");
                        Serial.print("+-----------------------+--------------------------------------+\r\n");

                        Serial.print("| Use Static IP         | ");
                        Serial.print(dhcp == false ? "Enabled                              " : "Disabled (DHCP)                      ");
                        Serial.print("|\r\n");
                        Serial.print("+-----------------------+--------------------------------------+\r\n");

                        if (dhcp == false)
                        {
                            std::string ip = eth["ip"].as<std::string>();
                            std::string gateway = eth["gtw"].as<std::string>();
                            std::string subnet = eth["snm"].as<std::string>();
                            std::string dns1 = eth["dns1"].as<std::string>();
                            std::string dns2 = eth["dns2"].as<std::string>();
                        
                            Serial.print("| IP                    | ");
                            printLeftAlignedText(ip,37);
                            Serial.print(" |\r\n");
                            Serial.print("+-----------------------+--------------------------------------+\r\n");
                            
                            Serial.print("| GateWay               | ");
                            printLeftAlignedText(gateway,37);
                            Serial.print(" |\r\n");
                            Serial.print("+-----------------------+--------------------------------------+\r\n");

                            Serial.print("| SubnetMask            | ");
                            printLeftAlignedText(subnet,37);
                            Serial.print(" |\r\n");
                            Serial.print("+-----------------------+--------------------------------------+\r\n");

                            Serial.print("| DNS1                  | ");
                            printLeftAlignedText(dns1,37);
                            Serial.print(" |\r\n");
                            Serial.print("+-----------------------+--------------------------------------+\r\n");

                            Serial.print("| DNS2                  | ");
                            printLeftAlignedText(dns2,37);
                            Serial.print(" |\r\n");
                            Serial.print("+-----------------------+--------------------------------------+\r\n");
                        }
                    }
                }
            }
            
        }
        
        Serial.print("\r\n\r\n[1] LTE");
        Serial.print("\r\n[2] Ethernet");
        Serial.print("\r\n[9] Exit\r\n");
        Serial.print("\r\n\r\nSelect an option [1 - 9]\r\n");
        std::string settingStr;
        while (true)
        {
            delay(80);
            settingStr = getSerialInput();

            if (settingStr == "1")
            {
                return configureLTE();
            }
            else if (settingStr == "2")
            {
                return configureEthernet();
            }
            else if (settingStr == "9")
            {
                Serial.print("\r\nService network has not been changed. Exiting settings\r\n\r\n");
                return Status(Status::Code::GOOD);
            }
            else 
            {
                Serial.print("\r\nInvalid input. Select an option [1 - 9]\r\n");
            }
        }
    }

    std::string CommandLineInterface::getSerialInput()
    {
        std::string input = ""; // 입력된 문자열을 저장할 변수
        char incomingChar;

        while (true) 
        {
            if (Serial.available() > 0) 
            {
                incomingChar = Serial.read();

                // 엔터 키가 눌리면 입력 종료
                if (incomingChar == '\n' || incomingChar == '\r') 
                {
                    if (!input.empty()) 
                    {
                        Serial.print("\r\n"); // 새 줄로 이동
                        break;
                    }
                }
                // 백스페이스 처리
                else if (incomingChar == 8 || incomingChar == 127) 
                {
                    if (!input.empty()) 
                    {
                        input.pop_back(); // 마지막 문자 삭제
                        Serial.print("\b \b"); // 시리얼 모니터에서 문자 지움
                    }
                }
                // 유효한 문자 입력
                else 
                {
                    input += incomingChar; // 입력된 문자 추가
                    Serial.print(incomingChar); // 입력된 문자 시리얼 모니터에 출력
                }
            }
        }
        return input;
    }

    Status CommandLineInterface::configureLTE()
    {
        Serial.print("\r\nService network has been set to LTE\r\n");
        JsonObject op = mJarvisJson["cnt"]["op"][0].as<JsonObject>();
        op["snic"] = "lte";
        JsonObject cnt = mJarvisJson["cnt"].as<JsonObject>();
        JsonArray catm1 = cnt["catm1"].to<JsonArray>();
        JsonObject _catm1 = catm1.add<JsonObject>();
        _catm1["md"]    = "LM5";
        _catm1["ctry"]  = "KR";

        JsonArray mc = mJarvisJson["cnt"]["mc"].as<JsonArray>();
        JsonArray mbTCP = mJarvisJson["cnt"]["mbtcp"].as<JsonArray>();
        bool isEthEnabled = false;
        for (auto obj : mc)
        {
            JsonObject _mc = obj.as<JsonObject>();
            if (_mc["eths"] == static_cast<uint8_t>(jvs::if_e::EMBEDDED))
            {
                isEthEnabled = true;
            }
        }

        for (auto obj : mbTCP)
        {
            JsonObject _mbTCP = obj.as<JsonObject>();
            if (_mbTCP["eths"] == static_cast<uint8_t>(jvs::if_e::EMBEDDED))
            {
                isEthEnabled = true;
            }
        }
        
        if (isEthEnabled == false)
        {
            for (int i = mEthernetArray.size() - 1; i >= 0; i--) 
            {
                JsonObject obj = mEthernetArray[i];
                if (obj["eths"] == 0) 
                {
                    mEthernetArray.remove(i);
                }
            }
            
        }
        
        
        return saveJarvisJson();
    }

    Status CommandLineInterface::configureEthernet()
    {
        Serial.print("\r\nService network has been set to Ethernet\r\n");
        JsonObject cnt = mJarvisJson["cnt"].as<JsonObject>();
        cnt.remove("catm1");
        cnt["catm1"].to<JsonArray>();

        JsonObject op = cnt["op"][0].as<JsonObject>();
        op["snic"] = "eth";

        JsonObject _eth;
        bool isEthEnabled = false;

        if (mEthernetArray.isNull())
        {
            mEthernetArray = cnt["eth"].to<JsonArray>();
            _eth = mEthernetArray.add<JsonObject>();
            _eth["eths"] = 0;
            isEthEnabled = true;
        }

        for (auto eth : mEthernetArray)
        {
            JsonObject ethObj = eth.as<JsonObject>();
            if (ethObj["eths"] == static_cast<uint8_t>(jvs::if_e::EMBEDDED))
            {
                _eth = ethObj;
                isEthEnabled = true;
            }
        }
        
        if(isEthEnabled == false)
        {
            _eth = mEthernetArray.add<JsonObject>();
            _eth["eths"] = 0;
        }

        std::string settingStr;
        while (true)
        {
            Serial.print("\r\nEnter whether to enable DHCP [Y/N]\r\n");
            delay(80);
            settingStr = getSerialInput();

            if (settingStr == "Y" || settingStr == "y")
            {
                Serial.print("\r\nDHCP is enabled.\r\n");
                _eth["dhcp"] = true;
                _eth["ip"]   = nullptr;
                _eth["snm"]  = nullptr;
                _eth["gtw"]  = nullptr;
                _eth["dns1"] = nullptr;
                _eth["dns2"] = nullptr;
                
                return saveJarvisJson();
            }
            else if (settingStr == "N" || settingStr == "n")
            {
                _eth["dhcp"] = false;
                std::string staticIP      = _eth["ip"].as<std::string>();
                std::string gateway = _eth["gtw"].as<std::string>();
                std::string subnet  = _eth["snm"].as<std::string>();
                std::string dns1    = _eth["dns1"].as<std::string>();
                std::string dns2    = _eth["dns2"].as<std::string>();

                while (true)
                {
                    Serial.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
                    Serial.print("+--------------------------------------------------------------+\r\n");
                    Serial.print("|                  Service Network Settings                    |\r\n");
                    Serial.print("+-----------------------+--------------------------------------+\r\n");

                    Serial.print("|  Network Interface    | ");
                    Serial.print("Ethernet                             |\r\n");
                    Serial.print("+-----------------------+--------------------------------------+\r\n");

                    Serial.print("|  Use Static IP        | ");
                    Serial.print("Enabled                              ");
                    Serial.print("|\r\n");
                    Serial.print("+-----------------------+--------------------------------------+\r\n");
                
                    Serial.print("|  [1] IP               | ");
                    printLeftAlignedText(staticIP,37);
                    Serial.print(" |\r\n");
                    Serial.print("+-----------------------+--------------------------------------+\r\n");
                    
                    Serial.print("|  [2] GateWay          | ");
                    printLeftAlignedText(gateway,37);
                    Serial.print(" |\r\n");
                    Serial.print("+-----------------------+--------------------------------------+\r\n");

                    Serial.print("|  [3] SubnetMask       | ");
                    printLeftAlignedText(subnet,37);
                    Serial.print(" |\r\n");
                    Serial.print("+-----------------------+--------------------------------------+\r\n");

                    Serial.print("|  [4] DNS1             | ");
                    printLeftAlignedText(dns1,37);
                    Serial.print(" |\r\n");
                    Serial.print("+-----------------------+--------------------------------------+\r\n");

                    Serial.print("|  [5] DNS2             | ");
                    printLeftAlignedText(dns2,37);
                    Serial.print(" |\r\n");
                    Serial.print("+--------------------------------------------------------------+\r\n");
                    Serial.print("|  [6] >>>  Save and Exit  <<<                                 |\r\n");
                    Serial.print("|  [9] Exit                                                    |\r\n");
                    Serial.print("+--------------------------------------------------------------+\r\n");
                   
                    Serial.print("\r\nSelect an option [1 - 9]\r\n");
            
                    delay(80);
                    settingStr = getSerialInput();

                    uint16_t inputCommand = 0;
                    try 
                    {
                        inputCommand = std::stoul(settingStr);
                    } 
                    catch (const std::invalid_argument& e) 
                    {
                        Serial.print("\r\nInvalid input (non-numeric). Enter a number between [1 - 9]\r\n");
                        continue;
                    } 
                    catch (const std::out_of_range& e) 
                    {
                        Serial.print("\r\nInput is out of range. Enter a number between [1 - 9]\r\n");
                        continue;
                    }

                    switch (inputCommand)
                    {
                    case 1:
                    {
                        Serial.print("\r\nEnter a static IP address (e.g. 192.168.1.100)\r\n");
                        while (true)
                        {
                            staticIP = getSerialInput();
                            if(isValidIpFormat(staticIP))
                            {
                                Serial.printf("\r\nConfigured IP: %s \r\n", staticIP.c_str());
                                break;
                            }
                            else
                            {
                                Serial.print("\r\nInvalid IP address Enter again \r\n");
                            }
                        }
                        break;
                    }   
                    case 2:
                    {
                        Serial.print("\r\nEnter a gateway address (e.g. 192.168.1.1)\r\n");
                        while (true)
                        {
                            gateway = getSerialInput();
                            if(isValidIpFormat(gateway))
                            {
                                Serial.printf("\r\nConfigured Gateway: %s \r\n", gateway.c_str());
                                _eth["gtw"] = gateway.c_str();
                                break;
                            }
                            else
                            {
                                Serial.print("\r\nInvalid gateway address Enter again \r\n");
                            }
                        }
                        break;
                    }   
                    case 3:
                    {
                        Serial.print("\r\nEnter a subnet mask (e.g. 255.255.255.0)\r\n");
                        while (true)
                        {
                            subnet = getSerialInput();
                            if(isValidIpFormat(subnet, true))
                            {
                                Serial.printf("\r\nConfigured Subnet Mask: %s \r\n", subnet.c_str());
                                // 
                                break;
                            }
                            else
                            {
                                Serial.print("\r\nInvalid subnet mask Enter again \r\n");
                            }
                        }
                        break;
                    }   
                    case 4:
                    {
                        Serial.print("\r\nEnter DNS1 server address (e.g. 8.8.8.8)\r\n");
                        while (true)
                        {
                            dns1 = getSerialInput();
                            if(isValidIpFormat(dns1))
                            {
                                Serial.printf("\r\nConfigured DNS1 Server: %s \r\n", dns1.c_str());
                                _eth["dns1"] = dns1.c_str();
                                break;
                            }
                            else
                            {
                                Serial.print("\r\nInvalid DNS1 server address Enter again \r\n");
                            }
                        }
                        break;
                    }   
                    case 5:
                    {
                        Serial.print("\r\nEnter DNS2 server address (e.g. 8.8.8.8)\r\n");
                        while (true)
                        {
                            dns2 = getSerialInput();
                            if(isValidIpFormat(dns2))
                            {
                                Serial.printf("\r\nConfigured DNS2 Server: %s \r\n", dns2.c_str());
                                break;
                            }
                            else
                            {
                                Serial.print("\r\nInvalid DNS1 server address Enter again \r\n");
                            }
                        }
                        break;
                    }   
                    case 6:
                    {
                        if (staticIP == "null")
                        {
                            Serial.print("\r\nStatic IP address has not been set. Check your network settings");
                            break;
                        }

                        if (gateway == "null")
                        {
                            Serial.print("\r\nGateway IP address has not been set. Check your network settings");
                            break;
                        }

                        if (subnet == "null")
                        {
                            Serial.print("\r\nSubnet Mask has not been set. Check your network settings");
                            break;
                        }

                        if (dns1 == "null")
                        {
                            Serial.print("\r\nDNS1 has not been set. Check your network settings");
                            break;
                        }
                        
                        if (dns2 == "null")
                        {
                            Serial.print("\r\nDNS2 has not been set. Check your network settings");
                            break;
                        }

                        _eth["ip"]   = staticIP;
                        _eth["gtw"]  = gateway;
                        _eth["snm"]  = subnet;
                        _eth["dns1"] = dns1;
                        _eth["dns2"] = dns2;
                        Serial.print("\r\nEthernet settings have been saved \r\n\r\n\r\n");
                        return saveJarvisJson();
                    }  
                    case 9:
                    {
                        Serial.print("\r\nEthernet settings has not been changed. Exiting settings\r\n\r\n");
                        return Status(Status::Code::GOOD);
                    }   
                    default:
                        Serial.print("\r\nInvalid input. Enter a number between [1 - 9]\r\n");
                        break;
                    }
                }
            }
            else 
            {
                Serial.print("\r\nInvalid input. Enter [Y / N]\r\n");
            }
        }

        
    }

    Status CommandLineInterface::saveJarvisJson()
    {
        File file = esp32FS.Open(JARVIS_PATH, "w", true);
        serializeJson(mJarvisJson, file);
        file.close();

        return Status(Status::Code::GOOD);
    }

    bool CommandLineInterface::isValidIpFormat(const std::string& ip, const bool& isSubnetmask)
    {
        std::regex validationRegex;

        if (isSubnetmask == true)
        {
            validationRegex.assign("^((255|254|252|248|240|224|192|128|0)\\.){3}(255|254|252|248|240|224|192|128|0)$");
        }
        else
        {
            validationRegex.assign("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
        }

        if (std::regex_match(ip, validationRegex))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    void CommandLineInterface::printCenteredText(const std::string& info, const size_t length)
    {
        size_t buffSize = length;
        char buff[buffSize];

        size_t inputLen = info.length();
        size_t totalPadding = buffSize - 1 - inputLen; // 마지막 '\0' 고려
        size_t leftPadding = totalPadding / 2;
        size_t rightPadding = totalPadding - leftPadding;

        // 좌측 패딩 추가
        memset(buff, ' ', leftPadding);
        
        // 문자열 복사
        strncpy(buff + leftPadding, info.c_str(), inputLen);

        // 우측 패딩 추가
        memset(buff + leftPadding + inputLen, ' ', rightPadding);

        // NULL 문자 추가
        buff[buffSize - 1] = '\0';

        Serial.print(buff);
    }

    void CommandLineInterface::printLeftAlignedText(const std::string& info, const size_t length)
    {
        size_t buffSize = length;
        char buff[buffSize];

        size_t inputLen = info.length();
        strncpy(buff, info.c_str(), inputLen);
        for (int i = inputLen; i < buffSize - 1; i++) 
        {
            buff[i] = ' ';
        }
        buff[buffSize - 1] = '\0';

        Serial.print(buff);
    }

}