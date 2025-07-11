#if defined(MT11)

#include "cip_util.h"
#include "cip_types.h"

using muffin::w5500::EthernetClient;

void printHex(const std::vector<uint8_t>& data) {
    for (uint8_t b : data) {
        if (b < 16) Serial.print("0");
        Serial.print(b, HEX);
        Serial.print(" ");
    }
    Serial.println();
}

// CIP 데이터 값을 DataType에 따라 출력
void printCipData(const cip_data_t& d)
{
    switch (d.DataType)
    {
        case CipDataType::BOOL:
            Serial.printf("BOOL (Boolean 1-bit in 1 byte) = %d\n", d.Value.BOOL);
            break;
        case CipDataType::SINT:
            Serial.printf("SINT (Signed 8-bit integer) = %d\n", d.Value.SINT);
            break;
        case CipDataType::USINT:
            Serial.printf("USINT (Unsigned 8-bit integer) = %u\n", d.Value.USINT);
            break;
        case CipDataType::BYTE:
            Serial.printf("BYTE (Bit string 8 bits) = 0x%02X\n", d.Value.USINT);
            break;
        case CipDataType::INT:
            Serial.printf("INT (Signed 16-bit integer) = %d\n", d.Value.INT);
            break;
        case CipDataType::UINT:
            Serial.printf("UINT (Unsigned 16-bit integer) = %u\n", d.Value.UINT);
            break;
        case CipDataType::WORD:
            Serial.printf("WORD (Bit string 16 bits) = 0x%04X\n", d.Value.UINT);
            break;
        case CipDataType::DINT:
            Serial.printf("DINT (Signed 32-bit integer) = %d\n", d.Value.DINT);
            break;
        case CipDataType::UDINT:
            Serial.printf("UDINT (Unsigned 32-bit integer) = %u\n", d.Value.UDINT);
            break;
        case CipDataType::DWORD:
            Serial.printf("DWORD (Bit string 32 bits) = 0x%08X\n", d.Value.UDINT);
            break;
        case CipDataType::REAL:
            Serial.printf("REAL (32-bit float IEEE754) = %f\n", d.Value.REAL);
            break;
        case CipDataType::LINT:
            Serial.printf("LINT (Signed 64-bit integer) = %lld\n", (long long)d.Value.LINT);
            break;
        case CipDataType::ULINT:
            Serial.printf("ULINT (Unsigned 64-bit integer) = %llu\n", (unsigned long long)d.Value.ULINT);
            break;
        case CipDataType::LREAL:
            Serial.printf("LREAL (64-bit float IEEE754) = %lf\n", d.Value.LREAL);
            break;
        case CipDataType::LWORD:
            Serial.printf("LWORD (Bit string 64 bits) = N/A\n");
            break;
        case CipDataType::STRING:
            Serial.printf("STRING (Length=%zu) = \"%.*s\"\n",
                          d.Value.STRING.Length,
                          (int)d.Value.STRING.Length,
                          d.Value.STRING.Data);
            break;
        default:
            Serial.printf("Unknown Type = 0x%04X\n", static_cast<uint16_t>(d.DataType));
            break;
    }
}


