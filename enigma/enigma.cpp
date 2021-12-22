#include "pch.h"
#include "enigma.h"
#include "framework.h"

TextChar::TextChar(char c):
  c_(c)
{
}
std::optional<TextChar> TextChar::Create(char c) //Allow A-Z
{
  return (c < 'A' || c > 'Z') ? std::optional<TextChar>{} : TextChar(c);
}
char TextChar::ToChar() const
{
  return c_;
}

Lamp Machine::Press(Key key) const
{
  if (auto lamp = Lamp::Create(key.ToChar()))
    return *lamp;
  throw std::exception("Press failure");
}

