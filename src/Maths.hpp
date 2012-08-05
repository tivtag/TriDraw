#pragma once

#include <algorithm>

template<typename T>
T clamp(const T value, const T minimum, const T maximum)
{
   return std::min(maximum, std::max(minimum, value));
}
