#pragma once
#include <WiFi101.h>
#include <WiFiUdp.h>

class WiFiRTPConnection
{
public:
  void begin()
  {
    // Because the confounded device CRASHES if we do this in the constructor, called during global static initialisation

    if (WiFi.status() == WL_NO_SHIELD)
    {
      return;
    }

    auto Status = WL_IDLE_STATUS;
    while (Status != WL_CONNECTED)
    {
      Status = static_cast<decltype(Status)>(WiFi.begin("nice", "try"));
      delay(1000);
    }
  }

  long GetRSSI()
  {
    return WiFi.RSSI();
  }
  
  unsigned long GetEpoch()
  {
    static const unsigned int LocalPort = 2390;
    UDPConnection.begin(LocalPort);
    
    RequestTime();
    delay(1000);
    
    uint8_t PacketBuffer[NTP_PACKET_SIZE] = {};
    if (UDPConnection.parsePacket())
    {
      static const unsigned long SeventyYears = 2208988800;
      
      UDPConnection.read(PacketBuffer, NTP_PACKET_SIZE);
      unsigned long SecsSince1900 = word(PacketBuffer[40], PacketBuffer[41]) << 16 | word(PacketBuffer[42], PacketBuffer[43]);
      
      UDPConnection.stop();
      return (SecsSince1900 - SeventyYears);
    }
    else
    {
      UDPConnection.stop();
      return 0;
    }
  }

private:
  void RequestTime()
  {
    uint8_t PacketBuffer[NTP_PACKET_SIZE] = {};
    PacketBuffer[0] = 0b11100011;
    PacketBuffer[1] = 0;
    PacketBuffer[2] = 6;
    PacketBuffer[3] = 0xEC;
    PacketBuffer[12] = 49;
    PacketBuffer[13] = 0x4E;
    PacketBuffer[14] = 49;
    PacketBuffer[15] = 52;

    static const IPAddress TimeServer(129, 6, 15, 28);
    UDPConnection.beginPacket(TimeServer, 123);
    UDPConnection.write(PacketBuffer, NTP_PACKET_SIZE);
    UDPConnection.endPacket();
  }
  
  WiFiUDP UDPConnection;
  static const size_t NTP_PACKET_SIZE = 48;
};
