#pragma once
#include "Definitions.h"
#include "Tuple.h"
#include "Display.h"
#include "Bitmaps.h"

#include <ArduinoJson.h>
#include "FirmataConnection.h"

class Capacitor
{
public:
  static bool DetectCapacitor()
  {
    for (size_t Index = 0; Index != ArrayCount(MutuallyExclusivePins); ++Index)
    {
      auto
        FirstPin = MutuallyExclusivePins[Index].First,
        SecondPin = MutuallyExclusivePins[Index].Second,
        ThirdPin = MutuallyExclusivePins[Index].Third;
      auto Readings = SetUpAndExclusivelyMeasureCapacitance(FirstPin, SecondPin, ThirdPin);
      
      if (Readings.First)
      {
        Display::GetInstance().setCursor(48, 16);        
        Display::GetInstance().println("Capacitor");

        // Resistance
        Display::GetInstance().setCursor(48, 32);
        Display::GetInstance().println(String("C: ") + DoubleToString(Readings.Second) + static_cast<char>(229) + "F");

        // Pin 1
        Display::GetInstance().setCursor(4, 32);
        Display::GetInstance().print(SecondPin);

        // Pin 2
        Display::GetInstance().setCursor(39, 32);
        Display::GetInstance().print(FirstPin);

        Display::GetInstance().drawBitmap(0, 16, POLARISED_CAPACITOR, 48, 48, WHITE);

        StaticJsonBuffer<JSON_OBJECT_SIZE(2)> Buffer;
        auto & Root = Buffer.createObject();
        Root["Component"] = "Capacitor";
        Root["Capacitance"] = Readings.Second;
        FirmataConnection::SendResultToHost(Root);
        return true;
      }
    }
    
    return false;
  }
  
  static Pair<bool, double> MeasureCapacitance(uint8_t FromPin, uint8_t ToPin)
  {
    unsigned int PreviousCapacitorVoltage = 0, CurrentCapacitorVoltage = 0, VoltageDelta = 0, MeasurementCount = 0;
    unsigned long TimeEnd, AnalogReadTimeStart, AnalogReadTimeEnd;
    
    DischargeCapacitor(FromPin, ToPin);
    
    digitalWrite(FromPin, HIGH);
    digitalWrite(ToPin, LOW);
  
    if ((analogRead(AnalogReadMappings[FromPin]) - analogRead(AnalogReadMappings[ToPin])) >= 648)
    {
      return { false };
    }

    auto TimeStart = micros();
    while (
      AnalogReadTimeStart = micros(),
      CurrentCapacitorVoltage = analogRead(AnalogReadMappings[FromPin]) - analogRead(AnalogReadMappings[ToPin]),
      AnalogReadTimeEnd = micros(),

      TimeStart += (AnalogReadTimeEnd - AnalogReadTimeStart),
      TimeEnd = micros(),
      (CurrentCapacitorVoltage < 648)
    )
    {
      MeasurementCount++;
      VoltageDelta += CurrentCapacitorVoltage - PreviousCapacitorVoltage;
      PreviousCapacitorVoltage = CurrentCapacitorVoltage;
  
      if ((TimeEnd - TimeStart) > 1000000)
      {
        return { false };
      }
    }

    if ((static_cast<double>(VoltageDelta) / MeasurementCount) < 0.5)
    {
      return { false };
    }
  
    return { true, (TimeEnd - TimeStart) / (PinResistances[FromPin] + PinResistances[ToPin]) * 1000 };  
  }

private:
  static void DischargeCapacitor(uint8_t FromPin, uint8_t ToPin)
  {  
    digitalWrite(FromPin, LOW);
    digitalWrite(ToPin, LOW);

    while (IsReadingValid(MeasureVoltage(FromPin, ToPin) / 1.f))
    {
      delay(5);
    }
  }
  
  static Pair<bool, double> SetUpAndExclusivelyMeasureCapacitance(uint8_t FromPin, uint8_t ToPin, uint8_t ExcludePin)
  {
    pinMode(ExcludePin, INPUT);
    pinMode(FromPin, OUTPUT);
    pinMode(ToPin, OUTPUT);
  
    auto Readings = MeasureCapacitance(FromPin, ToPin);
    DischargeCapacitor(FromPin, ToPin);
    
    pinMode(ExcludePin, OUTPUT);
    pinMode(FromPin, OUTPUT);
    pinMode(ToPin, OUTPUT);
  
    return Readings;
  }
};

