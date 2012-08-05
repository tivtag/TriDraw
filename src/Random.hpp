#pragma once

#include <cmath>
#include <ctime>
#include <cstdlib>

#include "OpenGL.hpp"

inline void setup_rand()
{
   srand(static_cast<unsigned int>(time(NULL)));
}

inline GLfloat random_float()
{
   return (GLfloat)rand() / (GLfloat)RAND_MAX;
}

inline GLfloat random_float(const float minimum, const float maximum)
{
   return minimum + random_float() * (maximum - minimum);
}

template<typename T>
inline T random_integer(const T minimum, const T maximum)
{
   const int value = rand();
   const T range = maximum - minimum;

   return minimum + (value % range);
}

inline unsigned char random_byte()
{
   return static_cast<unsigned char>(rand() % 255);
}

inline bool should_mutate(const int chance)
{
   return random_integer(0, chance) == 1;
}