// 1756-pm020_-en-p.pdf Page 19
// 코드표 참조 : https://support.ptc.com/help/kepware/drivers/en/index.html#page/kepware/drivers/OMRONNJETHERNET/CIP_Error_Codes.html 
std::string getCIPStatusDescription(uint8_t statusCode) {
    switch (statusCode) {
        case 0x00: return "Success";
        case 0x01: return "Connection-related service failed along the connection path";
        case 0x02: return "Resource unavailable";
        case 0x03: return "Invalid parameter value";
        case 0x04: return "Path segment error. Tag does not exist in the device";
        case 0x05: return "Path destination unknown. Structure member does not exist or array element is out of range";
        case 0x06: return "Partial transfer; only part of the expected data was transferred";
        case 0x07: return "Loss of connection";
        case 0x08: return "Service not supported";
        case 0x09: return "Invalid attribute value";
        case 0x0A: return "Attribute list error";

        case 0x0B: return "Object cannot perform the requested service in its current mode/state";
        case 0x0C: return "Object cannot perform service in current state";
        case 0x0D: return "Requested instance of object to be created already exists";
        case 0x0E: return "A request to modify a non-editable attribute was received";
        case 0x0F: return "A permission / privilege check failed";
        case 0x10: return "The device’s current mode/state prohibits the execution of the requested service";
        case 0x11: return "Reply data too large";
        case 0x12: return "The service specified an operation that would fragment a primitive data value";
        case 0x13: return "Not enough data";
        case 0x14: return "Attribute not supported";
        case 0x15: return "Too much data. The service supplied more data than expected";
        case 0x16: return "The object specified does not exist in the device";
        case 0x17: return "The fragmentation sequence for this service is not currently active for this data";
        case 0x18: return "The attribute data of this object was not saved prior to the requested service";
        case 0x19: return "The attribute data of this object was not saved due to a failure during the attempt";
        case 0x1A: return "Routing failure; request packet too large";
        case 0x1B: return "Routing failure; response packet too large";
        case 0x1C: return "Missing attribute in list entry data";
        case 0x1D: return "Invalid attribute value list";
        case 0x1E: return "Embedded service error. One or more services returned an error within a multiple-service packet service";
        case 0x1F: return "Vendor-specific error. Consult vendor documentation. See Also: 0x1F Extended Error Codes";
        case 0x20: return "Invalid parameter. Parameter does not meet the requirements of the CIP specification or Omron specification. See Also: 0x20 Extended Error Codes";
        case 0x21: return "An attempt was made to write to a write-once medium that has already been written";
        case 0x22: return "Invalid reply received. Reply service code does not match the request service code or reply message is shorter than the minimum expected reply size";
        case 0x23: return "The message received is larger than the receiving buffer can handle";
        case 0x24: return "The format of the received message is not supported by the server";
        case 0x25: return "The key segment included as the first segment in the path does not match the destination module";
        case 0x26: return "The size of the path sent with the service request is not large enough to allow the request to be routed to an object or too much routing data was included";
        case 0x27: return "Unexpected attribute in list.";
        case 0x28: return "The member ID specified in the request does not exist in the specified class, instance, or attribute";
        case 0x29: return "A request to modify a non-modifiable member was received";
        case 0x2A: return "DeviceNet-specific error";
        case 0x2B: return "A CIP to Modbus translator received an unknown Modbus exception code";
        case 0x2C: return "A request to read a non-readable attribute was received";
        case 0x2D: return "A requested object instance cannot be deleted";
        case 0x2E: return "The object supports the service, but not for the designated application path (for example, attribute)";
        // 0x2C ~ 0xCF Reserved by CIP.
        // 0xCF ~ 0xFF Object class specific errors.
        default:
            return "Unknown error";
    }
}

// 1756-pm020_-en-p.pdf Page 13
int cipDataTypeSize(CipDataType type) {
    switch (type) {
        // 1-byte types
        case CipDataType::BOOL:
        case CipDataType::SINT:
        case CipDataType::USINT:
        case CipDataType::BYTE:  // N/A지만 1바이트
            return 1;

        // 2-byte types
        case CipDataType::INT:
        case CipDataType::UINT:
        case CipDataType::WORD:  // N/A지만 2바이트
            return 2;

        // 4-byte types
        case CipDataType::DINT:
        case CipDataType::UDINT:
        case CipDataType::DWORD: // N/A지만 4바이트
        case CipDataType::REAL:
            return 4;

        // 8-byte types
        case CipDataType::LINT:
        case CipDataType::ULINT:
        case CipDataType::LREAL:
        case CipDataType::LWORD: // N/A지만 8바이트
            return 8;

        case CipDataType::STRING:
            return 86;
        // Not supported / unknown
        default:
            return 1; // 기본값
    }
}


int cipDataTypeSizeFromRaw(uint16_t rawType) {
    CipDataType type = static_cast<CipDataType>(rawType);
    switch (type) {
        // 1-byte types
        case CipDataType::BOOL:
        case CipDataType::SINT:
        case CipDataType::USINT:
        case CipDataType::BYTE:  // N/A지만 1바이트
            return 1;

        // 2-byte types
        case CipDataType::INT:
        case CipDataType::UINT:
        case CipDataType::WORD:  // N/A지만 2바이트
            return 2;

        // 4-byte types
        case CipDataType::DINT:
        case CipDataType::UDINT:
        case CipDataType::DWORD: // N/A지만 4바이트
        case CipDataType::REAL:
            return 4;

        // 8-byte types
        case CipDataType::LINT:
        case CipDataType::ULINT:
        case CipDataType::LREAL:
        case CipDataType::LWORD: // N/A지만 8바이트
            return 8;

        case CipDataType::STRING:
            return 86;              // 4 for length + 82 data

        default:
            return -1; // 알 수 없는 타입 또는 미지원
    }
}

