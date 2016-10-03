
#ifndef IDLIB_SDL
#define IDLIB_SDL

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif
#include <gl/gl.h>
#include "glext.h"

#define GLDECLARE(x) extern PFN##x##PROC x
#define GLDEFINE(x) PFN##x##PROC x
#define GLGETPROC(x) x = (PFN##x##PROC)SDL_GL_GetProcAddress(#x); assert(x);

typedef struct {
	short height;
	short location[256];
	unsigned char width[256];
	short total_width;
	GLuint texture;
} font_t;

extern SDL_Window * window;
extern int surfaceWidth, surfaceHeight;

/*
   IDLIBC_SHADERS
*/

void SHADERS_Init(void);
void UseColorShader(float * matrix, float * vertices, uint8_t color);
void UseTextureShader(float * matrix, float * vertices, int blend, unsigned int texture, float * texcoord);
void UseFontShader(float * matrix, float * vertices, int blend, uint8_t color, GLuint texture, float * texcoord);
void UseFadeShader(float fade, GLuint texture, float * vertices, float * texcoords);

#endif IDLIB_SDL
