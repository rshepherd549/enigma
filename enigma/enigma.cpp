#include "pch.h"
#include <algorithm>
#include <numeric>
#include "enigma.h"
#include "framework.h"

bool operator==(const LeftTerminal& left1, const LeftTerminal& left2)
{
  return left1.terminal == left2.terminal;
}

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

RightTerminal Wheel::RotateRight_(RightTerminal right) const
{
  return RightTerminal{*Terminal::Create( ( right.terminal.Value()
                                          + TotalRotation_())
                                        % c_numChars)};
}
RightTerminal Wheel::UnRotateRight_(RightTerminal right) const
{
  return RightTerminal{*Terminal::Create( ( right.terminal.Value()
                                          - TotalRotation_())
                                        % c_numChars)};
}

LeftTerminal Wheel::ToLeft(RightTerminal right) const
{
  return connections_.ToLeft(RotateRight_(right));
}

RightTerminal Wheel::ToRight(LeftTerminal left) const
{
  return UnRotateRight_(connections_.ToRight(left));
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
                     std::array<std::reference_wrapper<const Connections>,3> connectionss):
  turnAroundWheel_{crossConnections},
  wheels_({Wheel{connectionss[0].get(), *Key::Create('A')},
           Wheel{connectionss[1].get(), *Key::Create('A')},
           Wheel{connectionss[2].get(), *Key::Create('A')}})
{
}

Lamp Scrambler::ToLamp(Key in) const
{
  auto terminal = commutator_.ToTerminal(in);

  terminal = std::accumulate(wheels_.crbegin(), wheels_.crend(),
                             terminal,
                             [](const auto leftTerminal, const auto& wheel)
  {
    return wheel.ToLeft(RightTerminal{leftTerminal.terminal});
  });

  terminal = turnAroundWheel_.TurnAround(terminal);

  terminal = std::accumulate(wheels_.cbegin(), wheels_.cend(),
                             terminal,
                             [](const auto leftTerminal, const auto& wheel)
  {
    return LeftTerminal{wheel.ToRight(leftTerminal).terminal};
  });

  return commutator_.ToLamp(terminal);
}

Machine::Machine():
  connectionss_{{Connections::CreateIdentity(),
                 Connections::CreateIdentity(),
                 Connections::CreateIdentity(),
                 Connections::CreateIdentity(),
                 Connections::CreateIdentity()}},
  scrambler_{CrossConnections::CreateReverse(),
             {connectionss_[0],connectionss_[1],connectionss_[2]}}
{
}

Lamp Machine::ToLamp(Key key) const
{
  return scrambler_.ToLamp(key);
}

