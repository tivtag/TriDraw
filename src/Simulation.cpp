#include "Simulation.hpp"
#include "Constants.hpp"
#include "Random.hpp"
#include "Effect.hpp"
#include "TexturedQuadEffect.hpp"

#include <climits>

Simulation::Simulation()
   : program(0), positionAttribute(0), colorAttribute(0), vboId(0),
     viewportWidth(0), viewportHeight(0), windowWidth(0), windowHeight(0),
     currentBuffer(0), targetBuffer(0), filledTargetBuffer(false),

     renderTextureActiveA(true), renderTextureIdA(0), renderTextureIdB(0),
     fbo(0), originalFbo(0),

     currentVertexCount(0), 
     bestFitness(ULONG_MAX), bestVertexCount(0), targetTotalFitness(0UL), outputMode(OutputMode::CurrentBufferScaled)
{
}

Simulation::~Simulation()
{
   delete currentBuffer;
   delete targetBuffer;
}

bool Simulation::setup(const GLuint windowWidth, const GLuint windowHeight, const GLuint viewportWidth, const GLuint viewportHeight, AssetLoader& assetLoader)
{   
   setup_rand();
   
   this->windowWidth = windowWidth;
   this->windowHeight = windowHeight;
   this->viewportWidth = viewportWidth;
   this->viewportHeight = viewportHeight;

   const char VertexShaderCode[] = 
      "attribute vec4 a_position;      \n"
      "attribute lowp vec4 a_color;    \n"
      "varying lowp vec4 v_color;      \n"
      "                                \n"
      "void main() {                   \n"
      "   gl_Position = a_position;    \n"
      "   v_color = a_color;           \n"
      "}                               \n";

   const char FragmentShaderCode[] = 
      "precision mediump float;        \n"
      "varying lowp vec4 v_color;      \n"
      "void main() {                   \n"
      "  gl_FragColor = v_color;       \n"
      "}                               \n";

   program = createProgram(VertexShaderCode, FragmentShaderCode);
   if(!program)
   {
      LOGE("Could not create program.");
      return false;
   }

   // Load shader attribute locations
   positionAttribute = glGetAttribLocation(program, "a_position");
   checkGlError("glGetAttribLocation");
   LOGI("glGetAttribLocation(\"a_position\") = %d\n", positionAttribute);

   colorAttribute = glGetAttribLocation(program, "a_color");
   checkGlError("glGetAttribLocation");
   LOGI("glGetAttribLocation(\"a_color\") = %d\n", colorAttribute);

   // Load triangle currentBuffer
   glGenBuffers(1, &vboId);
   checkGlError("glGenBuffers");

   glBindBuffer(GL_ARRAY_BUFFER, vboId);
   checkGlError("glBindBuffer");

   // Set the currentBuffer's data
   glBufferData(GL_ARRAY_BUFFER, Constants::MaximumVertexCount * sizeof(ColorVertex), &currentVertices[0], GL_DYNAMIC_DRAW);
   checkGlError("glBufferData");
      
   // Create image buffers
   const int BufferSize = viewportWidth*viewportHeight*4; /* RGBA format*/
   currentBuffer = new unsigned char[BufferSize];
   std::fill_n(currentBuffer, BufferSize, 0);

   targetBuffer = new unsigned char[BufferSize];
   std::fill_n(targetBuffer, BufferSize, 0);
   
   if(!create_render_target())
   {
      LOGE("Could not create render target!");
      return false;
   }
   
   LOGW("Integer Sizes: %d %d", sizeof(unsigned long), sizeof(unsigned long));
   LOGI("Success engine_init_display");
      
   if(!load_content(assetLoader))
   {
      LOGE("Could not load content!");
      return false;
   }

   create_random_triangles();
   copy_current_to_best_triangles();
   LOGI("Simulation is ready to go.");
   return true;
}

