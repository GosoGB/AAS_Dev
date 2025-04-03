#include "MelsecClient.h"
#include <string.h>

// 기본 ASCII 모드용 기본 헤더 (4E 프레임)
const String baseHeader = "500000FF03FF00";

MelsecClient::MelsecClient() 
  : _port(0), _ip(nullptr), plcSeries(0), monitoringTimer(0x04), dataFormat(MC_ASCII) {
}

MelsecClient::MelsecClient(const char *ip, uint16_t port, MitsuPLCSeries series)
  : _port(port), _ip(ip), plcSeries(series), monitoringTimer(0x04), dataFormat(MC_ASCII) {
  tcp.setIP(ip);
  tcp.setPort(port);
}

bool MelsecClient::begin(const char *ip, uint16_t port, MitsuPLCSeries series) {
  _ip = ip;
  _port = port;
  plcSeries = series;
  tcp.setIP(ip);
  tcp.setPort(port);
  return tcp.connectTCP();
}

bool MelsecClient::begin() {
  return begin(_ip, _port, (MitsuPLCSeries)plcSeries);
}

bool MelsecClient::Connected() 
{
  if (tcp.connected == false)
  {
    return false;
  }
  
  
  if (dataFormat == MC_ASCII)
  {
    String cmd = baseHeader;

    // NOP 명령: 모니터링 타이머 + 명령(0C00) + 서브커맨드(0000)
    // 데이터 길이 = 2(타이머) + 2(커맨드) + 2(서브커맨드) = 6 = 0x0006
    cmd += "0006";     // Request data length (6 bytes = 12 ASCII chars)
    cmd += "0010";     // Monitoring timer (0010 = 1000ms)
    cmd += "0C00";     // Command = 0x000C (NOP)
    cmd += "0000";     // Subcommand = 0x0000

    String response = sendAndReceive(cmd);

    // 최소 응답 길이 확인 (헤더 + 응답코드까지 22)
    if (response == "ERROR" || response.length() < 22)
    {
      tcp.closeConnection();
      return false;
    }

    String endCode = response.substring(34, 38);
    
    if (endCode.equalsIgnoreCase("0000")) 
    {
      return true;
    }
    else
    {
      tcp.closeConnection();
      return false;
    }
  }
  else
  {
    uint8_t frame[1024];
    int index = 0;

    // 1. 서두부 (Subheader ~ Request data length)
    frame[index++] = 0x50;  // Subheader (ASCII 헤더 기준)
    frame[index++] = 0x00;
    frame[index++] = 0x00;  // Network No.
    frame[index++] = 0xFF;  // PC No.
    frame[index++] = 0xFF;  // I/O No (LSB)
    frame[index++] = 0x03;  // I/O No (MSB)
    frame[index++] = 0x00;  // Unit No.

    // 2. 데이터 길이 (Request Data Length)
    frame[index++] = 0x06;  // Length LSB
    frame[index++] = 0x00;  // Length MSB

    // 3. 모니터링 타이머 (1000ms)
    frame[index++] = 0x10;  // Timer LSB
    frame[index++] = 0x00;  // Timer MSB

    // 4. 커맨드 (NOP = 0x000C)
    frame[index++] = 0x0C;  // Command LSB
    frame[index++] = 0x00;  // Command MSB

    // 5. 서브커맨드
    frame[index++] = 0x00;  // Subcommand LSB
    frame[index++] = 0x00;  // Subcommand MSB
    String response = sendAndReceive(String((char *)frame, index));

    // 최소 응답 길이 확인 (헤더 + 응답코드까지 22)
    if (response == "ERROR" || response.length() < 22)
    {
      tcp.closeConnection();
      return false;
    }

    String endCode = response.substring(34, 38);
    if (endCode.equalsIgnoreCase("0000")) 
    {
      return true;
    }
    else
    {
      tcp.closeConnection();
      return false;
    }
  }
}

