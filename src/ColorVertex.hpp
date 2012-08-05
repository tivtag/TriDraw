#pragma once

#include "OpenGL.hpp"
#include "Random.hpp"
#include "Constants.hpp"
#include "Maths.hpp"

struct ColorVertex
{
#pragma pack(push,1)
   GLfloat x;
   GLfloat y;

   unsigned char r;	
   unsigned char g;
   unsigned char b;
   unsigned char a;
#pragma pack(pop)

   ColorVertex() : x(0.0f), y(0.0f), r(0), g(0), b(0), a(0)
   {
   }

   ColorVertex(const GLfloat _x, const GLfloat _y, const unsigned char _r, const unsigned char _g, const unsigned char _b, const unsigned char _a)
      :x(_x), y(_y), r(_r), g(_g), b(_b), a(_a)
   {
   }

   inline void mutate()
   {
      if(should_mutate(Constants::MovePointMaxMutationRate))
      {
         x = random_float(-1.0f, 1.0f);
         y = random_float(-1.0f, 1.0f);
      }

      if(should_mutate(Constants::MovePointMidMutationRate))
      {
         const float MediumMovement = 0.01f;
         x += random_float(-MediumMovement, MediumMovement);
         y += random_float(-MediumMovement, MediumMovement);
      }

      if(should_mutate(Constants::MovePointMinMutationRate))
      {
         const float MinimumMovement = 0.001f;
         x += random_float(-MinimumMovement, MinimumMovement);
         y += random_float(-MinimumMovement, MinimumMovement);
      }

      if(should_mutate(Constants::ColorMutateRateMin))
      {
         const int Range = 3;
         r += random_integer(-Range, Range);
      }
         
      if(should_mutate(Constants::ColorMutateRateMin))
      {
         const int Range = 3;
         g += random_integer(-Range, Range);
      }
                
      if(should_mutate(Constants::ColorMutateRateMin))
      {
         const int Range = 3;
         b += random_integer(-Range, Range);
      }
                    
      if(should_mutate(Constants::ColorMutateRateMin))
      {
         const int Range = 2;
         a += random_integer(-Range, Range);
      }
      
      if(should_mutate(Constants::ColorMutateRate))
      {
         r = random_byte();
      }
      if(should_mutate(Constants::ColorMutateRate))
      {
         g = random_byte();
      }
      if(should_mutate(Constants::ColorMutateRate))
      {
         b = random_byte();
      }
      if(should_mutate(Constants::ColorMutateRate))
      {
         a = random_byte(); // ToDo: Min 25
      }
   }
};
