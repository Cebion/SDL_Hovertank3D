
#ifndef IDLIB_SDL
#define IDLIB_SDL

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif
#include <GL/gl.h>
#include "glext.h"

#define GLDECLARE(x) extern PFN##x##PROC x
#define GLDEFINE(x) PFN##x##PROC x
#define GLGETPROC(x) x = (PFN##x##PROC)SDL_GL_GetProcAddress(#x); assert(x);

#define _GLDECLARE(x) extern PFN##x##PROC _##x
#define _GLDEFINE(x) PFN##x##PROC _##x
#define _GLGETPROC(x) _##x = (PFN##x##PROC)SDL_GL_GetProcAddress(#x); assert(x);

typedef struct {
	int16_t height;
	int16_t location[256];
	uint8_t width[256];
	int16_t total_width;
	GLuint texture;
} font_t;

extern SDL_Window * window;
extern int windowFullscreen;
extern int windowWidth, windowHeight;
extern int viewportX, viewportY;
extern int viewportWidth, viewportHeight;

/*
   IDLIBC_SHADERS
*/

void SHADERS_Init(void);
void UseColorShader(float * matrix, float * vertices, uint8_t color);
void UseTextureShader(float * matrix, float * vertices, int blend, unsigned int texture, float * texcoord);
void UseFontShader(float * matrix, float * vertices, int blend, uint8_t color, GLuint texture, float * texcoord);
void UseFadeShader(float fade, GLuint texture, float * vertices, float * texcoords);

#endif//IDLIB_SDL