String MelsecClient::sendAndReceive(const String &command) {
  if (dataFormat == MC_ASCII)
  {
    return tcp.sendAndReceive(command);
  }

  // MC_BINARY 처리
  const uint8_t *data = (const uint8_t *)command.c_str();
  int length = command.length();
  // Serial.print("DATA : ");
  // for (size_t i = 0; i < length; i++)
  // {
    // Serial.printf("%02X",data[i]);
  // }
  // Serial.println();
  uint8_t response[1024];
  int respLen = tcp.sendAndReceiveBinary(data, length, response);

  if (respLen <= 0) return "ERROR";

  // 응답 바이트를 HEX 문자열로 변환
  String result = "";
  char buf[3];
  for (int i = 0; i < respLen; i++) {
    sprintf(buf, "%02X", response[i]);
    result += String(buf);
  }
  // log_d("result : %s",result.c_str());
  return result;
}


int MelsecClient::sendAndReceive(const String &command, uint16_t buffer[]) {
  String resp = sendAndReceive(command);
  if (resp == "ERROR") return 0;
  return hexStringToWords(resp, buffer);
}


String MelsecClient::extractBinaryData(const String &response) 
{
  const int HEADER_SIZE = 11;  // Binary 응답 헤더 길이 (고정)
  if (response.length() <= HEADER_SIZE * 2) return "";

  // HEX 문자 2자리씩 1바이트 → 인덱스 22부터가 데이터
  return response.substring(HEADER_SIZE * 2);
}

String MelsecClient::extractAsciiData(const String &response) 
{
  if (response.length() <= 22) return "";
  return response.substring(22);  // 실제 응답 데이터
}

int MelsecClient::hexStringToWords(const String &hexStr, uint16_t buffer[]) {
  int wordCount = hexStr.length() / 4;
  String fitted = fitStringToWords(hexStr, wordCount);
  for (int i = 0; i < wordCount; i++) {
    uint16_t word = 0;
    for (int j = 0; j < 4; j++) {
      char c = fitted[i * 4 + j];
      word <<= 4;
      if (c >= '0' && c <= '9')
        word |= (c - '0');
      else if (c >= 'A' && c <= 'F')
        word |= (c - 'A' + 10);
      else if (c >= 'a' && c <= 'f')
        word |= (c - 'a' + 10);
    }
    buffer[i] = word;
  }
  return wordCount;
}

int MelsecClient::wordsToHexString(const uint16_t data[], int wordCount, String &hexStr) {
  hexStr = "";
  char buf[5];
  for (int i = 0; i < wordCount; i++) {
    sprintf(buf, "%.4X", data[i]);
    hexStr += String(buf);
  }
  return wordCount;
}

int MelsecClient::wordArrayToByteArray(const uint16_t words[], uint8_t bytes[], int wordCount) {
  for (int i = 0; i < wordCount; i++) {
    bytes[i * 2]     = (uint8_t)(words[i] >> 8);
    bytes[i * 2 + 1] = (uint8_t)(words[i] & 0xFF);
  }
  return wordCount * 2;
}

String MelsecClient::fitStringToWords(const String &input, int wordCount) {
  int requiredLength = wordCount * 4;
  String output = input;
  int diff = requiredLength - input.length();
  if (diff > 0) {
    for (int i = 0; i < diff; i++) {
      output = "0" + output;
    }
  } else if (diff < 0) {
    output = output.substring(0, requiredLength);
  }
  return output;
}

String MelsecClient::stringToHexASCII(const String &input) {
  String result = "";
  for (size_t i = 0; i < input.length(); i++) {
    char buf[3];
    sprintf(buf, "%02X", input[i]);
    result += String(buf);
  }
  while (result.length() % 4 != 0) {
    result += "0";
  }
  String inverted = "";
  for (size_t i = 0; i < result.length(); i += 4) {
    inverted += result.substring(i+2, i+4) + result.substring(i, i+2);
  }
  return inverted;
}

bool MelsecClient::isHexMemory(const MitsuDeviceType type)
{
  switch (type)
  {
  case X:
  case Y:
  case B:
  case W:
  case SB:
  case SW:
  case DX:
  case DY:
    return true;
  case SM:
  case M:
  case L:
  case F:
  case V:
  case TS:
  case TC:
  case LTS:
  case LTC:
  case STS:
  case STC:
  case LSTS:
  case LSTC:
  case CS:
  case CC:
  case LCS:
  case LCC:
  case SD:
  case D:
  case TN:
  case CN:
  case Z:
  case LTN:
  case STN:
  case LSTN:
  case LCN:
  case LZ:
    return false;
  default:
      return false;
  }
}