bool decodeCipValue( CipDataType dataType, size_t dataStart, const std::vector<uint8_t>& response, cip_value_u& outValue, std::vector<uint8_t>& outputRawData )
{
    switch (dataType)
    {
        case CipDataType::BOOL:
            outValue.BOOL = (response[dataStart] != 0);
            outputRawData.emplace_back(response[dataStart]);
            return true;

        case CipDataType::SINT:
            outValue.SINT = static_cast<int8_t>(response[dataStart]);
            outputRawData.emplace_back(response[dataStart]);
            return true;

        case CipDataType::USINT:
        case CipDataType::BYTE:
            outValue.USINT = response[dataStart];
            outputRawData.emplace_back(response[dataStart]);
            return true;

        case CipDataType::INT:
            outValue.INT = static_cast<int16_t>(
                response[dataStart] |
                (response[dataStart + 1] << 8)
            );

            for (size_t i = 0; i < 2; i++)
            {
                outputRawData.emplace_back(response[dataStart+i]);
            }
            
            return true;

        case CipDataType::UINT:
        case CipDataType::WORD:
            outValue.UINT = static_cast<uint16_t>(
                response[dataStart] |
                (response[dataStart + 1] << 8)
            );

            for (size_t i = 0; i < 2; i++)
            {
                outputRawData.emplace_back(response[dataStart+i]);
            }
            return true;

        case CipDataType::DINT:
        case CipDataType::DWORD:
            outValue.DINT = static_cast<int32_t>(
                response[dataStart] |
                (response[dataStart + 1] << 8) |
                (response[dataStart + 2] << 16) |
                (response[dataStart + 3] << 24)
            );

            for (size_t i = 0; i < 4; i++)
            {
                outputRawData.emplace_back(response[dataStart+i]);
            }
            return true;

        case CipDataType::UDINT:
            outValue.UDINT = static_cast<uint32_t>(
                response[dataStart] |
                (response[dataStart + 1] << 8) |
                (response[dataStart + 2] << 16) |
                (response[dataStart + 3] << 24)
            );

            for (size_t i = 0; i < 4; i++)
            {
                outputRawData.emplace_back(response[dataStart+i]);
            }
            return true;

        case CipDataType::REAL: {
            uint32_t raw = (
                response[dataStart] |
                (response[dataStart + 1] << 8) |
                (response[dataStart + 2] << 16) |
                (response[dataStart + 3] << 24)
            );
            memcpy(&outValue.REAL, &raw, sizeof(float));

            for (size_t i = 0; i < 4; i++)
            {
                outputRawData.emplace_back(response[dataStart+i]);
            }
            return true;
        }

        case CipDataType::LINT:
            outValue.LINT = 0;
            for (int i = 0; i < 8; ++i)
                outValue.LINT |= (static_cast<int64_t>(response[dataStart + i]) << (i * 8));

            for (size_t i = 0; i < 8; i++)
            {
                outputRawData.emplace_back(response[dataStart+i]);
            }
            return true;

        case CipDataType::ULINT:
            outValue.ULINT = 0;
            for (int i = 0; i < 8; ++i)
                outValue.ULINT |= (static_cast<uint64_t>(response[dataStart + i]) << (i * 8));

            for (size_t i = 0; i < 8; i++)
            {
                outputRawData.emplace_back(response[dataStart+i]);
            }
            return true;

        case CipDataType::LREAL: 
        {
            uint64_t raw = 0;
            for (int i = 0; i < 8; ++i)
                raw |= (static_cast<uint64_t>(response[dataStart + i]) << (i * 8));
            memcpy(&outValue.LREAL, &raw, sizeof(double));

            for (size_t i = 0; i < 8; i++)
            {
                outputRawData.emplace_back(response[dataStart+i]);
            }
            return true;
        }

        case CipDataType::STRING:
        {
            uint32_t strLen =
                (response[dataStart]) |
                (response[dataStart + 1] << 8) |
                (response[dataStart + 2] << 16) |
                (response[dataStart + 3] << 24);

            if (strLen > 82 || dataStart + 4 + strLen > response.size()) return false;

            outValue.STRING.Length = strLen;
            memcpy(outValue.STRING.Data, &response[dataStart + 4], strLen);
            outValue.STRING.Data[strLen] = '\0';

            for (size_t i = strLen; i-- > 0;) 
            {
                outputRawData.emplace_back(response[dataStart + 4 + i]);
            }
            
            return true;
        }

        default:
            return false;
    }
}



