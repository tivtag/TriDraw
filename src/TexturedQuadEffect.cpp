#include "TexturedQuadEffect.hpp"
#include "Texture.hpp"
#include "Effect.hpp"

TexturedQuadEffect::TexturedQuadEffect()
   : program(0), positionAttribute(0), uvAttribute(0), _flipY(false), vbo(0), flipVbo(0)
{
}

TexturedQuadEffect::~TexturedQuadEffect()
{
   glDeleteBuffers(1, &vbo);
   glDeleteBuffers(1, &flipVbo);

   // ToDo: Delete shaders
   glDeleteProgram(program);
}

bool TexturedQuadEffect::load()
{  
   const char* VertexShaderCode = "                            \n"
      "attribute highp vec4	a_position;                      \n"
      "attribute mediump vec4 a_uv;                            \n"
      "                                                        \n"
      "varying mediump vec2	v_uv;                            \n"
      "                                                        \n"
      "void main(void)                                         \n"
      "{                                                       \n"
      "   gl_Position = a_position;                            \n"
      "   v_uv        = a_uv.xy;                               \n"
      "}";
   
   const char* FragmentShaderCode = "                          \n"
      "uniform sampler2D u_sampler2d;                          \n"
      "varying mediump vec2 v_uv;                              \n"
      "                                                        \n"
      "void main(void)                                         \n"
      "{                                                       \n"
      "    gl_FragColor = texture2D(u_sampler2d, v_uv);        \n"
      "}";

   program = createProgram(VertexShaderCode, FragmentShaderCode);
   checkGlError("createProgram");

   if(!program)
   {
      LOGE("Could not create fullscreen program.");
      return false;
   }

   // Get attribute locations
   positionAttribute = glGetAttribLocation(program, "a_position");
   checkGlError("glGetAttribLocation");

   uvAttribute = glGetAttribLocation(program, "a_uv");
   checkGlError("glGetAttribLocation");

   // Vertex Buffer
   glGenBuffers(1, &vbo);
   checkGlError("glGenBuffers");
   {
      const GLfloat fullscreenVertices[] = {
          -1.0f, -1.0f, 0.0f, 1.0f,
           1.0f, -1.0f, 1.0f, 1.0f,
          -1.0f,  1.0f, 0.0f, 0.0f,
                     
          -1.0f,  1.0f, 0.0f, 0.0f,
           1.0f, -1.0f, 1.0f, 1.0f,
           1.0f,  1.0f, 1.0f, 0.0f
      };

      const GLuint vertexStride = 4 * sizeof(GLfloat); // 2 floats for the pos, 2 for the UVs

      // Bind the VBO
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      checkGlError("glBindBuffer");

      // Set the buffer's data
      glBufferData(GL_ARRAY_BUFFER, 6 * vertexStride, fullscreenVertices, GL_STATIC_DRAW);
      checkGlError("glBufferData");

      // Unbind the VBO
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      checkGlError("glBindBuffer 0");
   }

   // Flip
   glGenBuffers(1, &flipVbo);
   checkGlError("glGenBuffers");
   {
      const GLfloat fullscreenVertices[] = {      
          -1.0f,  1.0f, 0.0f, 1.0f,
           1.0f,  1.0f, 1.0f, 1.0f,
          -1.0f, -1.0f, 0.0f, 0.0f,
                     
          -1.0f, -1.0f, 0.0f, 0.0f,
           1.0f,  1.0f, 1.0f, 1.0f,
           1.0f, -1.0f, 1.0f, 0.0f
      };

      const GLuint vertexStride = 4 * sizeof(GLfloat); // 2 floats for the pos, 2 for the UVs

      // Bind the VBO
      glBindBuffer(GL_ARRAY_BUFFER, flipVbo);
      checkGlError("glBindBuffer");

      // Set the buffer's data
      glBufferData(GL_ARRAY_BUFFER, 6 * vertexStride, fullscreenVertices, GL_STATIC_DRAW);
      checkGlError("glBufferData");

      // Unbind the VBO
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      checkGlError("glBindBuffer 0");
   }

   LOGI("Successfully loaded textured quad effect.");
   return true;
}

void TexturedQuadEffect::draw()
{
   glActiveTexture(GL_TEXTURE0);

   // Bind program
   glUseProgram(program);
   checkGlError("glUseProgram textured quad");
   
   const GLuint vertexStride = 4 * sizeof(GLfloat); // 3 floats for the pos, 2 for the UVs

   // Bind the VBO
   glBindBuffer(GL_ARRAY_BUFFER, _flipY ? flipVbo : vbo);
   checkGlError("glBindBuffer textured quad");

   // Pass the vertex data
   glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, vertexStride, 0);
   checkGlError("glVertexAttribPointer");

   glEnableVertexAttribArray(positionAttribute);
   checkGlError("glEnableVertexAttribArray");

   // Pass the texture coordinates data
   glVertexAttribPointer(uvAttribute, 2, GL_FLOAT, GL_FALSE, vertexStride, (void*) (2 * sizeof(GLfloat)));
   checkGlError("glVertexAttribPointer");

   glEnableVertexAttribArray(uvAttribute);
   checkGlError("glEnableVertexAttribArray");

   // Draws a non-indexed triangle array
   glDrawArrays(GL_TRIANGLES, 0, 6);
   checkGlError("glDrawArrays textured quad");

   // unwind
   glDisableVertexAttribArray(positionAttribute);
   glDisableVertexAttribArray(uvAttribute);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glUseProgram(0);
}
