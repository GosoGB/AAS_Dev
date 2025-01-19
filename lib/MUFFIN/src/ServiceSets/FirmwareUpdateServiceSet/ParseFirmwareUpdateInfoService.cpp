/**
 * @file ParseFirmwareUpdateInfoService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 펌웨어 업데이트 정보를 파싱하는 서비스를 정의합니다.
 * 
 * @date 2025-01-20
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <ArduinoJson.h>

#include "Common/Assert.h"
#include "Common/Convert/ConvertClass.h"
#include "Common/Logger/Logger.h"
#include "DataFormat/JSON/JSON.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "ParseFirmwareUpdateInfoService.h"



namespace muffin {

    Status validateFormat(const char* payload, JsonDocument* output)
    {
        JSON json;
        Status ret = json.Deserialize(payload, output);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE FIRMWARE INFO: %s", ret.c_str());
            return ret;
        }

        bool isValid = true;
        isValid &= output->containsKey("mac");
        isValid &= output->containsKey("deviceType");
        isValid &= output->containsKey("url");
        isValid &= output->containsKey("otaId");
        isValid &= output->containsKey("mcu1");
        isValid &= output->containsKey("mcu2");
        if (isValid == false)
        {
            LOG_ERROR(logger, "MANDATORY KEYS CANNOT BE MISSING");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }
    
        if ((*output)["mac"].isNull() == true)
        {
            LOG_ERROR(logger, "MAC ADDRESS CANNOT BE NULL OR EMPTY");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }
        else if (strcmp((*output)["mac"].as<const char*>(), macAddress.GetEthernet()) != 0)
        {
            LOG_ERROR(logger, "THE MAC ADDRESS DOES NOT MATCH THAT OF THE DEVICE");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }

        isValid &= (*output)["deviceType"].isNull() == false;
        isValid &= (*output)["otaId"].isNull() == false;
        isValid &= (*output)["url"].isNull() == false;
        if (isValid == false)
        {
            LOG_ERROR(logger, "MANDATORY KEY'S VALUE CANNOT BE NULL");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }

    #if defined(MODLINK_L)
        if (strcmp((*output)["deviceType"].as<const char*>(), "MODLINK-L") != 0)
        {
            LOG_ERROR(logger, "THE DEVICE TYPE DOES NOT MATCH THAT OF THE DEVICE");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }
    #elif defined(MODLINK_T2)
        if (strcmp((*output)["deviceType"].as<const char*>(), "MODLINK-T2") != 0)
        {
            LOG_ERROR(logger, "THE DEVICE TYPE DOES NOT MATCH THAT OF THE DEVICE");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }
    #endif

        if ((*output)["otaId"].is<uint32_t>() == false)
        {
            LOG_ERROR(logger, "INVALID OTA ID: %s", (*output)["otaId"].as<const char*>());
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }
        
        const char* url = (*output)["url"].as<const char*>();
        if (strlen(url) > sizeof(ota::url_t::Host))
        {
            LOG_ERROR(logger, "[OUT OF RANGE] HOST URL MUST BE SHORTER THAN 64");
            return Status(Status::Code::BAD_REQUEST_TOO_LARGE);
        }

        if ((((*output)["mcu1"].isNull() == false) && ((*output)["mcu1"].is<JsonObject>() == false)) ||
            (((*output)["mcu2"].isNull() == false) && ((*output)["mcu2"].is<JsonObject>() == false)))
        {
            LOG_ERROR(logger, "FIRMWARE INFO MUST BE A JSON OBJECT");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }
    }

    http_scheme_e parseHttpScheme(const char* url)
    {
        if (strncmp(url, "http://", 7) == 0)
        {
            return http_scheme_e::HTTP;
        }
        else if (strncmp(url, "https://", 8) == 0)
        {
            return http_scheme_e::HTTPS;
        }
        else
        {
            LOG_ERROR(logger, "INVALID SCHEME: %s", url);
            return http_scheme_e::INVALID;
        }
    }

    Status parseDownloadURL(const char* url, ota::fw_info_t* esp32)
    {
        esp32->Head.DownloadURL.Scheme = parseHttpScheme(url);
        if (esp32->Head.DownloadURL.Scheme == http_scheme_e::INVALID)
        {
            LOG_ERROR(logger, "INVALID HTTP PROTOCOL SCHEME");
            return Status(Status::Code::BAD_PROTOCOL_VERSION_UNSUPPORTED);
        }
        const size_t posHost = esp32->Head.DownloadURL.Scheme == http_scheme_e::HTTP ? 7 : 8;

        const char* ptr = url + posHost;
        bool hasPortDelimiter = false;
        size_t posPort = posHost;

        while (ptr != nullptr)
        {
            if (*ptr == ':')
            {
                hasPortDelimiter = true;
                ++posPort;
                break;
            }
            esp32->Head.DownloadURL.Host[posPort++ - posHost] = *ptr++;
        }
        
        if (hasPortDelimiter == false)
        {
            LOG_ERROR(logger, "PORT DELIMITER NOT FOUND");
            return Status(Status::Code::BAD_SERVER_URI_INVALID);
        }

        esp32->Head.DownloadURL.Port = Convert.ToUInt16(url + posPort);
        if (esp32->Head.DownloadURL.Port == UINT16_MAX)
        {
            LOG_ERROR(logger, "FAILED TO PARSE SERVER PORT INFO");
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        LOG_INFO(logger, "Host: %s, Port: %u", esp32->Head.DownloadURL.Host, esp32->Head.DownloadURL.Port);
    }

    Status parseFirmwareInfo(JsonObject json, ota::fw_info_t* output)
    {
        memset(output, 0, sizeof(ota::fw_info_t));

        output->Head.HasNewFirmware = true;
        output->Head.VersionCode = json["vc"].as<uint16_t>();
        strncpy(output->Head.SemanticVersion, json["version"].as<const char*>(), sizeof(ota::fw_head_t::SemanticVersion));
        output->Size.Total = json["fileTotalSize"].as<uint64_t>();
        strncpy(output->Checksum.Total, json["checksum"].as<const char*>(), sizeof(ota::fw_cks_t::Total));

        LOG_INFO(logger, "Semantic Version: %s", output->Head.SemanticVersion);
        LOG_INFO(logger, "Version Code: %u", output->Head.VersionCode);
        LOG_INFO(logger, "Total File Size: %u", output->Size.Total);
        LOG_INFO(logger, "CRC32 Checksum: %s", output->Checksum.Total);

        JsonArray arrayFileNumber     = json["fileNo"].as<JsonArray>();
        JsonArray arrayFilePath       = json["filePath"].as<JsonArray>();
        JsonArray arrayFileSize       = json["fileSize"].as<JsonArray>();
        JsonArray arrayFileChecksum   = json["fileChecksum"].as<JsonArray>();
        if ((arrayFileNumber.size() != arrayFilePath.size()) ||
            (arrayFileNumber.size() != arrayFileSize.size()) ||
            (arrayFileNumber.size() != arrayFileChecksum.size()))
        {
            LOG_ERROR(logger, "THE NUMBERS OF CHUNK ARRAYS DO NOT MATCH");
            return Status(Status::Code::BAD_SEQUENCE_NUMBER_INVALID);
        }
        output->Chunk.Count = arrayFileNumber.size();

        for (uint8_t idx = 0; idx < output->Chunk.Count; ++idx)
        {
            output->Chunk.IndexArray[idx] = arrayFileNumber[idx].as<uint8_t>();
        }

        for (uint8_t idx = 0; idx < output->Chunk.Count; ++idx)
        {
            strncpy(
                output->Chunk.PathArray[idx],
                arrayFilePath[idx].as<const char*>(),
                sizeof(ota::chk_head_t::PathArray[0])
            );
        }
        
        for (uint8_t idx = 0; idx < output->Chunk.Count; ++idx)
        {
            output->Size.Array[idx] = arrayFileSize[idx].as<uint32_t>();
        }

        for (uint8_t idx = 0; idx < output->Chunk.Count; ++idx)
        {
            strncpy(
                output->Checksum.Array[idx],
                arrayFileChecksum[idx].as<const char*>(),
                sizeof(ota::fw_cks_t::Array[0])
            );
        }

        return Status(Status::Code::GOOD);
    }

    Status strategyESP32(JsonDocument& json, ota::fw_info_t* output)
    {
        if (json["mcu1"].isNull() == true)
        {
            LOG_INFO(logger, "No firmware update available for ESP32");
            output->Head.HasNewFirmware = false;
            return Status(Status::Code::GOOD);
        }

        JsonObject obj = json["mcu1"].as<JsonObject>();
        return parseFirmwareInfo(obj, output);
    }

    Status strategyMEGA2560(JsonDocument& json, ota::fw_info_t* output)
    {
        if (json["mcu2"].isNull() == true)
        {
            LOG_INFO(logger, "No firmware update available for ATmega2560");
            return Status(Status::Code::GOOD);
        }

        JsonObject obj = json["mcu2"].as<JsonObject>();
        return parseFirmwareInfo(obj, output);
    }

    Status ParseFirmwareUpdateInfoService(const char* payload, ota::fw_info_t* esp32, ota::fw_info_t* mega2560)
    {
        JsonDocument doc;
        Status ret = validateFormat(payload, &doc);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO PARSE FIRMWARE UPDATE INFO: %s", ret.c_str());
            return ret;
        }

        esp32->Head.ID = doc["otaId"].as<uint32_t>();
        ret = parseDownloadURL(doc["url"].as<const char*>(), esp32);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO PARSE DOWNLOAD URL: %s", ret.c_str());
            return ret;
        }

        ret = strategyESP32(doc, esp32);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO PARSE INFO FOR ESP32");
            return ret;
        }

        ret = strategyMEGA2560(doc, mega2560);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO PARSE INFO FOR ATmega2560");
            return ret;
        }
        
        return Status(Status::Code::GOOD);
    }
}