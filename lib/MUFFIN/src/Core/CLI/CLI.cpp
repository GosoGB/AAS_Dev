/**
 * @file CLI.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @brief 
 * @version 1.2.2
 * @date 2025-01-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */



#include <regex>

#include "HardwareSerial.h"
#include "CLI.h"
#include "Storage/ESP32FS/ESP32FS.h"
#include "Common/Logger/Logger.h"
#include "JARVIS/Include/TypeDefinitions.h"

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
                Serial.print("\r네트워크 설정을 원하시면 엔터 키를 입력하세요.");
                Serial.print(" ");
                Serial.print(countdown);
                Serial.print("초");

                countdown--;
                
                if (countdown < 0) 
                {
                    Serial.print("\r\n시간이 만료되었습니다. 디바이스를 실행 합니다.\r\n");
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
            doc["ver"] = "v2";

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
            Serial.print("+---------------------+------------+-------+\r\n");
            Serial.print("|             현재 네트워크 정보             |\r\n");
            Serial.print("+---------------------+------------+-------+\r\n");
            Serial.print("| 네트워크 인터페이스   | 사용 국가  |  모뎀  |\r\n");
            Serial.print("+---------------------+------------+-------+\r\n");
            Serial.print("| LTE                 | KR         | LM5   |\r\n");
            Serial.print("+---------------------+------------+-------+\r\n");
        }
        else
        {
            JSON json;
            File file = esp32FS.Open(JARVIS_PATH, "r", false);
            const size_t size = file.size();
            char buffer[size + 1] = {'\0'};
            for (size_t idx = 0; idx < size; ++idx)
            {
                buffer[idx] = file.read();
            }
            file.close();

            ret = json.Deserialize(buffer, &mJarvisJson);
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
                Serial.print("+---------------------+------------+-------+\r\n");
                Serial.print("|             현재 네트워크 정보           |\r\n");
                Serial.print("+---------------------+------------+-------+\r\n");
                Serial.print("| 네트워크 인터페이스  | 사용 국가 |  모뎀 |\r\n");
                Serial.print("+---------------------+------------+-------+\r\n");
                Serial.print("| LTE                 | ");
                Serial.print(ctry.c_str());
                Serial.print("         | ");
                Serial.print(model.c_str());
                Serial.print("   |\r\n");
                Serial.print("+---------------------+------------+-------+\r\n");

            }
            else if(serviceNetwork == "eth")
            {
                JsonObject eth = mJarvisJson["cnt"]["eth"][0].as<JsonObject>();
                bool dhcp = eth["dhcp"].as<bool>();

                Serial.print("+-----------------------+--------------------+\r\n");
                Serial.print("|              현재 네트워크 정보            |\r\n");
                Serial.print("+-----------------------+--------------------+\r\n");

                Serial.print("| 네트워크 인터페이스   | ");
                Serial.print("Ethernet");
                Serial.print("           |\r\n");
                Serial.print("+-----------------------+--------------------+\r\n");

                Serial.print("| 고정 IP 사용 여부     | ");
                Serial.print(dhcp == false ? "사용          " : "사용안함(DHCP)");
                Serial.print("     |\r\n");
                Serial.print("+-----------------------+--------------------+\r\n");

                if (dhcp == false)
                {
                    std::string ip = eth["ip"].as<std::string>();
                    std::string gateway = eth["gtw"].as<std::string>();
                    std::string subnet = eth["snm"].as<std::string>();
                    std::string dns1 = eth["dns1"].as<std::string>();
                    std::string dns2 = eth["dns2"].as<std::string>();
                
                    Serial.print("| IP                    | ");
                    printNetworkInfo(ip);
                    Serial.print("|\r\n");
                    Serial.print("+-----------------------+--------------------+\r\n");

                    Serial.print("| GateWay               | ");
                    printNetworkInfo(gateway);
                    Serial.print("|\r\n");
                    Serial.print("+-----------------------+--------------------+\r\n");

                    Serial.print("| SubnetMask            | ");
                    printNetworkInfo(subnet);
                    Serial.print("|\r\n");
                    Serial.print("+-----------------------+--------------------+\r\n");

                    Serial.print("| DNS1                  | ");
                    printNetworkInfo(dns1);
                    Serial.print("|\r\n");
                    Serial.print("+-----------------------+--------------------+\r\n");

                    Serial.print("| DNS2                  | ");
                    printNetworkInfo(dns2);
                    Serial.print("|\r\n");
                    Serial.print("+-----------------------+--------------------+\r\n");
                }
            
            }
            
        }
        
        std::string settingStr;
        while (true)
        {
            Serial.print("\r\n서비스 네트워크를 선택해 주세요. (1 또는 2)\r\n");
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
                Serial.print("잘못된 입력입니다. 1 또는 2를 입력해주세요.\r\n");
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
        Serial.print("서비스 네트워크는 LTE로 설정 되었습니다.\r\n");
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
        Serial.print("서비스 네트워크는 Ethernet으로 설정 되었습니다.\r\n");
        JsonObject cnt = mJarvisJson["cnt"].as<JsonObject>();
        cnt.remove("catm1");
        cnt["catm1"].to<JsonArray>();

        JsonObject op = cnt["op"][0].as<JsonObject>();
        op["snic"] = "eth";
        JsonArray ethObj = cnt["eth"].to<JsonArray>();
        JsonObject _eth = ethObj.add<JsonObject>();

        std::string settingStr;

        while (true)
        {
            Serial.print("DHCP 설정 여부를 입력해주세요 (Y/N)\r\n");
            delay(80);
            settingStr = getSerialInput();

            if (settingStr == "Y" || settingStr == "y")
            {
                Serial.print("DHCP로 설정합니다.\r\n");
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

                Serial.print("고정 IP를 입력해주세요 (예: 192.168.1.100)\r\n");
                while (true)
                {
                    staticIP = getSerialInput();
                    if(isValidIpFormat(staticIP))
                    {
                        Serial.printf("설정 된 IP : %s \r\n",staticIP.c_str());
                        _eth["ip"] = staticIP.c_str();
                        break;
                    }
                    else
                    {
                        Serial.print("유효하지 않은 IP 정보입니다. 다시 입력해 주세요 \r\n");
                    }
                }

                Serial.print("\r\n서브넷 마스크를 입력해주세요 (예: 255.255.255.0)\r\n");
                while (true)
                {
                    subnetMask = getSerialInput();
                    if(isValidIpFormat(subnetMask, true))
                    {
                        Serial.printf("설정 된 서브넷 마스크 : %s \r\n",subnetMask.c_str());
                        _eth["snm"] = subnetMask.c_str();
                        break;
                    }
                    else
                    {
                        Serial.print("유효하지 않은 서브넷 마스크 정보입니다. 다시 입력해 주세요 \r\n");
                    }
                }

                Serial.print("\r\n게이트웨이를 입력해주세요 (예: 192.168.1.1)\r\n");
                while (true)
                {
                    gateway = getSerialInput();
                    if(isValidIpFormat(gateway))
                    {
                        Serial.printf("설정 된 게이트웨이 : %s \r\n",gateway.c_str());
                        _eth["gtw"] = gateway.c_str();
                        break;
                    }
                    else
                    {
                        Serial.print("유효하지 않은 게이트웨이 정보입니다. 다시 입력해 주세요 \r\n");
                    }
                }

                Serial.print("\r\nDNS1 서버를 입력해주세요 (예: 8.8.8.8)\r\n");
                while (true)
                {
                    dnsServer1 = getSerialInput();
                    if(isValidIpFormat(dnsServer1))
                    {
                        Serial.printf("설정 된 DNS1 서버 : %s \r\n",dnsServer1.c_str());
                        _eth["dns1"] = dnsServer1.c_str();
                        break;
                    }
                    else
                    {
                        Serial.print("유효하지 않은 DNS1 서버 정보입니다. 다시 입력해 주세요 \r\n");
                    }
                }

                Serial.print("\r\nDNS2 서버를 입력해주세요 (예: 8.0.0.8)\r\n");
                while (true)
                {
                    dnsServer2 = getSerialInput();
                    if(isValidIpFormat(dnsServer2))
                    {
                        Serial.printf("설정 된 DNS2 서버 : %s \r\n",dnsServer2.c_str());
                        _eth["dns2"] = dnsServer2.c_str();
                        break;
                    }
                    else
                    {
                        Serial.print("유효하지 않은 DNS2 서버 정보입니다. 다시 입력해 주세요 \r\n");
                    }
                }

                return saveJarvisJson();
            }
            else 
            {
                Serial.print("잘못된 입력입니다. Y 또는 N을 입력해주세요.\r\n");
            }
        }

        
    }

    Status CommandLineInterface::saveJarvisJson()
    {
        const size_t size = measureJson(mJarvisJson) + 1;
        char buffer[size] = {'\0'};
        serializeJson(mJarvisJson, buffer, size);

        Serial.println(buffer);

        File file = esp32FS.Open(JARVIS_PATH, "w", true);
        if (file == false)
        {
            LOG_ERROR(logger, "FAILED TO OPEN JARVIS DIRECTORY");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        file.write(reinterpret_cast<uint8_t*>(buffer), size);
        file.flush();
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

    void CommandLineInterface::printNetworkInfo(const std::string& info)
    {
        size_t buffSize = 20;
        char buff[20];

        int inputLen = info.length();
        strncpy(buff, info.c_str(), inputLen);
        for (int i = inputLen; i < buffSize - 1; i++) 
        {
            buff[i] = ' ';
        }
        buff[buffSize - 1] = '\0';

        Serial.print(buff);
    }

}