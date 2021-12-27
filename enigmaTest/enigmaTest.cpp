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

    TEST_METHOD(TestTransformChar)
    {
      Machine m;
      Assert::AreEqual('A', m.ToLamp(*Key::Create('A')).Value());
      Assert::AreEqual('B', m.ToLamp(*Key::Create('B')).Value());
    }
  };
}
