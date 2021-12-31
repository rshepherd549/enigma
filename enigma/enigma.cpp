#include "pch.h"
#include <algorithm>
#include <numeric>
#include "enigma.h"
#include "framework.h"

bool operator==(const LeftTerminal& left1, const LeftTerminal& left2)
{
  return left1.terminal == left2.terminal;
}
LeftTerminal LeftTerminal::Inc(int inc) const
{
  terminal.Inc(inc);
  return *this;
};
RightTerminal RightTerminal::Inc(int inc) const
{
  terminal.Inc(inc);
  return *this;
};

Connections::Connections(std::array<LeftTerminal, c_numChars> rightToLeft):
  rightToLeft_{std::move(rightToLeft)}
{
}

/*static*/ std::optional<Connections> Connections::Create(std::array<LeftTerminal, c_numChars> rightToLeft)
{
  return all_unique(rightToLeft.cbegin(), rightToLeft.cend())
       ? Connections{std::move(rightToLeft)}
       : std::optional<Connections>{};
}

/*static*/ std::optional<Connections> Connections::Create(std::array<unsigned char, c_numChars> rightToLeft_)
{
  std::array<LeftTerminal, c_numChars> rightToLeft;

  for (size_t i = 0; i != c_numChars; ++i)
  {
    const auto terminal = Terminal::Create(rightToLeft_[i]);
    if (!terminal)
      return {};
    rightToLeft[i] = LeftTerminal{*terminal};
  }

  return Create(rightToLeft);
}

/*static*/ Connections Connections::CreateIdentity()
{
  std::array<LeftTerminal, c_numChars> rightToLeft;
  for (unsigned char i = 0; i != c_numChars; ++i)
    rightToLeft[i] = LeftTerminal{*Terminal::Create(i)};
  return Connections{std::move(rightToLeft)};
}

LeftTerminal Connections::ToLeft(RightTerminal right) const
{
  return rightToLeft_[right.terminal.Value()];
}

RightTerminal Connections::ToRight(LeftTerminal left) const
{
  const auto left_itr = std::find(rightToLeft_.cbegin(), rightToLeft_.cend(), left);
  const auto left_idx = std::distance(rightToLeft_.cbegin(), left_itr);
  return RightTerminal{*Terminal::Create(static_cast<unsigned char>(left_idx))};
}

CrossConnections::CrossConnections(std::array<LeftTerminal, c_numChars> connections):
  Connections{std::move(connections)}
{
}

//Ensure connections are paired i.e. if i -> j then j -> i
/*static*/ std::optional<CrossConnections> CrossConnections::Create(std::array<LeftTerminal, c_numChars> connections)
{
  if (!all_unique(connections.cbegin(), connections.cend()))
    return {};

  const CrossConnections crossConnections{connections};

  for (Terminal::value_type terminal = 0; terminal != c_numChars; ++terminal)
  {
    const LeftTerminal left{*Terminal::Create(terminal)};
    if (crossConnections.ToLeft(crossConnections.ToRight(left)) != left)
      return {};
  }

  return crossConnections;
}
/*static*/ CrossConnections CrossConnections::CreateReverse()
{
  std::array<LeftTerminal, c_numChars> rightToLeft;
  for (unsigned char terminal = 0; terminal != c_numChars; ++terminal)
  {
    const auto crossTerminal = static_cast<unsigned char>(c_numChars-1-terminal);
    rightToLeft[terminal] = LeftTerminal{*Terminal::Create(crossTerminal)};
  }
  return CrossConnections{std::move(rightToLeft)};
}

LeftTerminal CrossConnections::TurnAround(LeftTerminal left) const
{
  return rightToLeft_[left.terminal.Value()];
}

Wheel::Wheel(const Connections& connections, Key ringSetting):
  connections_{connections},
  ringSetting_{ringSetting}
{
}

size_t Wheel::TotalRotation_() const
{
  return (ringSetting_.Value() - 'A') + rotation_;
}

LeftTerminal Wheel::ToLeft(RightTerminal right) const
{
  const auto wheelRotation = static_cast<int>(TotalRotation_());
  const auto effective_right = right.Inc(wheelRotation);
  const auto effective_left = connections_.ToLeft(effective_right);
  const auto left = effective_left.Inc(-wheelRotation);
  return left;
}

RightTerminal Wheel::ToRight(LeftTerminal left) const
{
  const auto wheelRotation = static_cast<int>(TotalRotation_());
  const auto effective_left = left.Inc(-wheelRotation);
  const auto effective_right = connections_.ToRight(effective_left);
  const auto right = effective_right.Inc(wheelRotation);
  return right;
}

