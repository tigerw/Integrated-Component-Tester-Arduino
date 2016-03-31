#pragma once
#include "Definitions.h"
#include "Bitmaps.h"

class Transistor
{
public:
  static bool DetectNPN()
  {
    auto PriorCurrent = SetUpAndExclusivelyMeasureCurrent(2, 4, 3);
    
    digitalWrite(2, HIGH);
    digitalWrite(4, LOW);
    digitalWrite(3, HIGH);
    delay(StablisationDelay);
    
    auto NewCurrent = MeasureCurrent(2, 4);    
    auto TransistorMeasurements = MeasureHFEAndVCE();
    
    digitalWrite(3, LOW);
    digitalWrite(2, LOW);
    digitalWrite(4, LOW);

  
    // Check that:
    // NewCurrent is greater than PriorCurrent - subtraction is greater than zero
    // Former is significantly larger than the latter - subtraction is greater than a positive constant (defined by IsCurrentReadingValid)
    // Prevents confusion when other components are connected and there are small fluctuations between readings that are actually (in theory) equal
    if (IsReadingValid((NewCurrent - PriorCurrent) * 10.f) && (TransistorMeasurements.First >= 100.f))
    {
      Display::GetInstance().setCursor(48, 16);
      if (TransistorMeasurements.First > 500.f)
      {
        Display::GetInstance().println("DLTN BJT-NPN");
      }
      else
      {      
        Display::GetInstance().println("BJT-NPN");
      }

      // Gain
      Display::GetInstance().setCursor(48, 32);
      Display::GetInstance().println(String("hFE: ") + DoubleToString(TransistorMeasurements.First));

      // Voltage drop
      Display::GetInstance().setCursor(48, 40);
      Display::GetInstance().println(String("Vce: ") + DoubleToString(TransistorMeasurements.Second) + "V");

      // Activation voltage
      auto VActivation = FindActivationVoltage();
      Display::GetInstance().setCursor(48, 48);
      Display::GetInstance().println(String("Vact: ") + DoubleToString(VActivation) + "V");

      // Collector
      Display::GetInstance().setCursor(23, 18);
      Display::GetInstance().print(2);

      // Base
      Display::GetInstance().setCursor(2, 44);
      Display::GetInstance().print(3);

      // Emitter
      Display::GetInstance().setCursor(23, 55);
      Display::GetInstance().print(4);
      
      Display::GetInstance().drawBitmap(0, 16, BJT_NPN, 48, 48, WHITE);
      
      StaticJsonBuffer<JSON_OBJECT_SIZE(4)> Buffer;
      auto & Root = Buffer.createObject();
      Root["Component"] = "NPN Transistor";
      Root["Gain"] = TransistorMeasurements.First;
      Root["Collector-emitter voltage"] = TransistorMeasurements.Second;
      Root["Activation voltage"] = VActivation;
      FirmataConnection::SendResultToHost(Root);
      return true;
    }
    
    return false;
  }

private:
  static Pair<double, double> MeasureHFEAndVCE()
  {
    return { /* HFE */ MeasureCurrent(2, 4) / MeasureCurrent(3, 4), /* Vce */ MeasureVoltage(2, 4) };
  }

  static double FindActivationVoltage()
  {
    digitalWrite(2, HIGH);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);

    auto PriorCurrent = MeasureCurrent(2, 4);
    unsigned char Value = 0;

    for (; Value != 255; ++Value)
    {
      analogWrite(3, Value);
      if (IsReadingValid(MeasureCurrent(2, 4) - PriorCurrent))
      {
        break;
      }
    }

    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);

    return (Value / 255.f) * RefVoltage;
  }
};

