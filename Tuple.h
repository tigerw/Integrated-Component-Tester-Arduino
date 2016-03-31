#pragma once

template <typename FirstPairType, typename SecondPairType>
struct Pair
{
  FirstPairType First;
  SecondPairType Second;
};

template <typename FirstTripleType, typename SecondTripleType, typename ThirdTripleType>
struct Triple
{
  FirstTripleType First;
  SecondTripleType Second;
  ThirdTripleType Third;
};

template <typename FirstQuadrupleType, typename SecondQuadrupleType, typename ThirdQuadrupleType, typename FourthQuadrupleType>
struct Quadruple
{
  FirstQuadrupleType First;
  SecondQuadrupleType Second;
  ThirdQuadrupleType Third;
  FourthQuadrupleType Fourth;
};
