#pragma once

#include <array>
#include <optional>
#include <vector>

using FrequencyHertz = float;
using Time = size_t;

struct Interception
{
  FrequencyHertz frequency{0}; //Frequency of transmission
  Time time{0};
};

using Callsign = std::array<char, 3>;
using Discriminant = std::array<char, 3>;
using IndicatorSetting = std::array<char, 3>;

struct Preamble
{
  Callsign from = {0,0,0}; //Sending station
  std::vector<Callsign> to; //Destination stations
  Time timeOfOrigin{0};
  size_t part{0};
  size_t numParts{0};
  Discriminant discriminant = {0,0,0}; //Distinguish between different types of Enimga traffic. It indicates which of many current keys was being used
  IndicatorSetting indicatorSetting = {0,0,0}; //Used for encoding and decoding
};

class TextChar
{
  char c_;
  TextChar(char c);
public:
  static std::optional<TextChar> Create(char c); //Allow A-Z
  char ToChar() const;
};

using Key = TextChar;
using Lamp = TextChar;
using EncipheredText = std::vector<TextChar>;

struct EnigmaMessage
{
  Interception interception;
  Preamble preamble;
  EncipheredText encipheredText;
};

class ScramblerUnit
{
};
class SteckerBoard
{
};
class Machine
{
  ScramblerUnit scramblerUnit_;
  SteckerBoard steckerBoard_;

public:
  Lamp Press(Key key) const;
};

