#include "TCPTransport.h"

TCPTransport::TCPTransport() {}

TCPTransport::TCPTransport(const char *ip, uint16_t port) : _ip(ip), _port(port) {}

String TCPTransport::sendAndReceive(const String &command) {
  if (!client.connected()) {
    if (!connectTCP()) {
      Serial.println("Connection failed.");
      return "";
    }
  }
  String response = "";
  int cmdLen = command.length();
  log_d("DATA : %s \n\n ",command.c_str());

  int written = client.write((const uint8_t *)command.c_str(), cmdLen);
  unsigned long start = millis();
  while (!client.available() && (millis() - start) < _timeout) {
    delay(1);
  }
  int availableBytes = client.available();
  if (availableBytes <= 0) {
    Serial.println("No data available from client after write.");
    client.stop();
    return "";
  }
  uint8_t resp[2048];
  int len = client.read(resp, availableBytes);
  for (int i = 0; i < len; i++) {
    response += (char)resp[i];
  }
  client.stop();
  connected = false;

  log_d("response : %s",response.c_str());
  return response;
}

int TCPTransport::sendAndReceiveBinary(const uint8_t *cmd, int length, uint8_t *responseBuf) {
  if (!client.connected()) {
    if (!connectTCP()) return 0;
  }

  Serial.print("SEND : ");
  
  for (size_t i = 0; i < length; i++)
  {
    Serial.printf("%02X ", cmd[i]);
  }
  Serial.println();
  client.write(cmd, length);

  unsigned long start = millis();
  while (!client.available() && (millis() - start) < _timeout) delay(1);
  log_d("available : %d", client.available());
  size_t idx = 0;
  while (client.available()>0)
  {
    responseBuf[idx] = client.read();
    idx++;
  }
  client.stop();

  Serial.print("RESPONSE : ");
  
  for (size_t i = 0; i < idx; i++)
  {
    Serial.printf("%02X ", responseBuf[i]);
  }
  Serial.println();

  return idx;
}


void TCPTransport::setIP(const char *ip) {
  _ip = ip;
}

void TCPTransport::setPort(uint16_t port) {
  _port = port;
}

void TCPTransport::setTimeout(unsigned long timeout) {
  _timeout = timeout;
}

unsigned long TCPTransport::getTimeout() {
  return _timeout;
}

bool TCPTransport::testConnection() {
  if (client.connect(_ip, _port)) {
    connected = true;
    Serial.println("Test connection successful.");
  } else {
    connected = false;
    Serial.println("Test connection failed.");
  }
  client.stop();
  return connected;
}

bool TCPTransport::connectTCP() {
  if (client.connect(_ip, _port)) {
    connected = true;
  } else {
    connected = false;
    Serial.println("TCP connection error.");
  }
  return connected;
}

void TCPTransport::closeConnection() {
  connected = false;
  client.stop();
}

int TCPTransport::send(const uint8_t command[], int length) {
  int written = client.write(command, length);
  if (written != length) {
    Serial.println("Error writing command.");
    return 0;
  }
  return written;
}

int TCPTransport::receive(uint8_t response[], int maxLength) {
  int loops = 0;
  while (!client.available() && loops < 1000) {
    loops++;
    delay(1);
  }
  int len = client.read(response, maxLength);
  if (len <= 0) {
    Serial.println("No data received.");
  }
  return len;
}
