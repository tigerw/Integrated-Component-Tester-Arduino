#pragma once
#include "Definitions.h"

class Resistor
{
public:  
  static bool DetectResistor()
  {
    for (size_t Index = 0; Index != ArrayCount(MutuallyExclusivePins); ++Index)
    {
      auto
        FirstPin = MutuallyExclusivePins[Index].First,
        SecondPin = MutuallyExclusivePins[Index].Second,
        ThirdPin = MutuallyExclusivePins[Index].Third;
      
      if (IsReadingValid(SetUpAndExclusivelyMeasureCurrent(FirstPin, SecondPin, ThirdPin)))
      {
        Display::GetInstance().setCursor(48, 16);        
        Display::GetInstance().println("Resistor");

        // Resistance
        auto Resistance = SetUpAndMeasureResistance(FirstPin, SecondPin);
        Display::GetInstance().setCursor(48, 32);
        Display::GetInstance().println(String("R: ") + DoubleToString(Resistance) + static_cast<char>(233));
  
        // Voltage drop
        auto VDrop = MeasureVoltage(FirstPin, SecondPin);
        Display::GetInstance().setCursor(48, 40);
        Display::GetInstance().println(String("Vdrop: ") + DoubleToString(VDrop) + "V");

        // Pin 1
        Display::GetInstance().setCursor(4, 32);
        Display::GetInstance().print(FirstPin);

        // Pin 2
        Display::GetInstance().setCursor(39, 32);
        Display::GetInstance().print(SecondPin);

        Display::GetInstance().drawBitmap(0, 16, RESISTOR, 48, 48, WHITE);
        
        StaticJsonBuffer<JSON_OBJECT_SIZE(3)> Buffer;
        auto & Root = Buffer.createObject();
        Root["Component"] = "Resistor";
        Root["Resistance"] = Resistance;
        Root["Voltage drop"] = VDrop;
        FirmataConnection::SendResultToHost(Root);
        return true;
      }
    }
  
    return false;
  }

private:
  static double SetUpAndMeasureResistance(uint8_t FromPin, uint8_t ToPin)
  {
    digitalWrite(FromPin, HIGH);
    digitalWrite(ToPin, LOW);
    delay(StablisationDelay);
    
    double AverageResistance = 0;
    for (auto Counter = 0; Counter != 100; ++Counter)
    {
      double ProbedComponentVoltage = MeasureVoltage(FromPin, ToPin);
      double Current = (RefVoltage - ProbedComponentVoltage) / (PinResistances[FromPin] + PinResistances[ToPin]);
      AverageResistance += ProbedComponentVoltage / Current;
    }
    
    digitalWrite(FromPin, LOW);
    digitalWrite(ToPin, LOW);
    
    return (AverageResistance / 100.f);
  }
};

