#pragma once
#include <string>

namespace Constants
{
   const int DownScaleFactor = 3;

   const int AddPolygonMutationRate = 120;
   const int RemovePolygonMutationRate = 215;
   const int MovePolygonMutationRate = 185;

   const int MovePointMaxMutationRate = 250;
   const int MovePointMidMutationRate = 150;
   const int MovePointMinMutationRate = 25;

   const int ColorMutateRateMin = 50;
   const int ColorMutateRate = 200;

   const int MinAlpha = 20;
   const int MaxAlpha = 255;

   const int MaximumTriangleCount = 280;
   const int MaximumVertexCount = MaximumTriangleCount * 3;
   const int InitialTriangleCount = 20;
}
