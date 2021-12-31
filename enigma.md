# Enigma

## Intercepted messages

Operator would listen to the message in morse code.
Unclear alternatives would be presented as several options.
May miss part of the text (not modelled here)

Different test harnesses:
- MS Native Unit Tests - integrated runner but can't work ouot how to add arbitrary messages
- GoogleTests - more basic integration but nicer macros and streaming

## Increment the wheels

  size_t inc = 1;
  std::for_each(wheels_.rbegin(), wheels_.rend(),
                [&inc](auto& wheel)
  {
    inc = wheel.Inc(inc);
  });

  std::find(wheels_.rbegin(), wheels_.rend(),
            [](auto& wheel)
  {
    return !wheel.Inc(1);
  });

  for (auto wheel = wheels_.rbegin();
       wheel != wheels_.rend() && !wheel->Inc(1);
       ++wheel)
  {}

  for_while(wheels_.rbegin(), wheels_.rend(), [](const auto& wheel)
  {
    return wheel->Inc(1);
  });

## Difficulties of creating array

template <typename T, size_t num, typename TFnc>
auto to_array(const std::array<T,num>& arrIn, const TFnc fnc) -> std::array<decltype(fnc(arrIn.front())), num>
{
  std::array<decltype(fnc(arrIn.front())), num> arrOut;
  std::transform(arrIn.cbegin(), arrIn.cend(), arrOut.begin(), fnc);
  return arrOut;
}

Explicit types are a pain for test creation

https://en.wikipedia.org/wiki/Enigma_machine