bool Simulation::load_content(AssetLoader& assetLoader)
{  
   // Initialize content and effects
   const std::string ImageAssets[] = { 
      std::string("austria-flag.jpg"),
      std::string("metro-1.jpg"),
      std::string("metro-2.jpg")
   };

   const std::size_t ImageAssetCount = sizeof(ImageAssets) / sizeof(ImageAssets[0]);

   // Randomly select one of the assets
   const std::size_t index = random_integer(0U, ImageAssetCount);
   const std::string asset = ImageAssets[index];

   if(!texture.loadFromAsset(asset, assetLoader))
   {
      LOGE("Could not load asset.");
      return false;
   }

   return texturedQuadEffect.load();
}

unsigned long long Simulation::calculate_total_fitness(const unsigned char* buffer)
{
   unsigned long long totalFitness = 0UL;
   
   for(unsigned int x = 0; x < viewportWidth; ++x)
   {
      for(unsigned int y = 0; y < viewportHeight; ++y)
      {
         const int index = (y * viewportWidth) + x;
         const int red   = buffer[index + 0];
         const int green = buffer[index + 1];
         const int blue  = buffer[index + 2];

         totalFitness += red * red + green * green + blue * blue;
      }
   }

   return totalFitness;
}

void Simulation::tick()
{
   glViewport(0, 0, viewportWidth, viewportHeight);

   if(!filledTargetBuffer)
   {
      fill_target_buffer();
   }
    
   glBindFramebuffer(GL_FRAMEBUFFER, fbo);
   {
      mutate_triangles();
      draw_triangles();
      compare_fitness();
   }
   glBindFramebuffer(GL_FRAMEBUFFER, originalFbo);
         
   draw_output();
}

void Simulation::compare_fitness()
{
   read_back_buffer(currentBuffer);
   const unsigned long long currentFitness = calculate_fitness();
    
   if(currentFitness <= bestFitness)
   {
      bestFitness = currentFitness;
      copy_current_to_best_triangles();

      // Toggle double buffered fbo
      renderTextureActiveA = !renderTextureActiveA;
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTextureActiveA ? renderTextureIdA : renderTextureIdB, 0);
#if DEBUG
      const unsigned long long currentTotalFitness = calculate_total_fitness(currentBuffer);
      LOGI("Found new best fitness %ul (%ul of %ul) !!! verts=%i", bestFitness, currentTotalFitness, targetTotalFitness, currentVertexCount);
#endif
   }
   else
   {
      copy_best_to_current_triangles();
   }
}

void Simulation::draw_output()
{
   // upscale
   if( outputMode != OutputMode::CurrentBufferOriginal )
      glViewport(0, 0, windowWidth, windowHeight);

   const GLuint textureId = outputMode == OutputMode::TargetTexture ? texture.id() : (renderTextureActiveA ? renderTextureIdB : renderTextureIdA);
   glBindTexture(GL_TEXTURE_2D, textureId);
   texturedQuadEffect.draw();
   glBindTexture(GL_TEXTURE_2D, 0);
}
 
bool Simulation::create_render_target()
{
   // Get the currently bound frame buffer object. On most platforms this just gives 0.
   glGetIntegerv(GL_FRAMEBUFFER_BINDING, &originalFbo);

   // Create a texture A for rendering to
   glGenTextures(1, &renderTextureIdA);
   glBindTexture(GL_TEXTURE_2D, renderTextureIdA);

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewportWidth, viewportHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);
   checkGlError("glTexImage2D A render target");

   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   
   // Create a texture A for rendering to
   glGenTextures(1, &renderTextureIdB);
   glBindTexture(GL_TEXTURE_2D, renderTextureIdB);

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewportWidth, viewportHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);
   checkGlError("glTexImage2D B render target");

   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   // Create the object that will allow us to render to the aforementioned texture
   glGenFramebuffers(1, &fbo);
   checkGlError("glGenFramebuffers");

   glBindFramebuffer(GL_FRAMEBUFFER, fbo);

   // Attach the texture to the FBO
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTextureIdA, 0);
   checkGlError("glFramebufferTexture2D render texture A ");
   
   // Check that our FBO creation was successful
   GLuint uStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

   if(uStatus != GL_FRAMEBUFFER_COMPLETE)
   {
      LOGE("ERROR: Failed to initialise FBO");
      return false;
   }   

   // Clear the colour for the FBO / PBuffer surface
   glClear(GL_COLOR_BUFFER_BIT);

   // Unbind
   glBindTexture(GL_TEXTURE_2D, 0);
   glBindFramebuffer(GL_FRAMEBUFFER, originalFbo);
   LOGI("Successfully loaded render target.");
   return true;
}

