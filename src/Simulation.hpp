#pragma once

#include "OpenGL.hpp"
#include "ColorVertex.hpp"
#include "Texture.hpp"
#include "AssetLoader.hpp"
#include "TexturedQuadEffect.hpp"

namespace OutputMode
{
   enum Enum {
      TargetTexture,
      CurrentBufferScaled,
      CurrentBufferOriginal
   };
}

class Simulation
{
public:
   Simulation();
   ~Simulation();

   bool setup(const GLuint windowWidth, const GLuint windowHeight, const GLuint viewportWidth, const GLuint viewportHeight, AssetLoader& assetLoader);

   void tick();
   void toggleOutputTexture();

private:
   bool load_content(AssetLoader&);

   bool create_render_target();
   void create_random_triangles();
   void copy_current_to_best_triangles();
   void copy_best_to_current_triangles();
   void mutate_triangles();
   void fill_target_buffer();
   void draw_triangles();
   void compare_fitness();
   void draw_output();
   void create_random_vertex(const int vertexIndex);

   void read_back_buffer(unsigned char* buffer);
   unsigned long long calculate_fitness();
   unsigned long long calculate_total_fitness(const unsigned char* buffer);
      
private:
   GLuint viewportWidth, viewportHeight;
   GLuint windowWidth, windowHeight;

   GLuint program;
   GLuint positionAttribute, colorAttribute;
   GLuint vboId;
   
   OutputMode::Enum outputMode;

   unsigned char* currentBuffer;
   unsigned char* targetBuffer;
   bool filledTargetBuffer;

   Texture texture;
   TexturedQuadEffect texturedQuadEffect;
 
   bool renderTextureActiveA;
   GLuint renderTextureIdA, renderTextureIdB;
   GLuint fbo;
   GLint originalFbo;

   // Genetics
   int currentVertexCount;
   ColorVertex currentVertices[Constants::MaximumVertexCount];

   unsigned long long bestFitness;
   int bestVertexCount;
   ColorVertex bestVertices[Constants::MaximumVertexCount];

   unsigned long long targetTotalFitness;
};
