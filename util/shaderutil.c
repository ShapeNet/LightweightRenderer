/**
 * Utilities for OpenGL shading language
 *
 * Brian Paul
 * 9 April 2008
 */


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include "glut_wrap.h"
#include "shaderutil.h"

/** time to compile previous shader */
static GLdouble CompileTime = 0.0;

/** time to linke previous program */
static GLdouble LinkTime = 0.0;

PFNGLCREATESHADERPROC CreateShader = NULL;
PFNGLDELETESHADERPROC DeleteShader = NULL;
PFNGLSHADERSOURCEPROC ShaderSource = NULL;
PFNGLGETSHADERIVPROC GetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog = NULL;
PFNGLCREATEPROGRAMPROC CreateProgram = NULL;
PFNGLDELETEPROGRAMPROC DeleteProgram = NULL;
PFNGLATTACHSHADERPROC AttachShader = NULL;
PFNGLLINKPROGRAMPROC LinkProgram = NULL;
PFNGLUSEPROGRAMPROC UseProgram = NULL;
PFNGLGETPROGRAMIVPROC GetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog = NULL;
PFNGLVALIDATEPROGRAMARBPROC ValidateProgramARB = NULL;
PFNGLUNIFORM1IPROC Uniform1i = NULL;
PFNGLUNIFORM1FVPROC Uniform1fv = NULL;
PFNGLUNIFORM2FVPROC Uniform2fv = NULL;
PFNGLUNIFORM3FVPROC Uniform3fv = NULL;
PFNGLUNIFORM4FVPROC Uniform4fv = NULL;
PFNGLUNIFORMMATRIX4FVPROC UniformMatrix4fv = NULL;
PFNGLGETACTIVEATTRIBPROC GetActiveAttrib = NULL;
PFNGLGETATTRIBLOCATIONPROC GetAttribLocation = NULL;

static void GLAPIENTRY
fake_ValidateProgram(GLuint prog)
{
   (void) prog;
}

GLboolean
ShadersSupported(void)
{
   if (GLEW_VERSION_2_0) {
      CreateShader = glCreateShader;
      DeleteShader = glDeleteShader;
      ShaderSource = glShaderSource;
      GetShaderiv = glGetShaderiv;
      GetShaderInfoLog = glGetShaderInfoLog;
      CreateProgram = glCreateProgram;
      DeleteProgram = glDeleteProgram;
      AttachShader = glAttachShader;
      LinkProgram = glLinkProgram;
      UseProgram = glUseProgram;
      GetProgramiv = glGetProgramiv;
      GetProgramInfoLog = glGetProgramInfoLog;
      ValidateProgramARB = (GLEW_ARB_shader_objects)
	 ? glValidateProgramARB : fake_ValidateProgram;
      Uniform1i = glUniform1i;
      Uniform1fv = glUniform1fv;
      Uniform2fv = glUniform2fv;
      Uniform3fv = glUniform3fv;
      Uniform4fv = glUniform4fv;
      UniformMatrix4fv = glUniformMatrix4fv;
      GetActiveAttrib = glGetActiveAttrib;
      GetAttribLocation = glGetAttribLocation;
      return GL_TRUE;
   }
   else if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader
	    && GLEW_ARB_shader_objects) {
      fprintf(stderr, "Warning: Trying ARB GLSL instead of OpenGL 2.x.  This may not work.\n");
      CreateShader = glCreateShaderObjectARB;
      DeleteShader = glDeleteObjectARB;
      ShaderSource = glShaderSourceARB;
      GetShaderiv = glGetObjectParameterivARB;
      GetShaderInfoLog = glGetInfoLogARB;
      CreateProgram = glCreateProgramObjectARB;
      DeleteProgram = glDeleteObjectARB;
      AttachShader = glAttachObjectARB;
      LinkProgram = glLinkProgramARB;
      UseProgram = glUseProgramObjectARB;
      GetProgramiv = glGetObjectParameterivARB;
      GetProgramInfoLog = glGetInfoLogARB;
      ValidateProgramARB = glValidateProgramARB;
      Uniform1i = glUniform1iARB;
      Uniform1fv = glUniform1fvARB;
      Uniform2fv = glUniform2fvARB;
      Uniform3fv = glUniform3fvARB;
      Uniform4fv = glUniform4fvARB;
      UniformMatrix4fv = glUniformMatrix4fvARB;
      GetActiveAttrib = glGetActiveAttribARB;
      GetAttribLocation = glGetAttribLocationARB;
      return GL_TRUE;
   }
   fprintf(stderr, "Sorry, GLSL not supported with this OpenGL.\n");
   return GL_FALSE;
}


