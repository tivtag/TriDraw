#include "Effect.hpp"

#include <stdio.h>
#include <stdlib.h>

GLuint loadShader(const GLenum shaderType, const char* sourceCode) 
{
   GLuint shader = glCreateShader(shaderType);

   if(shader)
   {
      glShaderSource(shader, 1, &sourceCode, NULL);
      glCompileShader(shader);

      GLint compiled = 0;
      glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

      if(!compiled)
      {
         GLint infoLength = 0;
         glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);

         if(infoLength) 
         {
            char* buffer = (char*)malloc(infoLength);

            if(buffer)
            {
               glGetShaderInfoLog(shader, infoLength, NULL, buffer);
               LOGE("Could not compile shader %d:\n%s\n", shaderType, buffer);
               free(buffer);
            }

            glDeleteShader(shader);
            shader = 0;
         }
      }
   }

   return shader;
}

GLuint createProgram(const char* vertexShaderCode, const char* fragmentShaderCode) 
{
   const GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexShaderCode);
   if(!vertexShader) 
   {
      LOGE("Error loading vertex shader");
      return 0;
   }

   const GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
   if(!fragmentShader)
   {
      LOGE("Error loading fragument shader");
      glDeleteShader(vertexShader);
      return 0;
   }

   GLuint program = glCreateProgram();
   if(program) 
   {
      glAttachShader(program, vertexShader);
      checkGlError("glAttachShader");

      glAttachShader(program, fragmentShader);
      checkGlError("glAttachShader");

      glLinkProgram(program);
      GLint linkStatus = GL_FALSE;
      glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

      if(linkStatus != GL_TRUE)
      {
         GLint bufferLength = 0;
         glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufferLength);

         if(bufferLength)
         {
            char* buffer = (char*)malloc(bufferLength);

            if(buffer)
            {
               glGetProgramInfoLog(program, bufferLength, NULL, buffer);
               LOGE("Could not link program:\n%s\n", buffer);
               free(buffer);
            }
         }
         
         glDeleteShader(vertexShader);
         glDeleteShader(fragmentShader);
         glDeleteProgram(program);
         program = 0;
      }
   }

   return program;
}
