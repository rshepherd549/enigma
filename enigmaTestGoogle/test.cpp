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
      EXPECT_EQ(*expectedC, tc->ToChar()) << "(" << test.line << ") " << test.msg;
  }
}

TEST(TestMachine, Press)
{
  Machine m;
  EXPECT_EQ('A', m.Press(*Key::Create('A')).ToChar());
  EXPECT_EQ('B', m.Press(*Key::Create('B')).ToChar());
}

TEST(TestCaseName, TestName) {
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(false) << "spagetti";
}