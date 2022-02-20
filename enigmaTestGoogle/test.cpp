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
    CrossConnections::CreateReverse(),
    { Connections::CreateIdentity(),
      Connections::CreateIdentity(),
      Connections::CreateIdentity(),
      Connections::CreateIdentity(),
      Connections::CreateIdentity()},
    { WheelSelection{*WheelIndex::Create(0), *Key::Create('A')},
      WheelSelection{*WheelIndex::Create(1), *Key::Create('A')},
      WheelSelection{*WheelIndex::Create(2), *Key::Create('A')}},
    *SteckerBoard::Create({})
  };
  auto m_ = m;

  EXPECT_EQ('Z', m.ToLamp(*Key::Create('A')).Value());
  EXPECT_EQ('Z', m.ToLamp(*Key::Create('A')).Value());
  EXPECT_EQ('Y', m.ToLamp(*Key::Create('B')).Value());
  EXPECT_EQ("SVOOLDLIOW", m.ToLamp("HELLOWORLD"));
  EXPECT_EQ("SVOOLDLIOW", m.ToLamp("HELLOWORLD"));
  EXPECT_EQ("HELLOWORLD", m.ToLamp("SVOOLDLIOW"));

  EXPECT_EQ('A', m_.ToLamp(*Key::Create('Z')).Value());
  EXPECT_EQ('A', m_.ToLamp(*Key::Create('Z')).Value());
  EXPECT_EQ('B', m_.ToLamp(*Key::Create('Y')).Value());
  EXPECT_EQ("HELLOWORLD", m_.ToLamp("SVOOLDLIOW"));
}

TEST(TestMachine, AddOneWheels)
{
  Machine m
  {
    CrossConnections::CreateReverse(),
    { *Connections::Create(std::array<unsigned char, c_numChars>{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25}),
      Connections::CreateIdentity(),
      Connections::CreateIdentity(),
      Connections::CreateIdentity(),
      Connections::CreateIdentity()},
    { WheelSelection{*WheelIndex::Create(0), *Key::Create('A')},
      WheelSelection{*WheelIndex::Create(1), *Key::Create('A')},
      WheelSelection{*WheelIndex::Create(2), *Key::Create('A')}},
    *SteckerBoard::Create({})
  };
  auto m_ = m;

  EXPECT_EQ('X', m.ToLamp(*Key::Create('A')).Value());
  EXPECT_EQ('X', m.ToLamp(*Key::Create('A')).Value());
  EXPECT_EQ('W', m.ToLamp(*Key::Create('B')).Value());
  EXPECT_EQ("QTMMJBJGMU", m.ToLamp("HELLOWORLD"));
  EXPECT_EQ("QTMMJBJGMU", m.ToLamp("HELLOWORLD"));
  EXPECT_EQ("HELLOWORLD", m.ToLamp("QTMMJBJGMU"));

  EXPECT_EQ('A', m_.ToLamp(*Key::Create('X')).Value());
  EXPECT_EQ('A', m_.ToLamp(*Key::Create('X')).Value());
  EXPECT_EQ('B', m_.ToLamp(*Key::Create('W')).Value());
  EXPECT_EQ("HELLOWORLD", m_.ToLamp("QTMMJBJGMU"));
}

TEST(TestMachine, ChangingWheels)
{
  Machine m0
  {
    CrossConnections::CreateReverse(),                         // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25
    { Connections::CreateIdentity(),
      Connections::CreateIdentity(),
      *Connections::Create(std::array<unsigned char, c_numChars>{ 0, 2, 1, 5, 3, 4, 9, 6, 7, 8,15,10,11,12,13,14,22,16,17,18,19,20,21,25,23,24}),
      Connections::CreateIdentity(),
      Connections::CreateIdentity()},
    { WheelSelection{*WheelIndex::Create(0), *Key::Create('A')},
      WheelSelection{*WheelIndex::Create(1), *Key::Create('A')},
      WheelSelection{*WheelIndex::Create(2), *Key::Create('A')}},
    *SteckerBoard::Create({})
  };
  auto m1 = m0;
  auto m2 = m0;
  auto m3 = m0;

  //p       +p      -p                      +p      -p
  //1: A-> 0-> 1-> 2-> 1-> 1-> 1->24->24->24->25->23->22->W
  EXPECT_EQ('W', m0.ToLamp(*Key::Create('A')).Value());
  //1: W->22->23->25->24->24->24-> 1-> 1-> 1-> 2-> 1-> 0->A
  EXPECT_EQ('A', m1.ToLamp(*Key::Create('W')).Value());

  //2: A-> 0-> 2-> 1->25->25->25-> 0-> 0-> 0-> 2-> 1->25->Z
  EXPECT_EQ('Z', m0.ToLamp(*Key::Create('A')).Value());

  //3: A-> 0-> 3-> 5-> 2-> 2-> 2->23->23->23-> 0-> 0->23->X
  EXPECT_EQ('X', m0.ToLamp(*Key::Create('A')).Value());

  //4: B-> 1-> 5-> 4-> 0-> 0-> 0->25->25->25-> 3-> 4-> 0->A
  EXPECT_EQ('A', m0.ToLamp(*Key::Create('B')).Value());

  EXPECT_EQ('Z', m1.ToLamp(*Key::Create('A')).Value());
  EXPECT_EQ('A', m1.ToLamp(*Key::Create('X')).Value());
  EXPECT_EQ('B', m1.ToLamp(*Key::Create('A')).Value());

  EXPECT_EQ("UTQQNFNKQY", m2.ToLamp("HELLOWORLD"));
  EXPECT_EQ("UQOQKFNKQV", m2.ToLamp("HELLOWORLD"));
  EXPECT_EQ("HLJLLWORSH", m2.ToLamp("UQOQKFNKJU"));

  EXPECT_EQ("HELLOWORLD", m3.ToLamp("UTQQNFNKQY"));
  EXPECT_EQ("HELLOWORLD", m3.ToLamp("UQOQKFNKQV"));
  EXPECT_EQ("UQOQKFNKJU", m3.ToLamp("HLJLLWORSH"));

}

TEST(TestMachine, TestSteckerBoard)
{
  EXPECT_TRUE( SteckerBoard::Create({{*Key::Create('A'),*Key::Create('B')},
                                     {*Key::Create('E'),*Key::Create('L')},
                                     {*Key::Create('W'),*Key::Create('D')}}));

  EXPECT_FALSE(SteckerBoard::Create({{*Key::Create('A'),*Key::Create('B')},
                                     {*Key::Create('A'),*Key::Create('L')},
                                     {*Key::Create('W'),*Key::Create('D')}})) << "Should detect duplicate Keys";

  Machine m
  {
    CrossConnections::CreateReverse(),
    { Connections::CreateIdentity(),
      Connections::CreateIdentity(),
      Connections::CreateIdentity(),
      Connections::CreateIdentity(),
      Connections::CreateIdentity()},
    { WheelSelection{*WheelIndex::Create(0), *Key::Create('A')},
      WheelSelection{*WheelIndex::Create(1), *Key::Create('A')},
      WheelSelection{*WheelIndex::Create(2), *Key::Create('A')}},
    *SteckerBoard::Create({{*Key::Create('A'),*Key::Create('B')},
                           {*Key::Create('E'),*Key::Create('L')},
                           {*Key::Create('W'),*Key::Create('D')}})
  };
  auto m_ = m;

  EXPECT_EQ("SOVVEDEIVW", m.ToLamp("HELLOWORLD"));

  EXPECT_EQ("HELLOWORLD", m_.ToLamp("SOVVEDEIVW"));
}