String MelsecClient::batchReadWrite(MitsuDeviceType device, uint32_t address, int count, bool read, bool isBit, const String &dataToWrite) {
  if (dataFormat == MC_ASCII) 
  {
    // 기존 ASCII 코드 유지
    String command = "";
    char buf[7];
    sprintf(buf, "%.4X", monitoringTimer);
    command += String(buf);
    command += (read ? "0401" : "1401");

    if (plcSeries == iQR_SERIES) {
      command += (isBit ? "0003" : "0002");
      command += String(DeviceTypeiQR[device]);
    } else {
      command += (isBit ? "0001" : "0000");
      command += String(DeviceTypeQL[device]);
    }

    if (isHexMemory(device))
    {
      char hexStr[7];  // 6자리 + 널종료
      sprintf(hexStr, "%06X", address);  // 대문자 HEX, 6자리 고정
      command += String(hexStr);
    }
    else
    {
      String addrStr = String(address);
      while (addrStr.length() < 6) 
      {
        addrStr = "0" + addrStr;
      }
      command += String(addrStr);
    }

    sprintf(buf, "%.4X", count);
    command += String(buf);

    command += dataToWrite;

    int dataLen = command.length();
    sprintf(buf, "%.4X", dataLen);
    String fullFrame = baseHeader + String(buf) + command;
    return fullFrame;
  }
  // ✅ BINARY 프레임 생성
  uint8_t frame[1024];
  int index = 0;

  // 1. 서두부 (Subheader ~ Request data length)
  frame[index++] = 0x50;
  frame[index++] = 0x00;
  frame[index++] = 0x00;
  frame[index++] = 0xFF;
  frame[index++] = 0xFF;
  frame[index++] = 0x03;
  frame[index++] = 0x00;

  // 임시로 나중에 채우는 요청 길이 위치 기억
  int lengthPos = index;
  frame[index++] = 0x00; // 데이터 길이 (2바이트, little-endian)
  frame[index++] = 0x00;

  // 2. CPU 모듈 명령
  frame[index++] = 0x10; // Monitoring timer (4)
  frame[index++] = 0x00;

  frame[index++] = (read ? 0x01 : 0x01); // Command (read: 0x0401 / write: 0x1401)
  frame[index++] = (read ? 0x04 : 0x14);

  // Subcommand
  if (plcSeries == iQR_SERIES) {
    frame[index++] = (isBit ? 0x03 : 0x02);
    frame[index++] = 0x00;
  } else {
    frame[index++] = (isBit ? 0x01 : 0x00);
    frame[index++] = 0x00;
  }

  if (isHexMemory(device))
  {
    char buf[16];
    sprintf(buf, "%X", address);  // HEX 문자열로
    uint32_t convertedAddress = strtoul(buf, nullptr, 16);  // 다시 10진수로
    // 3. 디바이스 주소
    frame[index++] = (uint8_t)(convertedAddress & 0xFF);
    frame[index++] = (uint8_t)((convertedAddress >> 8) & 0xFF);
    frame[index++] = (uint8_t)((convertedAddress >> 16) & 0xFF);
  }
  else
  {
    frame[index++] = (uint8_t)(address & 0xFF);
    frame[index++] = (uint8_t)((address >> 8) & 0xFF);
    frame[index++] = (uint8_t)((address >> 16) & 0xFF);
  }


  // 4. 디바이스 코드
  frame[index++] = getDeviceCode(device); // 예: M=0x90, D=0xA8 등

  // 5. 읽기/쓰기 개수
  frame[index++] = (uint8_t)(count & 0xFF);
  frame[index++] = (uint8_t)((count >> 8) & 0xFF);

  // 6. 쓰기 데이터가 있다면 추가
  if (!read && dataToWrite.length() > 0) {
    // HEX 문자열을 바이트로 변환하여 붙임
    for (int i = 0; i < dataToWrite.length(); i += 2) {
      String byteStr = dataToWrite.substring(i, i + 2);
      uint8_t b = (uint8_t)strtoul(byteStr.c_str(), nullptr, 16);
      frame[index++] = b;
    }
  }

  // 요청 길이 계산하여 삽입
  uint16_t reqLen = index - 9;
  frame[lengthPos] = (uint8_t)(reqLen & 0xFF);
  frame[lengthPos + 1] = (uint8_t)((reqLen >> 8) & 0xFF);

  // 바이너리 전송 버퍼 준비 완료
  return String((char *)frame, index);  // 실제 전송은 sendAndReceiveBinary에서 처리
}

