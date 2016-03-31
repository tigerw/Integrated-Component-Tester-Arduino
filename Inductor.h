#pragma once
#include "Definitions.h"

class Inductor
{
public:
  static bool DetectInductor()
  {
    for (size_t Index = 0; Index != ArrayCount(MutuallyExclusivePins); ++Index)
    {
      auto
        FirstPin = MutuallyExclusivePins[Index].First,
        SecondPin = MutuallyExclusivePins[Index].Second,
        ThirdPin = MutuallyExclusivePins[Index].Third;
        
      if (SetUpAndExclusivelyMeasureInductance(FirstPin, SecondPin, ThirdPin))
      {
        return true;
      }
    }

    return false;
  }

private:
  static bool MeasureInductance(uint8_t FromPin, uint8_t ToPin)
  {
    digitalWrite(FromPin, HIGH);
    digitalWrite(ToPin, LOW);
    delay(100);
    
    digitalWrite(FromPin, LOW);
    digitalWrite(ToPin, LOW);
    delay(StablisationDelay);
    
    auto PreviousVoltage = MeasureVoltage(FromPin, ToPin);
    auto PreviousCurrent = MeasureCurrent(FromPin, ToPin);
    delay(1);
    auto NewVoltage = MeasureVoltage(FromPin, ToPin);
    auto NewCurrent = MeasureCurrent(FromPin, ToPin);
  
    auto Inductance = ((PreviousVoltage + NewVoltage) / 2) / (PreviousCurrent - NewCurrent);
    if (IsReadingValid(Inductance))
    {
      Display::GetInstance().println(String("Inductor found between pins ") + FromPin + " and " + ToPin);
      Display::GetInstance().println(String("Inductance: ") + Inductance + "mH");
      return true;
    }
  
    return false;
  }
  
  static bool SetUpAndExclusivelyMeasureInductance(uint8_t FromPin, uint8_t ToPin, uint8_t ExcludePin)
  {
    pinMode(ExcludePin, INPUT);
    pinMode(FromPin, OUTPUT);
    pinMode(ToPin, OUTPUT);
  
    bool Readings = MeasureInductance(FromPin, ToPin);
    
    pinMode(ExcludePin, OUTPUT);
    pinMode(FromPin, OUTPUT);
    pinMode(ToPin, OUTPUT);
  
    return Readings;  
  }
};

