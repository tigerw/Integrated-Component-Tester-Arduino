#include <math.h>
#include "SAMD_AnalogCorrection.h"
#include <RTCZero.h>

#include "Battery.h"
#include "Capacitor.h"
#include "Definitions.h"
#include "Diode.h"
#include "Display.h"
#include "FirmataConnection.h"
#include "Inductor.h"
#include "Resistor.h"
#include "Transistor.h"
#include "WiFi.h"

RTCZero InternalRTC;
WiFiRTPConnection WiFiRTP;
auto LastTimeUpdate = millis();
bool LastStatusBarDrawState = false;

void setup()
{
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  
  pinMode(2 , OUTPUT);
  pinMode(3 , OUTPUT);
  pinMode(4 , OUTPUT);

  FirmataConnection::begin();
  Display::begin();
  Display::GetInstance().setTextColor(WHITE);
  
  Display::GetInstance().setCursor(0, 8);
  for (uint8_t i=0; i != 21; i++)
  {
    Display::GetInstance().write(248);
  }

  analogReadResolution(12);
  analogReadCorrection(17, 2057);
  
  InternalRTC.begin();
  WiFiRTP.begin();
}

void loop()
{
  FirmataConnection::ProcessData();
  
  if (millis() - LastTimeUpdate > 5000)
  {
    auto Epoch = WiFiRTP.GetEpoch();
    if (Epoch != 0)  // Not exactly what should be done - epoch could be zero, in theory - but workable in practice
    {
      InternalRTC.setEpoch(Epoch);
    }
    LastTimeUpdate = millis();

    Display::ClearHeaderArea();
    Display::GetInstance().setCursor(0, 0);

    if (LastStatusBarDrawState)
    {
      auto RSSI = WiFiRTP.GetRSSI();
      Display::GetInstance().drawBitmap(0, 0, ((RSSI < -100) ? WIFI_LOW : ((RSSI < -50) ? WIFI_MEDIUM : WIFI_HIGH)), 8, 8, WHITE);
    }
    else
    {  
      Display::GetInstance().print(
        TypeToPaddedString(InternalRTC.getHours(), 1) +
        ':' +
        TypeToPaddedString(InternalRTC.getMinutes(), 1)
      );
    }

    Display::GetInstance().setCursor(36, 0);
    Display::GetInstance().print(
      String("20") +
      TypeToPaddedString(InternalRTC.getYear(), 1) +
      '-' +
      TypeToPaddedString(InternalRTC.getMonth(), 1) +
      '-' +
      TypeToPaddedString(InternalRTC.getDay(), 1) +
      ' ' +
      TypeToPaddedString(round(Battery::MeasureBatteryCapacity() / RefVoltage * 100.f), 2) +
      '%'
    );
    
    LastStatusBarDrawState = !LastStatusBarDrawState;
  }
  
  Display::GetInstance().display();
  Display::GetInstance().startscrollright(1, 1);
  
  Display::GetInstance().setCursor(0, 16);
  Display::ClearNonHeaderArea();
  
  if (Transistor::DetectNPN())
  {
  }
  else if (Capacitor::DetectCapacitor())
  {
  }
  else if (false && Inductor::DetectInductor())
  {
  }
  else if (Diode::DetectDiode())
  {
  }
  else if (Resistor::DetectResistor())
  {
  }
  else
  {
    Display::GetInstance().drawBitmap(40, 16, NONE, 48, 48, WHITE);
    Display::GetInstance().display();
    Display::GetInstance().stopscroll();
    
    StaticJsonBuffer<JSON_OBJECT_SIZE(1)> Buffer;
    auto & Root = Buffer.createObject();
    Root["Component"] = "N/A";
    FirmataConnection::SendResultToHost(Root);
  }

  Display::GetInstance().display();
}

