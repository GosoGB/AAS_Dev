

#include <Arduino.h>
#include <WiFi.h>
#include "MelsecConstants.h"
#include "TCPTransport.h"

enum MCDataFormat {
  MC_BINARY,
  MC_ASCII
};

class MelsecClient {
public:
  MelsecClient();
  MelsecClient(const char *ip, uint16_t port, MitsuPLCSeries series = QL_SERIES);

  // Initialize communication with the PLC
  bool begin(const char *ip, uint16_t port, MitsuPLCSeries series = QL_SERIES);
  bool begin();

  // Set communication mode (ASCII or BINARY)
  void setDataFormat(MCDataFormat format) { dataFormat = format; }
  MCDataFormat getDataFormat() { return dataFormat; }
  bool Connected();
  // Write operations
  bool writeWords(MitsuDeviceType device, uint32_t address, int wordCount, const uint16_t data[]);
  bool writeWord(MitsuDeviceType device, uint32_t address, uint16_t word);
  bool writeBit(MitsuDeviceType device, uint32_t address, uint8_t value);
  bool writeBits(MitsuDeviceType device, uint32_t address, int count, const bool *values);


  // Read operations
  int readWords(MitsuDeviceType device, uint32_t address, int wordCount, uint16_t buffer[]);
  int readBits(MitsuDeviceType device, uint32_t address, int count, bool *buffer);

  
  // CPU control functions
  bool run(bool force, MitsuRemoteControlClearMode clearMode);
  bool pause(bool force);
  bool stop();
  bool clear();
  bool reset();
  String getCPUModel();

  // Error description
  String getErrorDescription(const String &error);

  // Monitoring timer
  uint8_t getMonitoringTimer();
  void setMonitoringTimer(uint8_t time);

private:
  uint16_t _port;
  const char *_ip;
  TCPTransport tcp;
  int plcSeries; // 0: QL_SERIES, 1: iQR_SERIES
  uint8_t monitoringTimer; // in seconds
  MCDataFormat dataFormat;  // Communication mode (default MC_ASCII)

  // Low-level communication
  String sendAndReceive(const String &command);
  int sendAndReceive(const String &command, uint16_t buffer[]);

  // Data conversion helpers
  int hexStringToWords(const String &hexStr, uint16_t buffer[]);
  int wordsToHexString(const uint16_t data[], int wordCount, String &hexStr);
  int wordArrayToByteArray(const uint16_t words[], uint8_t bytes[], int wordCount);

  String fitStringToWords(const String &input, int wordCount);
  String stringToHexASCII(const String &input);
  String extractAsciiData(const String &response);
  String extractBinaryData(const String &response);

  uint8_t getDeviceCode(MitsuDeviceType device);

  bool isHexMemory(const MitsuDeviceType type);
  // Frame generation (기존 방식: 숫자 주소 사용)
  String batchReadWrite(MitsuDeviceType device, uint32_t address, int count, bool read, bool isBit = false, const String &dataToWrite = "");
  String moduleControl(MitsuPLCRemoteControl control, MitsuRemoteControlMode mode = DONT_EXECUTE_FORCIBLY, MitsuRemoteControlClearMode clearMode = DO_NOT_CLEAR);
};
