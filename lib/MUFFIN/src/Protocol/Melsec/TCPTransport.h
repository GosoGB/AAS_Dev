#ifndef TCP_TRANSPORT_H
#define TCP_TRANSPORT_H

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

class TCPTransport {
public:
  TCPTransport();
  TCPTransport(const char *ip, uint16_t port);

  // Sends a hex string command and returns the hex string response (ASCII mode)
  String sendAndReceive(const String &command);
  
  // NEW: Sends binary data and returns the binary response converted to a hex string
  int sendAndReceiveBinary(const uint8_t *cmd, int length, uint8_t *responseBuf);
  
  void setTimeout(unsigned long timeout);
  void setIP(const char *ip);
  void setPort(uint16_t port);
  bool testConnection();
  unsigned long getTimeout();
  bool connectTCP();
  void closeConnection();
  int send(const uint8_t command[], int length);
  int receive(uint8_t response[], int maxLength);

private:
  const char *_ip;
  uint16_t _port = 1000;
  WiFiClient client;
  bool connected = false;
  unsigned long _timeout = 100; // in milliseconds
};

#endif // TCP_TRANSPORT_H
