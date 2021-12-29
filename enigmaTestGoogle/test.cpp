#include "pch.h"
#include "enigma.h"

TEST(TestTextChar, Create)
{
  struct
  {
    size_t line;
    std::string msg;
    char c;
    std::optional<char> expectedC;
  } tests[] =
  {
    {__LINE__, "allow capital letters", 'A', 'A'},
    {__LINE__, "allow capital letters", 'P', 'P'},
    {__LINE__, "don't allow numbers", '1', {}},
    {__LINE__, "don't allow lower case", 'a', {}},
    {__LINE__, "don't allow null", 0, {}},
    {__LINE__, "don't allow space", ' ', {}},
    {__LINE__, "don't allow punctuation", '.', {}},
  };
  for (const auto& test: tests)
  {
    auto tc = TextChar::Create(test.c);
    EXPECT_EQ(!test.expectedC, !tc) << "(" << test.line << ") " << test.msg;

    if (auto expectedC = test.expectedC; expectedC && tc)
      EXPECT_EQ(*expectedC, tc->Value()) << "(" << test.line << ") " << test.msg;
  }
}

TEST(TestMachine, IdentityWheels)
{
  Machine m
  {
    { Connections::CreateIdentity(),
      Connections::CreateIdentity(),
      Connections::CreateIdentity(),
      Connections::CreateIdentity(),
      Connections::CreateIdentity()},
    CrossConnections::CreateReverse(),
    { WheelDescriptor{*WheelIndex::Create(0), *Key::Create('A')},
      WheelDescriptor{*WheelIndex::Create(1), *Key::Create('A')},
      WheelDescriptor{*WheelIndex::Create(2), *Key::Create('A')}}
  };
  auto key_a = *Key::Create('A');
  EXPECT_EQ('Z', m.ToLamp(key_a).Value());
  EXPECT_EQ('Z', m.ToLamp(key_a).Value());
}