void Simulation::create_random_triangles()
{
   currentVertexCount = 0;

   for( int triangleIndex = 0; triangleIndex < Constants::InitialTriangleCount; ++triangleIndex)
   {
      const int vertexIndex = 3 * triangleIndex;
      create_random_vertex(vertexIndex);
   }
}

void Simulation::copy_current_to_best_triangles()
{
   bestVertexCount = currentVertexCount;

   for( int index = 0; index < bestVertexCount; ++index)
   {
      bestVertices[index] = currentVertices[index];
   }
}

void Simulation::copy_best_to_current_triangles()
{
   currentVertexCount = bestVertexCount;

   for( int index = 0; index < bestVertexCount; ++index)
   {
      currentVertices[index] = bestVertices[index];
   }
}

void Simulation::create_random_vertex(const int vertexIndex)
{
   const float centerX = random_float(-1.0f, 1.0f);
   const float centerY = random_float(-1.0f, 1.0f);
   const float Range = 0.15f;

   const unsigned char red = random_byte();
   const unsigned char green = random_byte();
   const unsigned char blue = random_byte();
   const unsigned char alpha = random_byte();

   currentVertices[vertexIndex + 0] = ColorVertex(centerX + random_float(-Range, Range), centerY + random_float(-Range, Range), red, green, blue, alpha);
   currentVertices[vertexIndex + 1] = ColorVertex(centerX + random_float(-Range, Range), centerY + random_float(-Range, Range), red, green, blue, alpha);
   currentVertices[vertexIndex + 2] = ColorVertex(centerX + random_float(-Range, Range), centerY + random_float(-Range, Range), red, green, blue, alpha);
   currentVertexCount += 3;
}

void Simulation::mutate_triangles()
{
   if(should_mutate(Constants::AddPolygonMutationRate))
   {
      if((currentVertexCount+3) < Constants::MaximumVertexCount)
      {
         create_random_vertex(currentVertexCount);
      }
   }

   // move triangle
   if(should_mutate(Constants::MovePolygonMutationRate))
   {
      const int triangleCount = currentVertexCount/3;
      if(triangleCount > 0)
      {
         const int triangleIndexA = random_integer(0, triangleCount);
         const int triangleIndexB = random_integer(0, triangleCount);

         const ColorVertex b1 = currentVertices[triangleIndexB + 0];
         const ColorVertex b2 = currentVertices[triangleIndexB + 1];
         const ColorVertex b3 = currentVertices[triangleIndexB + 2];

         currentVertices[triangleIndexB + 0] = currentVertices[triangleIndexA + 0];
         currentVertices[triangleIndexB + 1] = currentVertices[triangleIndexA + 1];
         currentVertices[triangleIndexB + 2] = currentVertices[triangleIndexA + 2];

         currentVertices[triangleIndexA + 0] = b1;
         currentVertices[triangleIndexA + 1] = b2;
         currentVertices[triangleIndexA + 2] = b3;
      }
   }

   // remove triangle   
   if(should_mutate(Constants::RemovePolygonMutationRate))
   {
      const int triangleCount = currentVertexCount/3;
      if(triangleCount > 0)
      {
         const int triangleIndex = random_integer(0, triangleCount);
         const int nextTriangleIndex = triangleIndex + 1;

         for(int vertexIndex = nextTriangleIndex*3; vertexIndex < currentVertexCount; ++vertexIndex)
         {
            currentVertices[vertexIndex - 3] = currentVertices[vertexIndex];
         }

         currentVertexCount -= 3;
      }
   }

   // mutate vertices
   for(int vertexIndex = 0; vertexIndex < currentVertexCount; ++vertexIndex)
   {
      currentVertices[vertexIndex].mutate();
   }
}