GLuint
CompileShaderText(GLenum shaderType, const char *text)
{
   GLuint shader;
   GLint stat;
   GLdouble t0, t1;

   shader = CreateShader(shaderType);
   ShaderSource(shader, 1, (const GLchar **) &text, NULL);

   t0 = glutGet(GLUT_ELAPSED_TIME) * 0.001;
   glCompileShader(shader);
   t1 = glutGet(GLUT_ELAPSED_TIME) * 0.001;

   CompileTime = t1 - t0;

   GetShaderiv(shader, GL_COMPILE_STATUS, &stat);
   if (!stat) {
      GLchar log[1000];
      GLsizei len;
      GetShaderInfoLog(shader, 1000, &len, log);
      fprintf(stderr, "Error: problem compiling shader: %s\n", log);
      exit(1);
   }
   else {
      /*printf("Shader compiled OK\n");*/
   }
   return shader;
}


/**
 * Read a shader from a file.
 */
GLuint
CompileShaderFile(GLenum shaderType, const char *filename)
{
   const int max = 100*1000;
   int n;
   char *buffer = (char*) malloc(max);
   GLuint shader;
   FILE *f;

   f = fopen(filename, "r");
   if (!f) {
      fprintf(stderr, "Unable to open shader file %s\n", filename);
      free(buffer);
      return 0;
   }

   n = fread(buffer, 1, max, f);
   /*printf("read %d bytes from shader file %s\n", n, filename);*/
   if (n > 0) {
      buffer[n] = 0;
      shader = CompileShaderText(shaderType, buffer);
   }
   else {
      fclose(f);
      free(buffer);
      return 0;
   }

   fclose(f);
   free(buffer);

   return shader;
}


GLuint
LinkShaders(GLuint vertShader, GLuint fragShader)
{
   return LinkShaders3(vertShader, 0, fragShader);
}


GLuint
LinkShaders3(GLuint vertShader, GLuint geomShader, GLuint fragShader)
{
   GLuint program = CreateProgram();
   GLdouble t0, t1;

   assert(vertShader || fragShader);

   if (vertShader)
      AttachShader(program, vertShader);
   if (geomShader)
      AttachShader(program, geomShader);
   if (fragShader)
      AttachShader(program, fragShader);

   t0 = glutGet(GLUT_ELAPSED_TIME) * 0.001;
   LinkProgram(program);
   t1 = glutGet(GLUT_ELAPSED_TIME) * 0.001;

   LinkTime = t1 - t0;

   /* check link */
   {
      GLint stat;
      GetProgramiv(program, GL_LINK_STATUS, &stat);
      if (!stat) {
         GLchar log[1000];
         GLsizei len;
         GetProgramInfoLog(program, 1000, &len, log);
         fprintf(stderr, "Shader link error:\n%s\n", log);
         return 0;
      }
   }

   return program;
}


GLuint
LinkShaders3WithGeometryInfo(GLuint vertShader, GLuint geomShader, GLuint fragShader,
                             GLint verticesOut, GLenum inputType, GLenum outputType)
{
  GLuint program = CreateProgram();
  GLdouble t0, t1;

  assert(vertShader || fragShader);

  if (vertShader)
    AttachShader(program, vertShader);
  if (geomShader) {
    AttachShader(program, geomShader);
    glProgramParameteriARB(program, GL_GEOMETRY_VERTICES_OUT_ARB, verticesOut);
    glProgramParameteriARB(program, GL_GEOMETRY_INPUT_TYPE_ARB, inputType);
    glProgramParameteriARB(program, GL_GEOMETRY_OUTPUT_TYPE_ARB, outputType);
  }
  if (fragShader)
    AttachShader(program, fragShader);

  t0 = glutGet(GLUT_ELAPSED_TIME) * 0.001;
  LinkProgram(program);
  t1 = glutGet(GLUT_ELAPSED_TIME) * 0.001;

  LinkTime = t1 - t0;

  /* check link */
  {
    GLint stat;
    GetProgramiv(program, GL_LINK_STATUS, &stat);
    if (!stat) {
      GLchar log[1000];
      GLsizei len;
      GetProgramInfoLog(program, 1000, &len, log);
      fprintf(stderr, "Shader link error:\n%s\n", log);
      return 0;
    }
  }

  return program;
}


GLboolean
ValidateShaderProgram(GLuint program)
{
   GLint stat;
   ValidateProgramARB(program);
   GetProgramiv(program, GL_VALIDATE_STATUS, &stat);

   if (!stat) {
      GLchar log[1000];
      GLsizei len;
      GetProgramInfoLog(program, 1000, &len, log);
      fprintf(stderr, "Program validation error:\n%s\n", log);
      fflush(stderr);
      return 0;
   }

   return (GLboolean) stat;
}


