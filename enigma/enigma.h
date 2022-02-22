#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

#include "util.h"

static constexpr size_t c_numChars = 26;
static_assert(c_numChars % 2 == 0); //Check even
static constexpr size_t c_numCharsBy2 = c_numChars/2;

using TextChar = IntRange<char, 'A', c_numChars>;
using Key = TextChar;
using Lamp = TextChar;
using EncipheredText = std::vector<TextChar>;

using FrequencyHertz = float;
using Time = size_t;

struct Interception
{
  FrequencyHertz frequency{0}; //Frequency of transmission
  Time time{0};
};

using Callsign = std::array<TextChar, 3>;
using Discriminant = std::array<TextChar, 3>;
using IndicatorSetting = std::array<TextChar, 3>;

struct Preamble
{
  Callsign from{}; //Sending station
  std::vector<Callsign> to; //Destination stations
  Time timeOfOrigin{0};
  size_t part{0};
  size_t numParts{0};
  Discriminant discriminant{}; //Distinguish between different types of Enimga traffic. It indicates which of many current keys was being used
  IndicatorSetting indicatorSetting{}; //Used for encoding and decoding
};

struct EnigmaMessage
{
  Interception interception;
  Preamble preamble;
  EncipheredText encipheredText;
};

using Terminal = IntRange<unsigned char, 0, c_numChars>;

struct LeftTerminal
{
  Terminal terminal;
  LeftTerminal operator+(int inc) const;
  LeftTerminal operator-(int inc) const;
};
bool operator==(const LeftTerminal& left1, const LeftTerminal& left2);

struct RightTerminal
{
  Terminal terminal;
  RightTerminal operator+(int inc) const;
  RightTerminal operator-(int inc) const;
};

class Connections
{
protected:
  std::array<LeftTerminal, c_numChars> rightToLeft_;
  Connections(std::array<LeftTerminal, c_numChars> rightToLeft);
public:
  static std::optional<Connections> Create(std::array<LeftTerminal, c_numChars> rightToLefts);
  static Connections Create();

  LeftTerminal ToLeft(RightTerminal right) const;
  RightTerminal ToRight(LeftTerminal left) const;
};

class CrossConnection
{
  LeftTerminal from_;
  LeftTerminal to_;
  CrossConnection(LeftTerminal from, LeftTerminal to);
public:
  CrossConnection();
  static std::optional<CrossConnection> Create(LeftTerminal from, LeftTerminal to);
  LeftTerminal From() const;
  LeftTerminal To() const;
};

class CrossConnections: public Connections
{
  CrossConnections(Connections connections);
public:
  static std::optional<CrossConnections> Create(std::array<CrossConnection, c_numChars/2> crossConnections);
  static CrossConnections CreateReverse();

  LeftTerminal Transform(LeftTerminal left) const;
};

class Wheel
{
  const Connections& connections_;
public:
  Wheel(const Connections& connections);
  LeftTerminal ToLeft(RightTerminal right) const;
  RightTerminal ToRight(LeftTerminal left) const;
};

class Rotor
{
  std::reference_wrapper<const Wheel> wheel_;
  size_t rotation_{0};

  size_t TotalRotation_() const;
public:
  Rotor(const Wheel& wheel, Key ringSetting);
  LeftTerminal ToLeft(RightTerminal right) const;
  RightTerminal ToRight(LeftTerminal left) const;
  size_t Inc(size_t inc);
};

class TurnAboutWheel
{
  CrossConnections crossConnections_;
public:
  TurnAboutWheel(CrossConnections crossConnections);
  LeftTerminal Transform(LeftTerminal in) const;
};

class Commutator
{
public:
  LeftTerminal ToTerminal(Key key) const;
  Lamp ToLamp(LeftTerminal left) const;
};

constexpr size_t numMachineWheels = 5;
constexpr size_t numScramblerRotors = 3;

using WheelIndex = IntRange<unsigned char, 0, numMachineWheels>;
struct WheelSelection
{
  WheelIndex wheelIndex;
  Key ringSetting;
  WheelSelection(WheelIndex wheelIndex,
                 Key ringSetting);
};

class Scrambler
{
  TurnAboutWheel turnAroundWheel_;
  std::array<Rotor, numScramblerRotors> rotors_; //left to right
  Commutator commutator_;
public:
  Scrambler(TurnAboutWheel turnAroundWheel,
            const std::array<Wheel,numMachineWheels>& wheels);
  void Configure(const std::array<Wheel,numMachineWheels>& wheels,
                 std::array<WheelSelection,numScramblerRotors> selections);
  Lamp ToLamp(Key in);
};

struct Plug
{
  Key lhs;
  Key rhs;
};
using Plugs = std::vector<Plug>;
class PlugBoard
{
  Connections connections_;
  PlugBoard(Connections connections);
public:
  static std::optional<PlugBoard> Create(const Plugs& plugs);
  Key Transform(Key key) const;
};
class Machine
{
  std::array<Wheel,numMachineWheels> wheels_;
  Scrambler scrambler_;
  PlugBoard plugBoard_;

public:
  Machine(TurnAboutWheel turnAboutWheel,
          std::array<Wheel, numMachineWheels> wheels);
  void Configure(std::array<WheelSelection, numScramblerRotors> selections,
                 PlugBoard plugBoard);
  Lamp ToLamp(Key key);
  std::string ToLamp(const std::string_view keys);
};