bool MelsecClient::writeWords(MitsuDeviceType device, uint32_t address, int wordCount, const uint16_t data[])
{
  String dataStr = "";

  if (dataFormat == MC_ASCII) 
  {
    // ASCII 모드: 워드를 HEX 문자열로 변환
    wordsToHexString(data, wordCount, dataStr);
  } 
  else 
  {
    // BINARY 모드: 워드를 바이트 배열로 변환 후 HEX로 인코딩
    for (int i = 0; i < wordCount; i++) 
    {
      // 워드는 little-endian (LSB 먼저)
      uint8_t lsb = data[i] & 0xFF;
      uint8_t msb = (data[i] >> 8) & 0xFF;

      char hex[5];
      sprintf(hex, "%02X%02X", lsb, msb);  // LSB 먼저
      dataStr += String(hex);
    }
  }

  String frame = batchReadWrite(device, address, wordCount, false, false, dataStr);
  String resp = sendAndReceive(frame);

  log_d("writeWords response: %s", resp.c_str());

  return (resp != "ERROR");
}


bool MelsecClient::writeWord(MitsuDeviceType device, uint32_t address, uint16_t word) {
  return writeWords(device, address, 1, &word);
}

bool MelsecClient::writeBit(MitsuDeviceType device, uint32_t address, uint8_t value) {
  bool data[1];
  data[0] = value;
  return writeBits(device, address, 1, data);
}

bool MelsecClient::writeBits(MitsuDeviceType device, uint32_t address, int count, const bool *values)
{
  String data = "";

  if (dataFormat == MC_ASCII) 
  {
    for (int i = 0; i < count; i++) {
      data += values[i] ? "1" : "0";
    }
  } 
  else 
  {
    for (int i = 0; i < count; i += 2) 
    {
      uint8_t byte = 0x00;

      // 짝수 비트 → bit4
      if (values[i]) byte |= (1 << 4);

      // 홀수 비트 → bit0
      if (i + 1 < count && values[i + 1]) byte |= (1 << 0);

      char hex[3];
      sprintf(hex, "%02X", byte);
      data += String(hex);
    }
  }

  String frame = batchReadWrite(device, address, count, false, true, data);
  String resp = sendAndReceive(frame);
  log_d("writeBits response : %s", resp.c_str());

  return (resp != "ERROR");
}


int MelsecClient::readWords(MitsuDeviceType device, uint32_t address, int wordCount, uint16_t buffer[]) 
{
  String frame = batchReadWrite(device, address, wordCount, true);
  String resp = sendAndReceive(frame);

  if (dataFormat == MC_ASCII) 
  {
    String data = extractAsciiData(resp);
    if (data == "" || data.length() < wordCount * 4) return 0;
    return hexStringToWords(data, buffer);
  }

  // 🧠 Binary 모드
  String data = extractBinaryData(resp); 
  if (data.length() < wordCount * 4) return 0; 

  for (int i = 0; i < wordCount; i++) 
  {
    int byteIdx = i * 4; 
    uint8_t lsb = (uint8_t)strtoul(data.substring(byteIdx, byteIdx + 2).c_str(), nullptr, 16);
    uint8_t msb = (uint8_t)strtoul(data.substring(byteIdx + 2, byteIdx + 4).c_str(), nullptr, 16);
    buffer[i] = (msb << 8) | lsb;
  }

  return wordCount;
}


