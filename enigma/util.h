#pragma once

#include <algorithm>
#include <memory>

template <typename T, T begin_, size_t num_>
class IntRange
{
  T value_{begin_};
  IntRange(T value): value_{value}
  {}
public:
  using value_type = T;
  IntRange() = default;

  template <typename TIn>
  static std::optional<IntRange> Create(TIn value)
  {
    return (value < begin_ || value >= (begin_ + num_))
         ? std::optional<IntRange>{}
         : IntRange{static_cast<T>(value)};
  }

  static constexpr T begin()
  {
    return begin_;
  }
  static constexpr T end()
  {
    return static_cast<T>(begin_ + num_);
  }
  static constexpr size_t num()
  {
    return num_;
  }

  T Value() const
  {
    return value_;
  }
  size_t Index() const
  {
    return static_cast<size_t>(value_ - begin_);
  }
  bool operator==(const IntRange& other) const
  {
    return Value() == other.Value();
  }
  [[nodiscard]] IntRange operator+(int inc) const
  {
    return static_cast<T>((value_ + inc + num_) % num_);
  };

};

template <typename TItr, typename TPred>
bool any_itr(TItr itr, TItr end, TPred pred) //expect TPred::operator(Titr)
{
  for (; itr != end; ++itr)
    if (pred(itr))
      return true;

  return false;
}

template <typename TItr, typename TPred>
bool none_itr(TItr itr, TItr end, TPred pred) //expect TPred::operator(Titr)
{
  for (; itr != end; ++itr)
    if (pred(itr))
      return false;

  return true;
}

template <typename TItr>
bool all_unique(TItr begin, TItr end)
{
  return none_itr(begin, end, [begin](const auto curr)
         {
           return std::find(begin, curr, *curr) != curr;
         });
}

template <typename TItr, typename TPred>
TItr for_while(TItr itr, TItr end, TPred pred) //expect TPred::operator(*itr)
{
  while (itr != end && pred(*itr))
    ++itr;

  return itr;
}

template <typename T, size_t num, typename TFnc>
auto to_array(const std::array<T,num>& arrIn, const TFnc fnc) -> std::array<decltype(fnc(arrIn.front())), num>
{
  std::array<decltype(fnc(arrIn.front())), num> arrOut;
  std::transform(arrIn.cbegin(), arrIn.cend(), arrOut.begin(), fnc);
  return arrOut;
}