void Simulation::draw_triangles()
{
   // Bind and enable
   glBindBuffer(GL_ARRAY_BUFFER, vboId);
   
   // Send changed vertex data to GPU
   glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ColorVertex)*currentVertexCount, currentVertices);

   // Clear currentBuffer
   glClearColor(1, 1, 1, 1);    
   glClear(GL_COLOR_BUFFER_BIT);

   // Start using shader
   glUseProgram(program);
   checkGlError("glUseProgram"); 

   // Setup Vertex Data
   const GLuint vertexStride = sizeof(ColorVertex);

   // Position
   glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, vertexStride, 0);
   checkGlError("glVertexAttribPointer position");

   glEnableVertexAttribArray(positionAttribute);
   checkGlError("glEnableVertexAttribArray position");

   // Color
   glVertexAttribPointer(colorAttribute, 4, GL_UNSIGNED_BYTE, GL_TRUE, vertexStride, (void*)(2 * sizeof(GLfloat)));
   checkGlError("glVertexAttribPointer color");

   glEnableVertexAttribArray(colorAttribute);
   checkGlError("glEnableVertexAttribArray color");

   // Draw Triangles
   glDrawArrays(GL_TRIANGLES, 0, currentVertexCount);
   checkGlError("glDrawArrays");

   // Disable
   glDisableVertexAttribArray(positionAttribute);
   glDisableVertexAttribArray(colorAttribute);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Simulation::read_back_buffer(unsigned char* buffer)
{
   glReadPixels(
      0, 0, // first pixel to read
      viewportWidth, viewportHeight,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      buffer
   );

   checkGlError("glReadPixels");
}

unsigned long long Simulation::calculate_fitness()
{
   unsigned long long fitness = 0UL;
   
   for(unsigned int y = 0; y < viewportHeight; ++y)
   {
      for(unsigned int x = 0; x < viewportWidth; ++x)
      {
         const unsigned int index = (4U * y * viewportWidth) + x*4U;
         const int red   = targetBuffer[index + 0] - currentBuffer[index + 0];
         const int green = targetBuffer[index + 1] - currentBuffer[index + 1];
         const int blue  = targetBuffer[index + 2] - currentBuffer[index + 2];
         
         fitness += std::abs(red) + std::abs(green)+ std::abs(blue);
         //fitness += red * red + green * green + blue * blue;
      }
   }

   if(fitness < 10000UL)
   {
      const unsigned long long BufferSize = viewportHeight * viewportWidth * 4UL;
      LOGW("Very low fitness! Bug?! vw=%iu vh=%iu BufferSize=%lu", viewportWidth, viewportHeight, BufferSize);
      LOGW("%d %d", sizeof(unsigned long), sizeof(unsigned long));
   }

   return fitness;
}

void Simulation::fill_target_buffer()
{
   glClearColor(1, 1, 1, 1);
   glClear(GL_COLOR_BUFFER_BIT);
   glBindTexture(GL_TEXTURE_2D, texture.id());

   texturedQuadEffect.flipY(true);
   texturedQuadEffect.draw();
   texturedQuadEffect.flipY(false);
   read_back_buffer(targetBuffer);

   glClearColor(1, 1, 1, 1);
   glClear(GL_COLOR_BUFFER_BIT);
            
   glBindTexture(GL_TEXTURE_2D, 0);
   filledTargetBuffer = true;

   targetTotalFitness = calculate_total_fitness(targetBuffer);
   LOGI("Filled target buffer");
}

void Simulation::toggleOutputTexture()
{
#ifdef DEBUG
   if(outputMode == OutputMode::CurrentBufferScaled)
      outputMode = OutputMode::TargetTexture;
   else if(outputMode == OutputMode::TargetTexture)
      outputMode = OutputMode::CurrentBufferOriginal;
   else
      outputMode = OutputMode::CurrentBufferScaled;
#else
   if(outputMode == OutputMode::CurrentBufferScaled)
      outputMode = OutputMode::TargetTexture;
   else
      outputMode = OutputMode::CurrentBufferScaled;
#endif
}
