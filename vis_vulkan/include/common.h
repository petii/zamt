#pragma once

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

template <typename Container, typename Generator, typename... Args>
auto create_n(std::size_t count, Generator generator, Args&&... args)
{
  Container result;
  std::generate_n(std::back_inserter(result), count, [&generator, &args...] {
    return generator(std::forward<Args>(args)...);
  });
  return result;
}

// From and To are containers
template <typename From, typename To> To typeChangeMove(From&& from)
{
  return To{std::move_iterator(std::begin(from)), std::move_iterator(std::end(from))};
}

// For vulkan.hpp
// TODO: check back after a ::value_type is added to UniqueHandle
template <typename Type, typename UniqueType>
auto getHandles(std::vector<UniqueType>& vec)
{
  // static_assert(std::is_same_v<UniqueType, vk::UniqueHandler<Type>);
  std::vector<Type> res;
  res.reserve(vec.size());
  for (auto i = 0; i < vec.size(); ++i) {
    res.emplace_back(vec[i].get());
  }
  return res;
}

template <typename T, typename K>
auto combine_vectors(std::vector<T> one, std::vector<K> two)
{
  auto size = std::min(one.size(), two.size());
  std::vector<std::pair<T, K>> combined;
  combined.reserve(size);
  for (auto i = 0; i < size; ++i) {
    combined.emplace_back(std::make_pair(one[i], two[i]));
  }
  return combined;
}

template <typename InputIt, typename UnaryGenerator>
auto create_apply(InputIt from, InputIt to, UnaryGenerator g)
{
  std::vector<decltype(g(*from))> result;
  result.reserve(std::distance(from, to));
  for (auto it = from; it != to; ++it) {
    result.emplace_back(g(*it));
  }
  return result;
}

template <typename Cont> auto vector_view(const Cont& container)
{
  std::vector<typename Cont::value_type> result(std::size(container));
  std::copy(std::begin(container), std::end(container), result.begin());
  return std::move(result);
}

template <typename T> constexpr auto clamp(T lowerBound, T upperBound, T value)
{
  // static_assert(lowerBound < upperBound);
  return value < lowerBound ? lowerBound : value > upperBound ? upperBound : value;
}

template <typename T> 
auto getElementsAtIndex(std::size_t index)
{
  return std::vector<T>{};
}

template <typename T, typename FirstContainer, typename... Containers>
auto getElementsAtIndex(std::size_t index, FirstContainer&& first, Containers&&... containers)
{
  std::vector<T> indexthElements{first[index]};
  auto rest = getElementsAtIndex<T>(index, std::forward<Containers>(containers)...);
  for (auto&& element : rest) {
    indexthElements.emplace_back(element);
  }
  return indexthElements;
}