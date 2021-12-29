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
  auto m_ = m;

  EXPECT_EQ('Z', m.ToLamp(*Key::Create('A')).Value());
  EXPECT_EQ('Z', m.ToLamp(*Key::Create('A')).Value());
  EXPECT_EQ('Y', m.ToLamp(*Key::Create('B')).Value());
  EXPECT_EQ("SVOOLDLIOW", m.ToLamp("HELLOWORLD"));

  EXPECT_EQ('A', m_.ToLamp(*Key::Create('Z')).Value());
  EXPECT_EQ('A', m_.ToLamp(*Key::Create('Z')).Value());
  EXPECT_EQ('B', m_.ToLamp(*Key::Create('Y')).Value());
  EXPECT_EQ("HELLOWORLD", m_.ToLamp("SVOOLDLIOW"));
}

TEST(TestMachine, AddOneWheels)
{
  Machine m
  {
    { *Connections::Create({1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25}),
      Connections::CreateIdentity(),
      Connections::CreateIdentity(),
      Connections::CreateIdentity(),
      Connections::CreateIdentity()},
    CrossConnections::CreateReverse(),
    { WheelDescriptor{*WheelIndex::Create(0), *Key::Create('A')},
      WheelDescriptor{*WheelIndex::Create(1), *Key::Create('A')},
      WheelDescriptor{*WheelIndex::Create(2), *Key::Create('A')}}
  };
  auto m_ = m;

  EXPECT_EQ('X', m.ToLamp(*Key::Create('A')).Value());
  EXPECT_EQ('X', m.ToLamp(*Key::Create('A')).Value());
  EXPECT_EQ('W', m.ToLamp(*Key::Create('B')).Value());
  EXPECT_EQ("QTMMJBJGMU", m.ToLamp("HELLOWORLD"));

  EXPECT_EQ('A', m_.ToLamp(*Key::Create('X')).Value());
  EXPECT_EQ('A', m_.ToLamp(*Key::Create('X')).Value());
  EXPECT_EQ('B', m_.ToLamp(*Key::Create('W')).Value());
  EXPECT_EQ("HELLOWORLD", m_.ToLamp("QTMMJBJGMU"));
}

