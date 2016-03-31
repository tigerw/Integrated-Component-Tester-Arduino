#pragma once
#include "Definitions.h"
#include "Tuple.h"
#include "Capacitor.h"
#include "Bitmaps.h"

class Diode
{
public:
  static bool DetectDiode()
  {
    for (size_t Index = 0; Index != ArrayCount(MutuallyExclusivePins); ++Index)
    {
      auto
        FirstPin = MutuallyExclusivePins[Index].First,
        SecondPin = MutuallyExclusivePins[Index].Second,
        ThirdPin = MutuallyExclusivePins[Index].Third;
      auto ForwardReadings = SetUpAndExclusivelyMeasureDiode(FirstPin, SecondPin, ThirdPin);
      auto ReverseReadings = SetUpAndExclusivelyMeasureDiode(SecondPin, FirstPin, ThirdPin);
      auto Readings = ForwardReadings.First ? ForwardReadings : ReverseReadings;
      
      if (Readings.First)
      {
        Display::GetInstance().setCursor(48, 16);        
        Display::GetInstance().println("Diode");

        // Current in reverse bias
        Display::GetInstance().setCursor(48, 32);
        Display::GetInstance().println(String("Ir: ") + DoubleToString(Readings.Second) + "mA");
  
        // Voltage drop in forward bias
        Display::GetInstance().setCursor(48, 40);
        Display::GetInstance().println(String("Vf: ") + DoubleToString(Readings.Third) + "V");
  
        // Capacitance
        Display::GetInstance().setCursor(48, 48);
        Display::GetInstance().println(String("C: ") + (Readings.Fourth.First ? DoubleToString((Readings.Fourth.Second)) : "(?)") + static_cast<char>(229) + "F");

        // Anode
        Display::GetInstance().setCursor(4, 32);
        Display::GetInstance().print(ForwardReadings.First ? FirstPin : SecondPin);

        // Cathode
        Display::GetInstance().setCursor(39, 32);
        Display::GetInstance().print(ForwardReadings.First ? SecondPin : FirstPin);

        Display::GetInstance().drawBitmap(0, 16, DIODE, 48, 48, WHITE);
        
        StaticJsonBuffer<JSON_OBJECT_SIZE(4)> Buffer;
        auto & Root = Buffer.createObject();
        Root["Component"] = "Diode";
        Root["Reverse current"] = Readings.Second;
        Root["Forward voltage"] = Readings.Third;
        Root["Capacitance"] = Readings.Fourth.First ? Readings.Fourth.Second : 0;
        FirmataConnection::SendResultToHost(Root);
        return true;
      }
    }

    return false;
  }

private:
  static Quadruple<bool, double, double, Pair<bool, double>> SetUpAndExclusivelyMeasureDiode(uint8_t FromPin, uint8_t ToPin, uint8_t ExcludePin)
  {    
    pinMode(ExcludePin, INPUT);
    pinMode(FromPin, OUTPUT);
    pinMode(ToPin, OUTPUT);
  
    auto Readings = MeasureDiode(FromPin, ToPin);
    
    pinMode(ExcludePin, OUTPUT);
    pinMode(FromPin, OUTPUT);
    pinMode(ToPin, OUTPUT);
  
    return Readings;  
  }

  static Quadruple<bool, double, double, Pair<bool, double>> MeasureDiode(uint8_t FromPin, uint8_t ToPin)
  {
    digitalWrite(FromPin, HIGH);
    digitalWrite(ToPin, LOW);
    delay(StablisationDelay);
    
    auto ForwardCurrent = MeasureCurrent(FromPin, ToPin);
    auto ForwardVoltage = MeasureVoltage(FromPin, ToPin);
    
    digitalWrite(FromPin, LOW);
    digitalWrite(ToPin, HIGH);
    delay(StablisationDelay);

    auto ReverseCurrent = MeasureCurrent(ToPin, FromPin);

    if (!IsReadingValid((ForwardCurrent - ReverseCurrent) * 10.f))
    {
      digitalWrite(FromPin, LOW);
      digitalWrite(ToPin, LOW);
      return { false };
    }

    return { true, ReverseCurrent * 1000, ForwardVoltage, Capacitor::MeasureCapacitance(FromPin, ToPin) };
  }
};

