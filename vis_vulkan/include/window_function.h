#pragma once

#include <algorithm>
#include <cmath>
#include <iterator>
#include <numeric>
#include <type_traits>
#include <vector>

#include <iostream>

constexpr auto PI = 3.141592653589793;

template <typename Array, typename WindowFunction>
auto apply_window(Array&& input, WindowFunction function)
{
  using ArrayT = std::remove_reference_t<Array>;
  static_assert(std::is_arithmetic_v<typename ArrayT::value_type>);

  auto size = std::size(std::forward<Array>(input));
  ArrayT output(size);
  for (auto i = 0; i < size; ++i) {
    output[i] = function(i) * input[i];
  }

  return output;
}

namespace window {

namespace detail {

template <typename T> T& table()
{
  // basically a singleton
  static T t;
  return t;
}

template <std::size_t size, typename T> struct Hann {
  std::vector<T> table;
};

template <std::size_t size, typename T> auto getHannTable()
{
  std::vector<T> table(size);
  for (auto i = 0; i < size; ++i) {
    T y = 0.5 * (1.0 - std::cos(2 * PI * i / (size - 1)));
    table[i] = y;
  }
  return table;
}

} // namespace detail

template <std::size_t size, typename T> void setupHannTable()
{
  detail::table<detail::Hann<size, T>>().table = detail::getHannTable<size, T>();
}

template <typename T = float> constexpr auto identity(std::size_t index) { return 1; }

template <std::size_t sampleSize, typename T = float>
constexpr auto hann(std::size_t index)
{
  return detail::table<detail::Hann<sampleSize, T>>().table[index];
}

} // namespace window