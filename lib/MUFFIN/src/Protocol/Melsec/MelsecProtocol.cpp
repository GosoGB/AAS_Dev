#include "MelsecProtocol.h"
#include <string.h>

// 기본 ASCII 모드용 기본 헤더 (4E 프레임)
const String baseHeader = "500000FF03FF00";

MelsecProtocol::MelsecProtocol() 
  : _port(0), _ip(nullptr), plcSeries(0), monitoringTimer(0x04), dataFormat(MC_ASCII) {
}

MelsecProtocol::MelsecProtocol(const char *ip, uint16_t port, MitsuPLCSeries series)
  : _port(port), _ip(ip), plcSeries(series), monitoringTimer(0x04), dataFormat(MC_ASCII) {
  tcp.setIP(ip);
  tcp.setPort(port);
}

bool MelsecProtocol::begin(const char *ip, uint16_t port, MitsuPLCSeries series) {
  _ip = ip;
  _port = port;
  plcSeries = series;
  tcp.setIP(ip);
  tcp.setPort(port);
  return tcp.testConnection();
}

bool MelsecProtocol::begin() {
  return begin(_ip, _port, (MitsuPLCSeries)plcSeries);
}

String MelsecProtocol::sendAndReceive(const String &command) {
  if (dataFormat == MC_ASCII) {
    return tcp.sendAndReceive(command);
  }

  // MC_BINARY 처리
  const uint8_t *data = (const uint8_t *)command.c_str();
  int length = command.length();

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
  log_d("result : %s",result.c_str());
  return result;
}


int MelsecProtocol::sendAndReceive(const String &command, uint16_t buffer[]) {
  String resp = sendAndReceive(command);
  if (resp == "ERROR") return 0;
  return hexStringToWords(resp, buffer);
}


String MelsecProtocol::extractBinaryData(const String &response) 
{
  const int HEADER_SIZE = 11;  // Binary 응답 헤더 길이 (고정)
  if (response.length() <= HEADER_SIZE * 2) return "";

  // HEX 문자 2자리씩 1바이트 → 인덱스 22부터가 데이터
  return response.substring(HEADER_SIZE * 2);
}

String MelsecProtocol::extractAsciiData(const String &response) 
{
  if (response.length() <= 22) return "";
  return response.substring(22);  // 실제 응답 데이터
}

int MelsecProtocol::hexStringToWords(const String &hexStr, uint16_t buffer[]) {
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

int MelsecProtocol::wordsToHexString(const uint16_t data[], int wordCount, String &hexStr) {
  hexStr = "";
  char buf[5];
  for (int i = 0; i < wordCount; i++) {
    sprintf(buf, "%.4X", data[i]);
    hexStr += String(buf);
  }
  return wordCount;
}

int MelsecProtocol::wordArrayToByteArray(const uint16_t words[], uint8_t bytes[], int wordCount) {
  for (int i = 0; i < wordCount; i++) {
    bytes[i * 2]     = (uint8_t)(words[i] >> 8);
    bytes[i * 2 + 1] = (uint8_t)(words[i] & 0xFF);
  }
  return wordCount * 2;
}

String MelsecProtocol::fitStringToWords(const String &input, int wordCount) {
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

String MelsecProtocol::stringToHexASCII(const String &input) {
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

String MelsecProtocol::batchReadWrite(MitsuDeviceType device, uint32_t address, int count, bool read, bool isBit, const String &dataToWrite) {
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

    String addrStr = String(address);
    while (addrStr.length() < 6) {
      addrStr = "0" + addrStr;
    }
    command += addrStr;

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

  // 3. 디바이스 주소
  frame[index++] = (uint8_t)(address & 0xFF);
  frame[index++] = (uint8_t)((address >> 8) & 0xFF);
  frame[index++] = (uint8_t)((address >> 16) & 0xFF);

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

bool MelsecProtocol::writeWords(MitsuDeviceType device, uint32_t address, int wordCount, const uint16_t data[])
{
  String dataStr = "";

  if (dataFormat == MC_ASCII) {
    // ASCII 모드: 워드를 HEX 문자열로 변환
    wordsToHexString(data, wordCount, dataStr);
  } else {
    // BINARY 모드: 워드를 바이트 배열로 변환 후 HEX로 인코딩
    for (int i = 0; i < wordCount; i++) {
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


bool MelsecProtocol::writeWord(MitsuDeviceType device, uint32_t address, uint16_t word) {
  return writeWords(device, address, 1, &word);
}

bool MelsecProtocol::writeBit(MitsuDeviceType device, uint32_t address, uint8_t value) {
  String data = (value ? "1" : "0");
  String frame = batchReadWrite(device, address, 1, false, true, data);
  String resp = sendAndReceive(frame);
  return (resp != "ERROR");
}

bool MelsecProtocol::writeBits(MitsuDeviceType device, uint32_t address, int count, const bool *values)
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


int MelsecProtocol::readWords(MitsuDeviceType device, uint32_t address, int wordCount, uint16_t buffer[]) 
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


int MelsecProtocol::readBits(MitsuDeviceType device, uint32_t address, int count, bool *buffer) 
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
  log_d("frame : %s",frame.c_str());


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
    log_d("Bit[%d] from byte[0x%02X] = %d", i, byte, bitVal);
  }

  return count;
}

bool MelsecProtocol::run(bool force, MitsuRemoteControlClearMode clearMode) {
  String resp = moduleControl(RUN, force ? EXECUTE_FORCIBLY : DONT_EXECUTE_FORCIBLY, clearMode);
  return (resp != "ERROR");
}

bool MelsecProtocol::pause(bool force) {
  String resp = moduleControl(PAUSE, force ? EXECUTE_FORCIBLY : DONT_EXECUTE_FORCIBLY);
  return (resp != "ERROR");
}

bool MelsecProtocol::stop() {
  String resp = moduleControl(STOP);
  return (resp != "ERROR");
}

bool MelsecProtocol::clear() {
  String resp = moduleControl(CLEAR);
  return (resp != "ERROR");
}

bool MelsecProtocol::reset() {
  String resp = moduleControl(RESET);
  return (resp != "ERROR");
}

String MelsecProtocol::getCPUModel() {
  return moduleControl(CPU_MODEL);
}

String MelsecProtocol::getErrorDescription(const String &error) {
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

uint8_t MelsecProtocol::getDeviceCode(MitsuDeviceType device) {
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

uint8_t MelsecProtocol::getMonitoringTimer() {
  return monitoringTimer;
}

void MelsecProtocol::setMonitoringTimer(uint8_t time) {
  monitoringTimer = time;
}
