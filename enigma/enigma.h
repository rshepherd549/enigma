#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

#include "util.h"

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

static constexpr size_t c_numChars = 26;

using Terminal = IntRange<unsigned char, 0, c_numChars>;

struct LeftTerminal
{
  Terminal terminal;
  LeftTerminal Inc(int inc) const;
};
bool operator==(const LeftTerminal& left1, const LeftTerminal& left2);

struct RightTerminal
{
  Terminal terminal;
  RightTerminal Inc(int inc) const;
};

using TextChar = IntRange<char, 'A', c_numChars>;
using Key = TextChar;
using Lamp = TextChar;
using EncipheredText = std::vector<TextChar>;

struct EnigmaMessage
{
  Interception interception;
  Preamble preamble;
  EncipheredText encipheredText;
};

class Connections
{
protected:
  std::array<LeftTerminal, c_numChars> rightToLeft_;
  Connections(std::array<LeftTerminal, c_numChars> rightToLeft);
public:
  static std::optional<Connections> Create(std::array<LeftTerminal, c_numChars> rightToLeft);
  static std::optional<Connections> Create(std::array<unsigned char, c_numChars> rightToLeft);
  static Connections CreateIdentity();

  LeftTerminal ToLeft(RightTerminal right) const;
  RightTerminal ToRight(LeftTerminal left) const;
};

class CrossConnections: public Connections
{
  CrossConnections(std::array<LeftTerminal, c_numChars> connections);
public:
  static std::optional<CrossConnections> Create(std::array<LeftTerminal, c_numChars> connections);
  static CrossConnections CreateReverse();

  LeftTerminal TurnAround(LeftTerminal left) const;
};

class Wheel
{
  const Connections& connections_;
  Key ringSetting_;
  size_t rotation_{0};

  size_t TotalRotation_() const;
public:
  Wheel(const Connections& connections, Key ringSetting);
  LeftTerminal ToLeft(RightTerminal right) const;
  RightTerminal ToRight(LeftTerminal left) const;
  size_t Inc(size_t inc);
};

class TurnAboutWheel
{
  CrossConnections crossConnections_;
public:
  TurnAboutWheel(CrossConnections crossConnections);
  LeftTerminal TurnAround(LeftTerminal in) const;
};

class Commutator
{
public:
  LeftTerminal ToTerminal(Key key) const;
  Lamp ToLamp(LeftTerminal left) const;
};

constexpr size_t numScramblerWheels = 3;
class Scrambler
{
  TurnAboutWheel turnAroundWheel_;
  std::array<Wheel,numScramblerWheels> wheels_; //left to right
  Commutator commutator_;
public:
  Scrambler(CrossConnections crossConnections,
            std::array<Wheel,numScramblerWheels> wheels);
  Lamp ToLamp(Key in);
};

constexpr size_t numMachineWheels = 5;
using WheelIndex = IntRange<unsigned char, 0, numMachineWheels>;
struct WheelSelection
{
  WheelIndex wheelIndex;
  Key ringSetting;
  WheelSelection(IntRange<unsigned char, 0, numMachineWheels> wheelIndex,
                 Key ringSetting);
};
struct CrossPlugging
{
  Key lhs;
  Key rhs;
};
using CrossPluggings = std::vector<CrossPlugging>;
class PlugBoard
{
  std::array<Key, c_numChars> crossPluggings_;
  PlugBoard(const CrossPluggings& crossPluggings);
  static bool IsValid_(const CrossPluggings& crossPluggings);
public:
  static std::optional<PlugBoard> Create(const CrossPluggings& crossPluggings);
  Key Cross(Key key) const;
};
class Machine
{
  std::array<Connections,numMachineWheels> connectionss_;
  Scrambler scrambler_;
  PlugBoard plugBoard_;

public:
  Machine(CrossConnections crossConnections,
          std::array<Connections,numMachineWheels> connectionss,
          std::array<WheelSelection,numScramblerWheels> wheels,
          PlugBoard plugBoard);
  Lamp ToLamp(Key key);
  std::string ToLamp(const std::string_view keys);
};