bool checkConnection(EthernetClient& client) {
    return client.connected();
}

/* A Guide for EtherNet/IP™ Developers,
    Encapsulation Protocol
    Defines the communication relationship between two nodes known as an Encapsulation Session. The
    Encapsulation Protocol uses TCP/UDP Port 44818 for several Encapsulation Commands and for CIP Explicit
    Messaging. An example encapsulation command is the List_Identity Command that performs a “network
    who”. An Encapsulation Session must be established before any CIP communications can take place.
    Data format for the Encapsulation Protocol is Little-Endian.
*/
bool sendEncapsulationPacket(EIPSession& session, const std::vector<uint8_t>& serviceData, std::vector<uint8_t>& response) {
    if (!session.connected) 
    {
        LOG_ERROR(muffin::logger,"CONNECT ERROR");
        return false;
    }

    std::vector<uint8_t> rrData;
    rrData.insert(rrData.end(), {0x00,0x00,0x00,0x00, 0x00,0x00});  // Interface Handle (4바이트), Timeout
    rrData.push_back(0x02); rrData.push_back(0x00);                 // Item count = 2
    rrData.insert(rrData.end(), {0x00,0x00, 0x00,0x00});            // Null Address Item
    rrData.push_back(0xB2); rrData.push_back(0x00);                 // Unconnected Data Item (Type ID: 0x00B2)

    rrData.push_back((uint8_t)(serviceData.size() & 0xFF));         // LSB             
    rrData.push_back((uint8_t)((serviceData.size() >> 8) & 0xFF));  // MSB

    rrData.insert(rrData.end(), serviceData.begin(), serviceData.end());

    // debug
    Serial.print("[DEBUG] RRData Packet Dump (");
    Serial.print(rrData.size());
    Serial.println(" bytes):");

    for (size_t i = 0; i < rrData.size(); ++i) {
      if (i % 16 == 0 && i != 0) Serial.println();
      if (rrData[i] < 0x10) Serial.print("0");
      Serial.print(rrData[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // Encapsulation Header + RR Data
    std::vector<uint8_t> packet;
    packet.push_back(0x6F); packet.push_back(0x00); // SendRRData
    
    uint16_t len = rrData.size();
    
    packet.push_back(len & 0xFF); packet.push_back((len >> 8) & 0xFF);


    uint32_t handle = session.sessionHandle;
    Serial.print("[CIP] sendEncapsulationPacket RRData 패킷 전송용 세션 핸들: ");
    Serial.println(session.sessionHandle, HEX);

    Serial.print("[CIP] sendEncapsulationPacket sessionHandle (LE bytes): ");
    Serial.print(handle & 0xFF, HEX); Serial.print(" ");
    Serial.print((handle >> 8) & 0xFF, HEX); Serial.print(" ");
    Serial.print((handle >> 16) & 0xFF, HEX); Serial.print(" ");
    Serial.println((handle >> 24) & 0xFF, HEX);    


    uint32_t handle_be = session.sessionHandle;

    packet.push_back((handle_be >> 0) & 0xFF);
    packet.push_back((handle_be >> 8) & 0xFF);
    packet.push_back((handle_be >> 16) & 0xFF);
    packet.push_back((handle_be >> 24) & 0xFF);

    // Reserved (12 bytes)
    for (int i = 0; i < 12; ++i) packet.push_back(0x00);

    // Add options (4 bytes) = 0x00000000
    for (int i = 0; i < 4; ++i)
        packet.push_back(0x00);

    packet.insert(packet.end(), rrData.begin(), rrData.end());

    session.client->write(packet.data(), packet.size());
    delay(50);
    //response.clear();
    //while (session.client->available()) response.push_back(session.client->read());

    response.clear();
    size_t offset = 0;

    // Serial.println("[DEBUG] 수신 데이터 (1바이트씩):");

    while (session.client->available()) {
        int byte = session.client->read();  // 1바이트 읽기

        if (byte >= 0) {
            response.push_back((uint8_t)byte);

            // 오프셋 줄바꿈
            if (offset % 16 == 0) Serial.printf("\n%04zx: ", offset);

            Serial.printf("%02X ", (uint8_t)byte);
            ++offset;
        }
    }
    Serial.println();

    return !response.empty();
}

#endif