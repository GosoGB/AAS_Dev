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
                Serial.print("\rPress the Enter key to configure the network settings.");
                Serial.print(" ");
                Serial.print(countdown);
                Serial.print("  Seconds left.");

                countdown--;
                
                if (countdown < 0) 
                {
                    Serial.print("\r\nTime has expired. Proceeding with device execution.\r\n");
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
                return configureNetworkInterface();
            }
        }
    }

    Status CommandLineInterface::configureNetworkInterface()
    {
        logger.SetLevel(muffin::log_level_e::LOG_LEVEL_ERROR);
        esp32FS.Begin(false);
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
            cnt["melsec"].to<JsonArray>();

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
            Serial.print("+---------------------+------------+----------+\r\n");
            Serial.print("|         Current Network Information         |\r\n");
            Serial.print("+---------------------+------------+----------+\r\n");
            Serial.print("|  Network Interface  |   Country  |   Modem  |\r\n");
            Serial.print("+---------------------+------------+----------+\r\n");
            Serial.print("|         LTE         |     KR     |    LM5   |\r\n");
            Serial.print("+---------------------+------------+----------+\r\n");
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
                return ret;
            }
            LOG_INFO(logger, "Deserialized JARVIS config file");
            
            JsonObject op = mJarvisJson["cnt"]["op"][0].as<JsonObject>();
            std::string serviceNetwork = op["snic"].as<std::string>();

            if (serviceNetwork == "lte")
            {
                JsonObject lte = mJarvisJson["cnt"]["catm1"][0].as<JsonObject>();
                std::string model = lte["md"].as<std::string>();
                std::string ctry = lte["ctry"].as<std::string>();
                Serial.print("+---------------------+------------+----------+\r\n");
                Serial.print("|         Current Network Information         |\r\n");
                Serial.print("+---------------------+------------+----------+\r\n");
                Serial.print("|  Network Interface  |   Country  |   Modem  |\r\n");
                Serial.print("+---------------------+------------+----------+\r\n");
                Serial.print("|         LTE         | ");
                printCenteredText(ctry,12);
                Serial.print("| ");
                printCenteredText(model,10);
                Serial.print("|\r\n");
                Serial.print("+---------------------+------------+----------+\r\n");

            }
            else if(serviceNetwork == "eth")
            {
                JsonObject eth = mJarvisJson["cnt"]["eth"][0].as<JsonObject>();
                bool dhcp = eth["dhcp"].as<bool>();

                Serial.print("+-----------------------+---------------------+\r\n");
                Serial.print("|         Current Network Information         |\r\n");
                Serial.print("+-----------------------+---------------------+\r\n");

                Serial.print("| Network Interface     | ");
                Serial.print("Ethernet            |\r\n");
                Serial.print("+-----------------------+---------------------+\r\n");

                Serial.print("| Use Static IP         | ");
                Serial.print(dhcp == false ? "Enabled             " : "Disabled (DHCP)     ");
                Serial.print("|\r\n");
                Serial.print("+-----------------------+---------------------+\r\n");

                if (dhcp == false)
                {
                    std::string ip = eth["ip"].as<std::string>();
                    std::string gateway = eth["gtw"].as<std::string>();
                    std::string subnet = eth["snm"].as<std::string>();
                    std::string dns1 = eth["dns1"].as<std::string>();
                    std::string dns2 = eth["dns2"].as<std::string>();
                
                    Serial.print("| IP                    | ");
                    printLeftAlignedText(ip,20);
                    Serial.print(" |\r\n");
                    Serial.print("+-----------------------+---------------------+\r\n");

                    Serial.print("| GateWay               | ");
                    printLeftAlignedText(gateway,20);
                    Serial.print(" |\r\n");
                    Serial.print("+-----------------------+---------------------+\r\n");

                    Serial.print("| SubnetMask            | ");
                    printLeftAlignedText(subnet,20);
                    Serial.print(" |\r\n");
                    Serial.print("+-----------------------+---------------------+\r\n");

                    Serial.print("| DNS1                  | ");
                    printLeftAlignedText(dns1,20);
                    Serial.print(" |\r\n");
                    Serial.print("+-----------------------+---------------------+\r\n");

                    Serial.print("| DNS2                  | ");
                    printLeftAlignedText(dns2,20);
                    Serial.print(" |\r\n");
                    Serial.print("+-----------------------+---------------------+\r\n");
                }
            
            }
            
        }
        
        std::string settingStr;
        while (true)
        {
            Serial.print("\r\nPlease select a service network (1 or 2)\r\n");
            Serial.print("1. LTE \t 2. Ethernet\r\n");
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
            else 
            {
                Serial.print("\r\nInvalid input. Please enter 1 or 2.\r\n");
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
        Serial.print("\r\nService network has been set to LTE.\r\n");
        JsonObject op = mJarvisJson["cnt"]["op"][0].as<JsonObject>();
        op["snic"] = "lte";
        JsonObject cnt = mJarvisJson["cnt"].as<JsonObject>();
        JsonArray catm1 = cnt["catm1"].to<JsonArray>();
        JsonObject _catm1 = catm1.add<JsonObject>();
        _catm1["md"]    = "LM5";
        _catm1["ctry"]  = "KR";
        
        
        return saveJarvisJson();
    }

    Status CommandLineInterface::configureEthernet()
    {
        Serial.print("\r\nService network has been set to Ethernet.\r\n");
        JsonObject cnt = mJarvisJson["cnt"].as<JsonObject>();
        cnt.remove("catm1");
        cnt["catm1"].to<JsonArray>();

        JsonObject op = cnt["op"][0].as<JsonObject>();
        op["snic"] = "eth";

        cnt.remove("eth");
        JsonArray ethObj = cnt["eth"].to<JsonArray>();
        JsonObject _eth = ethObj.add<JsonObject>();

        std::string settingStr;

        while (true)
        {
            Serial.print("\r\nPlease enter whether to enable DHCP (Y/N)\r\n");
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
                std::string staticIP, subnetMask, gateway, dnsServer1, dnsServer2 ;

                Serial.print("\r\nPlease enter a static IP address (e.g. 192.168.1.100)\r\n");
                while (true)
                {
                    staticIP = getSerialInput();
                    if(isValidIpFormat(staticIP))
                    {
                        Serial.printf("\r\nConfigured IP: %s \r\n", staticIP.c_str());
                        _eth["ip"] = staticIP.c_str();
                        break;
                    }
                    else
                    {
                        Serial.print("\r\nInvalid IP address. Please enter again. \r\n");
                    }
                }

                Serial.print("\r\nPlease enter a subnet mask (e.g. 255.255.255.0)\r\n");
                while (true)
                {
                    subnetMask = getSerialInput();
                    if(isValidIpFormat(subnetMask, true))
                    {
                        Serial.printf("\r\nConfigured Subnet Mask: %s \r\n", subnetMask.c_str());
                        _eth["snm"] = subnetMask.c_str();
                        break;
                    }
                    else
                    {
                        Serial.print("\r\nInvalid subnet mask. Please enter again. \r\n");
                    }
                }

                Serial.print("\r\nPlease enter a gateway address (e.g. 192.168.1.1)\r\n");
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
                        Serial.print("\r\nInvalid gateway address. Please enter again. \r\n");
                    }
                }

                Serial.print("\r\nPlease enter DNS1 server address (e.g. 8.8.8.8)\r\n");
                while (true)
                {
                    dnsServer1 = getSerialInput();
                    if(isValidIpFormat(dnsServer1))
                    {
                        Serial.printf("\r\nConfigured DNS1 Server: %s \r\n", dnsServer1.c_str());
                        _eth["dns1"] = dnsServer1.c_str();
                        break;
                    }
                    else
                    {
                        Serial.print("\r\nInvalid DNS1 server address. Please enter again. \r\n");
                    }
                }

                Serial.print("\r\nPlease enter DNS2 server address (e.g. 8.8.8.8)\r\n");
                while (true)
                {
                    dnsServer2 = getSerialInput();
                    if(isValidIpFormat(dnsServer2))
                    {
                        Serial.printf("\r\nConfigured DNS2 Server: %s \r\n", dnsServer2.c_str());
                        _eth["dns2"] = dnsServer2.c_str();
                        break;
                    }
                    else
                    {
                        Serial.print("\r\nInvalid DNS2 server address. Please enter again. \r\n");
                    }
                }

                return saveJarvisJson();
            }
            else 
            {
                Serial.print("\r\nInvalid input. Please enter Y or N.\r\n");
            }
        }

        
    }

    Status CommandLineInterface::saveJarvisJson()
    {
        File file = esp32FS.Open(JARVIS_PATH, "w", true);
        serializeJson(mJarvisJson, file);
        file.close();

        Serial.print("\r\nAll settings have been saved. The device will now reboot. \r\n\r\n\r\n");

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