#pragma once
#include <ArduinoJson.h>
#include <Firmata.h>

class FirmataConnection
{
public:
  static void begin()
  {
    Firmata.setFirmwareVersion(FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);
    Firmata.attach(START_SYSEX, SystemExtensionRequestCallback);
    Firmata.begin(9600);
  }

  static void ProcessData()
  {
    while (Firmata.available())
    {
      Firmata.processInput();
    }
  }

  static void SendResultToHost(JsonObject & ResultantObject)
  {
    char Buffer[256];
    ResultantObject.printTo(Buffer, sizeof(Buffer));
    for (int i = 0; i != 5; ++i)
    Firmata.sendString(Buffer);
  }

private:
  static void SystemExtensionRequestCallback(uint8_t Command, uint8_t ArgumentCount /* argc */ , uint8_t * ArgumentVector /* argv */)
  {
    switch (Command)
    {
      case CAPABILITY_QUERY:
      {
        Firmata.write(START_SYSEX);
        Firmata.write(CAPABILITY_RESPONSE);
        // No capabilities, innit.
        Firmata.write(127);
        Firmata.write(END_SYSEX);
        break;
      }
      default:
      {
        Firmata.sendString("Unsupported command.");
        break;
      }
    }
  }
};

