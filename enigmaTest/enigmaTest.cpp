#include "pch.h"
#include "CppUnitTest.h"
#include "enigma.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace enigmaTest
{
  TEST_CLASS(enigmaTest)
  {
  public:
    TEST_METHOD(TestTextCharCreate)
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
        Assert::AreEqual(!test.expectedC, !tc);

        if (auto expectedC = test.expectedC; expectedC && tc)
          Assert::AreEqual(*expectedC, tc->Value());
      }
    }

    TEST_METHOD(TestIdentityWheels)
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
      Assert::AreEqual('Z', m.ToLamp(*Key::Create('A')).Value());
      Assert::AreEqual('Z', m.ToLamp(*Key::Create('A')).Value());
      Assert::AreEqual('Y', m.ToLamp(*Key::Create('B')).Value());
    }
  };
}
