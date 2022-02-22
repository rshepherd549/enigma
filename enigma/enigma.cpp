#include "pch.h"
#include <algorithm>
#include <numeric>
#include "enigma.h"
#include "framework.h"

bool operator==(const LeftTerminal& left1, const LeftTerminal& left2)
{
  return left1.terminal == left2.terminal;
}
LeftTerminal LeftTerminal::operator+(int inc) const
{
  return LeftTerminal{terminal + inc};
};
LeftTerminal LeftTerminal::operator-(int inc) const
{
  return operator+(-inc);
};
RightTerminal RightTerminal::operator+(int inc) const
{
  return RightTerminal{terminal + inc};
};
RightTerminal RightTerminal::operator-(int inc) const
{
  return operator+(-inc);
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

/*static*/ Connections Connections::Create()
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

CrossConnection::CrossConnection():
  from_{LeftTerminal{}}, to_{from_ + 1}
{}

CrossConnection::CrossConnection(LeftTerminal from, LeftTerminal to):
  from_(from), to_(to)
{}

std::optional<CrossConnection> CrossConnection::Create(LeftTerminal from, LeftTerminal to)
{
  return (to == from) ? std::optional<CrossConnection>{} : CrossConnection{from,to};
}
LeftTerminal CrossConnection::From() const
{
  return from_;
}
LeftTerminal CrossConnection::To() const
{
  return to_;
}

CrossConnections::CrossConnections(Connections connections):
  Connections{std::move(connections)}
{
}

static std::optional<Connections> ToConnections(std::array<CrossConnection, c_numChars/2>& crossConnectons)
{
  std::array<LeftTerminal, c_numChars> connections;
  for (const auto& crossConnection: crossConnectons)
  {
    connections[crossConnection.From().terminal.Index()] = crossConnection.To();
    connections[crossConnection.To().terminal.Index()] = crossConnection.From();
  }
  return Connections::Create(connections);
}

//Ensure connections are paired i.e. if i -> j then j -> i
/*static*/ std::optional<CrossConnections> CrossConnections::Create(std::array<CrossConnection, c_numChars/2> crossConnections)
{
  if (auto connections = ToConnections(crossConnections))
    return CrossConnections{*connections};
  return {};
}
/*static*/ CrossConnections CrossConnections::CreateReverse()
{
  std::array<CrossConnection, c_numChars/2> crossConnections;
  const auto numCharsBy2 = static_cast<char>(c_numChars/2);
  for (unsigned char terminal = 0; terminal != numCharsBy2; ++terminal)
  {
    if (const auto from = Terminal::Create(terminal))
      if (const auto to = Terminal::Create(c_numChars - terminal - 1))
        crossConnections[terminal] = *CrossConnection::Create(LeftTerminal{*from},
                                                              LeftTerminal{*to});
  }

  return *CrossConnections::Create(std::move(crossConnections));
}

LeftTerminal CrossConnections::Transform(LeftTerminal left) const
{
  return rightToLeft_[left.terminal.Value()];
}

Wheel::Wheel(const Connections& connections):
  connections_{connections}
{
}

LeftTerminal Wheel::ToLeft(RightTerminal right) const
{
  const auto left = connections_.ToLeft(right);
  return left;
}

RightTerminal Wheel::ToRight(LeftTerminal left) const
{
  const auto right = connections_.ToRight(left);
  return right;
}

Rotor::Rotor(const Wheel& wheel, Key ringSetting):
  wheel_{wheel},
  rotation_{ringSetting.Index()}
{
}

size_t Rotor::TotalRotation_() const
{
  return rotation_;
}

LeftTerminal Rotor::ToLeft(RightTerminal right) const
{
  const auto effective_right = right - static_cast<int>(rotation_);
  const auto effective_left = wheel_.get().ToLeft(effective_right);
  const auto left = effective_left + static_cast<int>(rotation_);
  return left;
}

RightTerminal Rotor::ToRight(LeftTerminal left) const
{
  const auto effective_left = left - static_cast<int>(rotation_);
  const auto effective_right = wheel_.get().ToRight(effective_left);
  const auto right = effective_right + static_cast<int>(rotation_);
  return right;
}

size_t Rotor::Inc(size_t inc) //Returns the increment for the next wheel
{
  rotation_ = (rotation_ + inc) % c_numChars;
  return rotation_ ? 0 : 1;
}

TurnAboutWheel::TurnAboutWheel(CrossConnections crossConnections):
  crossConnections_{std::move(crossConnections)}
{
}

LeftTerminal TurnAboutWheel::Transform(LeftTerminal left) const
{
  return crossConnections_.Transform(left);
}

LeftTerminal Commutator::ToTerminal(Key key) const
{
  return LeftTerminal{*Terminal::Create((key.Value()-'A'))};
}

Key Commutator::ToLamp(LeftTerminal left) const
{
  return *Lamp::Create('A'+left.terminal.Value());
}

Scrambler::Scrambler(TurnAboutWheel turnAroundWheel,
                     const std::array<Wheel,numMachineWheels>& wheels):
  turnAroundWheel_{turnAroundWheel},
  rotors_{{{wheels[0], Key{}},
           {wheels[1], Key{}},
           {wheels[2], Key{}}}}
{
}

void Scrambler::Configure(const std::array<Wheel,numMachineWheels>& wheels,
                          std::array<WheelSelection,numScramblerRotors> selections)
{
  rotors_ = {Rotor{wheels[selections[0].wheelIndex.Value()], selections[0].ringSetting},
             Rotor{wheels[selections[1].wheelIndex.Value()], selections[1].ringSetting},
             Rotor{wheels[selections[2].wheelIndex.Value()], selections[2].ringSetting}};
}

Lamp Scrambler::ToLamp(Key in)
{
  for_while(rotors_.rbegin(), rotors_.rend(), [](auto& wheel)
  {
    return wheel.Inc(1);
  });

  auto terminal = commutator_.ToTerminal(in);

  terminal = std::accumulate(rotors_.crbegin(), rotors_.crend(),
                             terminal, [](const auto leftTerminal, const auto& wheel)
  {
    return wheel.ToLeft(RightTerminal{leftTerminal.terminal});
  });

  terminal = turnAroundWheel_.Transform(terminal);

  terminal = std::accumulate(rotors_.cbegin(), rotors_.cend(),
                             terminal, [](const auto leftTerminal, const auto& wheel)
  {
    return LeftTerminal{wheel.ToRight(leftTerminal).terminal};
  });

  return commutator_.ToLamp(terminal);
}

WheelSelection::WheelSelection(IntRange<unsigned char, 0, numMachineWheels> wheelIndex,
                                 Key ringSetting):
  wheelIndex{wheelIndex},
  ringSetting{ringSetting}
{
}

std::optional<Connections> ToConnections(const Plugs& plugs)
{
  std::array<LeftTerminal, c_numChars> connections;

  //Default plugs to identity connections
  for (unsigned char terminal = 0; terminal != c_numChars; ++terminal)
    connections[terminal] = LeftTerminal{*Terminal::Create(terminal)};

  for (const auto& plug: plugs)
  {
    const auto lhs = static_cast<unsigned char>(plug.lhs.Index());
    const auto rhs = static_cast<unsigned char>(plug.rhs.Index());
    connections[lhs] = LeftTerminal{*Terminal::Create(rhs)};
    connections[rhs] = LeftTerminal{*Terminal::Create(lhs)};
  }
  return Connections::Create(std::move(connections));
}

PlugBoard::PlugBoard(Connections connections):
  connections_{std::move(connections)}
{}

/*static*/ std::optional<PlugBoard> PlugBoard::Create(const Plugs& plugs)
{
  if (auto connections = ToConnections(plugs))
    return PlugBoard(std::move(*connections));

  return {};
}
Key PlugBoard::Transform(Key key) const
{
  const auto terminalIn = RightTerminal{*Terminal::Create(key.Index())};
  const auto terminalOut = connections_.ToLeft(terminalIn);

  if (const auto keyOut = Key::Create(terminalOut.terminal.Value()))
    return *keyOut;

  throw;
}

Machine::Machine(TurnAboutWheel turnAboutWheel,
                 std::array<Wheel, numMachineWheels> wheels)
: wheels_{std::move(wheels)},
  scrambler_{turnAboutWheel, wheels_},
  plugBoard_{*PlugBoard::Create({})}
{
}

void Machine::Configure(std::array<WheelSelection, numScramblerRotors> selections,
                        PlugBoard plugBoard)
{
  scrambler_.Configure(wheels_, selections);
  plugBoard_ = std::move(plugBoard);
}

Lamp Machine::ToLamp(const Key key_)
{
  const auto pluggedKey  = plugBoard_.Transform(key_);
  const auto pluggedLamp = scrambler_.ToLamp(pluggedKey);
  const auto lamp        = plugBoard_.Transform(pluggedLamp);
  return lamp;
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