size_t Wheel::Inc(size_t inc) //Returns the increment for the next wheel
{
  rotation_ = (rotation_ + inc) % c_numChars;
  return rotation_ ? 0 : 1;
}

TurnAboutWheel::TurnAboutWheel(CrossConnections crossConnections):
  crossConnections_{std::move(crossConnections)}
{
}

LeftTerminal TurnAboutWheel::TurnAround(LeftTerminal left) const
{
  return crossConnections_.TurnAround(left);
}

LeftTerminal Commutator::ToTerminal(Key key) const
{
  return LeftTerminal{*Terminal::Create((key.Value()-'A'))};
}

Key Commutator::ToLamp(LeftTerminal left) const
{
  return *Lamp::Create('A'+left.terminal.Value());
}

Scrambler::Scrambler(CrossConnections crossConnections,
                     std::array<Wheel,numScramblerWheels> wheels):
  turnAroundWheel_{crossConnections},
  wheels_{std::move(wheels)}
{
}

Lamp Scrambler::ToLamp(Key in)
{
  for_while(wheels_.rbegin(), wheels_.rend(), [](auto& wheel)
  {
    return wheel.Inc(1);
  });

  auto terminal = commutator_.ToTerminal(in);

  terminal = std::accumulate(wheels_.crbegin(), wheels_.crend(),
                             terminal, [](const auto leftTerminal, const auto& wheel)
  {
    return wheel.ToLeft(RightTerminal{leftTerminal.terminal});
  });

  terminal = turnAroundWheel_.TurnAround(terminal);

  terminal = std::accumulate(wheels_.cbegin(), wheels_.cend(),
                             terminal, [](const auto leftTerminal, const auto& wheel)
  {
    return LeftTerminal{wheel.ToRight(leftTerminal).terminal};
  });

  return commutator_.ToLamp(terminal);
}

WheelDescriptor::WheelDescriptor(IntRange<unsigned char, 0, numMachineWheels> wheelIndex,
                                 Key ringSetting):
  wheelIndex{wheelIndex},
  ringSetting{ringSetting}
{
}

SteckerBoard::SteckerBoard(const CrossPluggings& crossPluggings)
{
  for (char i = 0; i != c_numChars; ++i)
    crossPluggings_[i] = *Key::Create('A'+i);

  for (const auto& crossPlugging: crossPluggings)
  {
    crossPluggings_[crossPlugging.lhs.Value()-'A'] = crossPlugging.rhs;
    crossPluggings_[crossPlugging.rhs.Value()-'A'] = crossPlugging.lhs;
  }
}
/*static*/ bool SteckerBoard::IsValid_(const CrossPluggings& crossPluggings)
{
  std::array<unsigned char, c_numChars> charsUsed{};

  const auto anyDuplicate = std::any_of(crossPluggings.cbegin(), crossPluggings.cend(), [&charsUsed](const auto& crossPlugging)
  {
    return charsUsed[crossPlugging.lhs.Value()-'A']++
        || charsUsed[crossPlugging.rhs.Value()-'A'];
  });

  return !anyDuplicate;
}

/*static*/ std::optional<SteckerBoard> SteckerBoard::Create(const CrossPluggings& crossPluggings)
{
  return IsValid_(crossPluggings)
       ? SteckerBoard(crossPluggings)
       : std::optional<SteckerBoard>{};
}
Key SteckerBoard::Cross(Key key) const
{
  return crossPluggings_[key.Value()-'A'];
}

Machine::Machine( CrossConnections crossConnections,
                  std::array<Connections,numMachineWheels> connectionss,
                  std::array<WheelDescriptor,numScramblerWheels> wheels,
                  SteckerBoard steckerBoard)
: connectionss_{std::move(connectionss)},
  scrambler_{std::move(crossConnections),
             {Wheel{connectionss_[wheels[0].wheelIndex.Value()], wheels[0].ringSetting},
              Wheel{connectionss_[wheels[1].wheelIndex.Value()], wheels[1].ringSetting},
              Wheel{connectionss_[wheels[2].wheelIndex.Value()], wheels[2].ringSetting}}},
  steckerBoard_{std::move(steckerBoard)}
{
}

Lamp Machine::ToLamp(const Key key_)
{
  const auto key = steckerBoard_.Cross(key_);
  const auto lamp = scrambler_.ToLamp(key);

  return steckerBoard_.Cross(lamp);
}

std::string Machine::ToLamp(const std::string_view keys)
{
  std::string lamps;

  for (const auto key_: keys)
  {
    const auto key = Key::Create(key_);
    if (!key)
      return {};
    lamps += ToLamp(*key).Value();
  }

  return lamps;
}