int MelsecClient::readBits(MitsuDeviceType device, uint32_t address, int count, bool *buffer) 
{
  if (dataFormat == MC_ASCII) {
    // 기존 ASCII 경로
    String frame = batchReadWrite(device, address, count, true, true);
    String resp = sendAndReceive(frame);
    String data = extractAsciiData(resp);

    if (data == "" || data.length() < count) return 0;

    for (int i = 0; i < count && i < data.length(); i++) {
      buffer[i] = (data[i] == '1');
    }
    return count;
  }

  // 🧠 Binary 모드

  String frame = batchReadWrite(device, address, count, true, true);
  const char *raw = frame.c_str();
  const uint8_t *cmd = (const uint8_t *)raw;

  uint8_t response[256];
  int len = tcp.sendAndReceiveBinary(cmd, frame.length(), response);
  if (len <= 11) return 0;

  const int payloadStart = 11;  // Header size
  for (int i = 0; i < count; i++) {
    int byteIndex = payloadStart + (i / 2);
    if (byteIndex >= len) break;

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

bool MelsecClient::run(bool force, MitsuRemoteControlClearMode clearMode) {
  String resp = moduleControl(RUN, force ? EXECUTE_FORCIBLY : DONT_EXECUTE_FORCIBLY, clearMode);
  return (resp != "ERROR");
}

bool MelsecClient::pause(bool force) {
  String resp = moduleControl(PAUSE, force ? EXECUTE_FORCIBLY : DONT_EXECUTE_FORCIBLY);
  return (resp != "ERROR");
}

bool MelsecClient::stop() {
  String resp = moduleControl(STOP);
  return (resp != "ERROR");
}

bool MelsecClient::clear() {
  String resp = moduleControl(CLEAR);
  return (resp != "ERROR");
}

bool MelsecClient::reset() {
  String resp = moduleControl(RESET);
  return (resp != "ERROR");
}

String MelsecClient::getCPUModel() {
  return moduleControl(CPU_MODEL);
}

String MelsecClient::getErrorDescription(const String &error) {
  uint16_t codeArray[1];
  hexStringToWords(error, codeArray);
  uint16_t errorCode = codeArray[0];
  
  if (errorCode >= 0x4000 && errorCode <= 0x4FFF)
    return "CPU module detected errors (non-MC protocol communication error).";
  else if (errorCode == 0x0050)
    return "Invalid command/subcommand header.";
  else if (errorCode == 0x0055)
    return "PLC in RUN mode cannot accept write command due to online change disabled.";
  else if (errorCode == 0xC050)
    return "Received non-binary data while ASCII mode is set.";
  else if (errorCode >= 0xC051 && errorCode <= 0xC054)
    return "The number of read/write points is out of allowed range.";
  else if (errorCode == 0xC056)
    return "Read/write request exceeds maximum address.";
  else if (errorCode == 0xC058)
    return "Data length mismatch in character area.";
  else if (errorCode == 0xC059)
    return "Incorrect command/subcommand or unsupported by CPU module.";
  else if (errorCode == 0xC05B)
    return "Specified device cannot be accessed by CPU module.";
  else if (errorCode == 0xC05C)
    return "Incorrect data (e.g., bit access on word device).";
  else if (errorCode == 0xC05D)
    return "No monitor registration.";
  else if (errorCode == 0xC05F)
    return "Request cannot be executed by CPU module.";
  else if (errorCode == 0xC060)
    return "Incorrect data specification for bit device.";
  else if (errorCode == 0xC061)
    return "Data length mismatch in character area.";
  else if (errorCode == 0xC06F)
    return "Communication data format mismatch (ASCII/Binary).";
  else if (errorCode == 0xC070)
    return "Device memory extension not supported for target station.";
  else if (errorCode == 0xC0B5)
    return "CPU module cannot handle specified data.";
  else if (errorCode == 0xC200)
    return "Incorrect remote password.";
  else if (errorCode == 0xC201)
    return "Communication port locked due to remote password.";
  else if (errorCode == 0xC240)
    return "Connected device does not match remote password unlock request.";
  
  return "UNKNOWN ERROR";
}

uint8_t MelsecClient::getDeviceCode(MitsuDeviceType device) {
  switch (device) {
    case M: return 0x90;
    case D: return 0xA8;
    case X: return 0x9C;
    case Y: return 0x9D;
    case L: return 0x92;
    case B: return 0xA0;
    case W: return 0xB4;
    default: return 0x00; // 기본값 또는 예외 처리
  }
}

uint8_t MelsecClient::getMonitoringTimer() {
  return monitoringTimer;
}

void MelsecClient::setMonitoringTimer(uint8_t time) {
  monitoringTimer = time;
}
