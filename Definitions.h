#pragma once
#include "Tuple.h"

static const double PinResistances[] = {
  // Bad practice
  0,
  0,
  560,
  220000,
  560
};

static const uint8_t AnalogReadMappings[] = {
  // Really bad practice
  255,
  255,
  A1,
  A2,
  A3
};

static const Triple<uint8_t, uint8_t, uint8_t> MutuallyExclusivePins[] = {
  { 2, 3, 4 },
  { 3, 4, 2 },
  { 2, 4, 3 }
};

static const unsigned long StablisationDelay = 100;
static const double RefVoltage = 3.3;
static const uint32_t MaxADCValue = 4095;

template <typename Type>
String TypeToPaddedString(Type Value, int PaddingCount)
{ // ugh, not ideal
  if (Value < 10 * PaddingCount)
  {
    String Padded;
    for (int Pad = 0; Pad != PaddingCount; ++Pad)
    {
      Padded += '0';
    }
    return Padded + Value;
  }
  return String(Value);
}

String DoubleToString(double Value)
{ // ughhhh, really not ideal, but the MKR1000 STRING CONSTRUCTOR TAKING FLOATING POINT VALUES IS **BROKEN**!! COME ON!
  String Return;
  int intPart = int(Value);
  long decPart = 100.f*(Value-intPart);
  Return += intPart;

  if(decPart>0)return(Return + '.' + decPart);
  else if(decPart<0)return(Return + '.' + (-1)*decPart);
  else return(Return + '.' + 00);
}

bool IsReadingValid(double Reading)
{
  return (!isnan(Reading) && (Reading > 0.00001));
}

template <typename ArrayType, size_t Count>
size_t ArrayCount(ArrayType (&Array)[Count])
{
  return Count;
}

double MeasureVoltage(uint8_t FromPin, uint8_t ToPin)
{
  return static_cast<double>((analogRead(AnalogReadMappings[FromPin]) - analogRead(AnalogReadMappings[ToPin]))) / MaxADCValue * RefVoltage;
}

double MeasureCurrent(uint8_t FromPin, uint8_t ToPin)
{    
    double AverageCurrent = 0;
    for (auto Counter = 0; Counter != 100; ++Counter)
    {
      double ProbedComponentVoltage = MeasureVoltage(FromPin, ToPin);
      double Current = (RefVoltage - ProbedComponentVoltage) / (PinResistances[FromPin] + PinResistances[ToPin]);
      AverageCurrent += Current;
    }
  
    return (AverageCurrent / 100.f);
}

double SetUpAndExclusivelyMeasureCurrent(uint8_t FromPin, uint8_t ToPin, uint8_t ExcludePin)
{
  pinMode(ExcludePin, INPUT);
  pinMode(FromPin, OUTPUT);
  pinMode(ToPin, OUTPUT);
  
  digitalWrite(FromPin, HIGH);
  digitalWrite(ToPin, LOW);
  delay(StablisationDelay);

  auto Current = MeasureCurrent(FromPin, ToPin);
  
  digitalWrite(FromPin, LOW);
  digitalWrite(ToPin, LOW);
  
  pinMode(ExcludePin, OUTPUT);
  pinMode(FromPin, OUTPUT);
  pinMode(ToPin, OUTPUT);

  return Current;
}