GLdouble
GetShaderCompileTime(void)
{
   return CompileTime;
}


GLdouble
GetShaderLinkTime(void)
{
   return LinkTime;
}


void
SetUniformValues(GLuint program, struct uniform_info uniforms[])
{
   GLuint i;

   for (i = 0; uniforms[i].name; i++) {
      uniforms[i].location
         = glGetUniformLocation(program, uniforms[i].name);

      switch (uniforms[i].type) {
      case GL_INT:
      case GL_SAMPLER_1D:
      case GL_SAMPLER_2D:
      case GL_SAMPLER_3D:
      case GL_SAMPLER_CUBE:
      case GL_SAMPLER_2D_RECT_ARB:
      case GL_SAMPLER_1D_SHADOW:
      case GL_SAMPLER_2D_SHADOW:
      case GL_SAMPLER_1D_ARRAY:
      case GL_SAMPLER_2D_ARRAY:
      case GL_SAMPLER_1D_ARRAY_SHADOW:
      case GL_SAMPLER_2D_ARRAY_SHADOW:
         assert(uniforms[i].value[0] >= 0.0F);
         Uniform1i(uniforms[i].location,
                     (GLint) uniforms[i].value[0]);
         break;
      case GL_FLOAT:
         Uniform1fv(uniforms[i].location, 1, uniforms[i].value);
         break;
      case GL_FLOAT_VEC2:
         Uniform2fv(uniforms[i].location, 1, uniforms[i].value);
         break;
      case GL_FLOAT_VEC3:
         Uniform3fv(uniforms[i].location, 1, uniforms[i].value);
         break;
      case GL_FLOAT_VEC4:
         Uniform4fv(uniforms[i].location, 1, uniforms[i].value);
         break;
      case GL_FLOAT_MAT4:
         UniformMatrix4fv(uniforms[i].location, 1, GL_FALSE,
                          uniforms[i].value);
         break;
      default:
         if (strncmp(uniforms[i].name, "gl_", 3) == 0) {
            /* built-in uniform: ignore */
         }
         else {
            fprintf(stderr,
                    "Unexpected uniform data type in SetUniformValues\n");
            abort();
         }
      }
   }
}


/** Get list of uniforms used in the program */
GLuint
GetUniforms(GLuint program, struct uniform_info uniforms[])
{
   GLint n, max, i;

   GetProgramiv(program, GL_ACTIVE_UNIFORMS, &n);
   GetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max);

   for (i = 0; i < n; i++) {
      GLint size, len;
      GLenum type;
      char name[100];

      glGetActiveUniform(program, i, 100, &len, &size, &type, name);

      uniforms[i].name = strdup(name);
      uniforms[i].size = size;
      uniforms[i].type = type;
      uniforms[i].location = glGetUniformLocation(program, name);
   }

   uniforms[i].name = NULL; /* end of list */

   return n;
}


void
PrintUniforms(const struct uniform_info uniforms[])
{
   GLint i;

   printf("Uniforms:\n");

   for (i = 0; uniforms[i].name; i++) {
      printf("  %d: %s size=%d type=0x%x loc=%d value=%g, %g, %g, %g\n",
             i,
             uniforms[i].name,
             uniforms[i].size,
             uniforms[i].type,
             uniforms[i].location,
             uniforms[i].value[0],
             uniforms[i].value[1],
             uniforms[i].value[2],
             uniforms[i].value[3]);
   }
}


/** Get list of attribs used in the program */
GLuint
GetAttribs(GLuint program, struct attrib_info attribs[])
{
   GLint n, max, i;

   GetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &n);
   GetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max);

   for (i = 0; i < n; i++) {
      GLint size, len;
      GLenum type;
      char name[100];

      GetActiveAttrib(program, i, 100, &len, &size, &type, name);

      attribs[i].name = strdup(name);
      attribs[i].size = size;
      attribs[i].type = type;
      attribs[i].location = GetAttribLocation(program, name);
   }

   attribs[i].name = NULL; /* end of list */

   return n;
}


void
PrintAttribs(const struct attrib_info attribs[])
{
   GLint i;

   printf("Attribs:\n");

   for (i = 0; attribs[i].name; i++) {
      printf("  %d: %s size=%d type=0x%x loc=%d\n",
             i,
             attribs[i].name,
             attribs[i].size,
             attribs[i].type,
             attribs[i].location);
   }
}
