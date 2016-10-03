
#include "JM_SB.H"
#include "IDLIB.H"

#define SAMPLERATE 44100
#define SAMPLES (SAMPLERATE/70)
const float SAMPLESTEP = 1.f/SAMPLERATE;

static void MyAudioCallback(void *userdata, Uint8 *stream, int len);

#if 0
#define GLEW_STATIC
#include <GL/glew.h>

#else

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

// shaders
#define PFNglCreateShaderPROC PFNGLCREATESHADERPROC
#define PFNglShaderSourcePROC PFNGLSHADERSOURCEPROC
#define PFNglCompileShaderPROC PFNGLCOMPILESHADERPROC
#define PFNglGetShaderivPROC PFNGLGETSHADERIVPROC
#define PFNglGetShaderInfoLogPROC PFNGLGETSHADERINFOLOGPROC
#define PFNglCreateProgramPROC PFNGLCREATEPROGRAMPROC
#define PFNglAttachShaderPROC PFNGLATTACHSHADERPROC
#define PFNglLinkProgramPROC PFNGLLINKPROGRAMPROC
#define PFNglGetProgramivPROC PFNGLGETPROGRAMIVPROC
#define PFNglGetProgramInfoLogPROC PFNGLGETPROGRAMINFOLOGPROC
#define PFNglUseProgramPROC PFNGLUSEPROGRAMPROC
#define PFNglGetUniformLocationPROC PFNGLGETUNIFORMLOCATIONPROC
#define PFNglGetAttribLocationPROC PFNGLGETATTRIBLOCATIONPROC
#define PFNglUniformMatrix4fvPROC PFNGLUNIFORMMATRIX4FVPROC
#define PFNglVertexAttribPointerPROC PFNGLVERTEXATTRIBPOINTERPROC
#define PFNglEnableVertexAttribArrayPROC PFNGLENABLEVERTEXATTRIBARRAYPROC
#define PFNglUniform1fPROC PFNGLUNIFORM1FPROC
#define PFNglUniform4fPROC PFNGLUNIFORM4FPROC
#define PFNglUniform1iPROC PFNGLUNIFORM1IPROC
#define PFNglActiveTexturePROC PFNGLACTIVETEXTUREPROC
#define PFNglUniform1fPROC PFNGLUNIFORM1FPROC


GLDEFINE(glCreateProgram);
GLDEFINE(glCreateShader);
GLDEFINE(glCompileShader);
GLDEFINE(glGetShaderiv);
GLDEFINE(glGetShaderInfoLog);
GLDEFINE(glShaderSource);
GLDEFINE(glAttachShader);
GLDEFINE(glLinkProgram);
GLDEFINE(glGetProgramiv);
GLDEFINE(glGetProgramInfoLog);
GLDEFINE(glUseProgram);
GLDEFINE(glGetUniformLocation);
GLDEFINE(glGetAttribLocation);
GLDEFINE(glUniformMatrix4fv);
GLDEFINE(glVertexAttribPointer);
GLDEFINE(glEnableVertexAttribArray);
GLDEFINE(glUniform1f);
GLDEFINE(glUniform4f);
GLDEFINE(glUniform1i);
GLDEFINE(glActiveTexture);


// frame buffer objects
#define PFNglGenFramebuffersPROC PFNGLGENFRAMEBUFFERSPROC
#define PFNglGenRenderbuffersPROC PFNGLGENRENDERBUFFERSPROC
#define PFNglBindRenderbufferPROC PFNGLBINDRENDERBUFFERPROC
#define PFNglRenderbufferStoragePROC PFNGLRENDERBUFFERSTORAGEPROC
#define PFNglBindFramebufferPROC PFNGLBINDFRAMEBUFFERPROC
#define PFNglFramebufferTexturePROC PFNGLFRAMEBUFFERTEXTUREPROC
#define PFNglFramebufferRenderbufferPROC PFNGLFRAMEBUFFERRENDERBUFFERPROC
#define PFNglCheckFramebufferStatusPROC PFNGLCHECKFRAMEBUFFERSTATUSPROC

GLDEFINE(glGenFramebuffers);
GLDEFINE(glGenRenderbuffers);
GLDEFINE(glBindRenderbuffer);
GLDEFINE(glRenderbufferStorage);
GLDEFINE(glBindFramebuffer);
GLDEFINE(glFramebufferTexture);
GLDEFINE(glFramebufferRenderbuffer);
GLDEFINE(glCheckFramebufferStatus);

#endif

SDL_Window * window;
int surfaceWidth, surfaceHeight;

int processedevents = 0;

#define NUMFRAMEBUFFERS 4
typedef struct {
	GLuint frameBuffer[NUMFRAMEBUFFERS];
	GLuint texture[NUMFRAMEBUFFERS];
	GLuint renderBuffer[NUMFRAMEBUFFERS];
	int width;
	int height;
} framebuffer_t;

framebuffer_t fbo;

matrix4x4_t matProj2D;
matrix4x4_t matProj;

unsigned screenofs;
unsigned int linewidth;

int NBKscan,NBKascii;
char keydown[128];
char key[8],keyB1,keyB2;
long highscore;
int level,bestlevel;

#define DRAWCHAR(x,y,n) DrawChar(x,(y)*8,n)

memptr soundseg;


typedef struct {
	int index;
	int xoffset;
	int yoffset;
} video_fbo_t;

float video_fade = 1;
int video_split = 200;
int video_crtc = 0;
int video_pel = 0;
int video_line_width = 40;
video_fbo_t video_fbo = {0};

int video_screenofs_fbo;
unsigned video_oldscreenofs;
int video_baseofs;
int video_screenofs_x;
int video_screenofs_y;

void GetAddressInfo(int address, int * base, int * fbo_index)
{
	if (video_line_width == 80)
	{
		if (address <= 80*200)
		{
			*base = 0;
			*fbo_index = 0;
			return;
		}
		if (address >= 0x4000 && address <= 0x4000+80*200)
		{
			*base = 0x4000;
			*fbo_index = 1;
			return;
		}
		if (address >= 0x8000 && address <= 0x8000+80*200)
		{
			*base = 0x8000;
			*fbo_index = 2;
			return;
		}
	}
	else if (video_line_width == 40)
	{
		if (address <= 40*200)
		{
			*base = 0;
			*fbo_index = 0;
			return;
		}
		if (address >= 0x3700 && address <= 0x3700+40*200)
		{
			*base = 0x3700;
			*fbo_index = 1;
			return;
		}
		if (address >= 0x7a00 && address <= 0x7a00+40*200)
		{
			*base = 0x7a00;
			*fbo_index = 2;
			return;
		}
		if (address >= 0xbd00 && address <= 0xbd00+40*200)
		{
			*base = 0xbd00;
			*fbo_index = 3;
			return;
		}
	}
	assert(0); // unknown address
}

memptr CreateTexture(int width, int height, const memptr planes, unsigned char background)
{
	int plane_width = width;
	width *= 8;
	int size = width * height;

	unsigned int *data;
	MMGetPtr((memptr*)&data,size*4);

	unsigned char *plane0 = (unsigned char*)planes;
	unsigned char *plane1 = plane0 + plane_width*height;
	unsigned char *plane2 = plane1 + plane_width*height;
	unsigned char *plane3 = plane2 + plane_width*height;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x+=8)
		{
			unsigned char by0 = plane0[y*plane_width+x/8];
			unsigned char by1 = plane1[y*plane_width+x/8];
			unsigned char by2 = plane2[y*plane_width+x/8];
			unsigned char by3 = plane3[y*plane_width+x/8];
			for (int b = 0; b < 8; b++)
			{
				unsigned char index =
					(((by0 >> (7-b))&1)<<0) |
					(((by1 >> (7-b))&1)<<1) |
					(((by2 >> (7-b))&1)<<2) |
					(((by3 >> (7-b))&1)<<3);
				if (index == background)
				{
					data[y*width+x+b] = 0;
				}
				else
				{
					data[y*width+x+b] = GetColor(index);
				}
			}
		}
	}

	gltexture *gltex;
	MMGetPtr((memptr*)&gltex, sizeof(*gltex));
	gltex->width = width;
	gltex->height = height;
	glGenTextures(1, &gltex->texture);

	glBindTexture(GL_TEXTURE_2D, gltex->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	MMFreePtr((memptr*)&data);

	return gltex;
}


typedef struct {
	short height;
	short location[256];
	unsigned char width[256];
	short total_width;
	GLuint texture;
} font_t;

#ifdef CreateFont
#undef CreateFont
#endif
void CreateFont()
{
	char * fontseg = (char*)grsegs[STARTFONT];
	font_t * font = (font_t*)fontseg;

	int total_width = 0;
	for (int i = 0; i < 256; i++)
	{
		total_width += font->width[i];
	}
	assert(total_width < 32768);
	int height = font->height;

	uint32_t * pixels;
	MMGetPtr((memptr*)&pixels, total_width*height*4);

	font_t * final_font;
	MMGetPtr((memptr*)&final_font, sizeof(font_t));

	final_font->height = height;
	memcpy(final_font->width, font->width, sizeof(final_font->width));
	final_font->total_width = total_width;

	int dx = 0;
	for (int i = 0; i < 256; i++)
	{
		final_font->location[i] = dx;
		int cwidth = font->width[i];
		int csize = (font->width[i]+7)/8 * height;
		char *c = (char*)fontseg + font->location[i] + csize; // TEST
		int x, y, b;
		if (cwidth == 0)
		{
			continue;
		}
		for (y = 0; y < font->height; y++)
		{
			int yoffset = y*total_width;
			for (x = 0; x < cwidth; x+= 8)
			{
				int xyoffset = x + dx + yoffset;
				uint8_t bits = *c++;
				int bc = cwidth-x;
				if (bc > 8) bc = 8;
				for (b = 0; b < bc; b++)
				{
					int offset = xyoffset + b;
					assert(offset*4 < total_width*height*4);
					pixels[offset] = ((bits>>(7-b))&1)? 0xFFFFFFFF : 0;
					MMValidatePtr((memptr*)&pixels);
				}
			}
		}
		dx += cwidth;
	}

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, total_width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	final_font->texture = texture;

	MMFreePtr(&grsegs[STARTFONT]);
	grsegs[STARTFONT] = (memptr)final_font;

	MMFreePtr((memptr*)&pixels);
}



typedef struct
{
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint program;
	GLuint matrix;
	GLuint position;
} common_shader_t;


int CreateCommonShader(common_shader_t * shader, const GLchar * vertexShaderSource, const GLchar * fragmentShaderSource)
{
	GLint logLength;

	shader->vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader->vertexShader, 1, &vertexShaderSource, 0);
	glCompileShader(shader->vertexShader);
	glGetShaderiv(shader->vertexShader, GL_INFO_LOG_LENGTH , &logLength);
	if (logLength > 1)
	{
		GLchar* log = (GLchar*)alloca(logLength);
		glGetShaderInfoLog(shader->vertexShader, logLength, 0, log);
		SDL_Log(log);
		assert(0);
		return 0;
	}
 
	shader->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader->fragmentShader, 1, &fragmentShaderSource, 0);
	glCompileShader(shader->fragmentShader);
	glGetShaderiv(shader->fragmentShader, GL_INFO_LOG_LENGTH , &logLength);
	if (logLength > 1)
	{
		GLchar* log = (GLchar*)alloca(logLength);
		glGetShaderInfoLog(shader->fragmentShader, logLength, 0, log);
		SDL_Log(log);
		assert(0);
		return 0;
	}

	shader->program = glCreateProgram();
	glAttachShader(shader->program, shader->vertexShader);
	glAttachShader(shader->program, shader->fragmentShader);
	glLinkProgram(shader->program);
	glGetProgramiv(shader->program, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 1)
	{
		GLchar* log = (GLchar*)alloca(logLength);
		glGetProgramInfoLog(shader->program, logLength, 0, log);
		SDL_Log(log);
		assert(0);
		return 0;
	}

	glUseProgram(shader->program);
 
	shader->matrix = glGetUniformLocation(shader->program, "matrix");
	shader->position = glGetAttribLocation(shader->program, "position");

	return 1;
}

void UseCommonShader(const common_shader_t * shader, const float * matrix, const float * vertices)
{
	glUseProgram(shader->program);

	glUniformMatrix4fv(shader->matrix, 1, GL_FALSE, matrix);

	glVertexAttribPointer(shader->position, 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(shader->position);
}




struct {
	common_shader_t shader;
	GLuint color;
} colorShader;

void CreateColorShader()
{
	const GLchar vertexShaderSource[] = {
		"#version 150\n"
		"uniform mat4 matrix;\n"
		"in vec3 position;\n"
		"void main()\n"
		"{\n"
		"    gl_Position = matrix * vec4(position, 1.0);\n"
		"}"
	};

	const GLchar fragmentShaderSource[] = {
		"#version 150\n"
		"uniform vec4 color;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"    outputColor = color;\n"
		"}"
	};

	CreateCommonShader(&colorShader.shader, vertexShaderSource, fragmentShaderSource);

	glUseProgram(colorShader.shader.program);

	colorShader.color = glGetUniformLocation(colorShader.shader.program, "color");
}

void UseColorShader(float * matrix, float * vertices, uint8_t color)
{
	UseCommonShader(&colorShader.shader, matrix, vertices);

	uint8_t u8[4];
	*(uint32_t*)u8 = GetColor(color);
	glUniform4f(colorShader.color, u8[0]/255.f, u8[1]/255.f, u8[2]/255.f, u8[3]/255.f);

	glDisable(GL_BLEND);
}




struct {
	common_shader_t shader;
	GLuint texcoord;
	GLint sampler;
} textureShader;

void CreateTextureShader()
{
	const GLchar vertexShaderSource[] = {
		"#version 150\n"
		"uniform mat4 matrix;\n"
		"in vec3 position;\n"
		"in vec2 texcoord;\n"
		"out vec2 uv;\n"
		"void main()\n"
		"{\n"
		"    gl_Position = matrix * vec4(position, 1.0);\n"
		"    uv = texcoord;\n"
		"}"
	};

	const GLchar fragmentShaderSource[] = {
		"#version 150\n"
		"uniform sampler2D texUnit;\n"
		"in vec2 uv;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"    outputColor = texture(texUnit, uv);\n"
		"    if (outputColor.a < 0.1) discard;\n"
		"}"
	};

	CreateCommonShader(&textureShader.shader, vertexShaderSource, fragmentShaderSource);

	glUseProgram(textureShader.shader.program);

	textureShader.texcoord = glGetAttribLocation(textureShader.shader.program, "texcoord");
	textureShader.sampler = glGetUniformLocation(textureShader.shader.program, "texUnit");
}

void UseTextureShader(float * matrix, float * vertices, int blend, unsigned int texture, float * texcoord)
{
	UseCommonShader(&textureShader.shader, matrix, vertices);

	glUniform1i(textureShader.sampler, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	if (blend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	glVertexAttribPointer(textureShader.texcoord, 2, GL_FLOAT, GL_FALSE, 0, texcoord);
	glEnableVertexAttribArray(textureShader.texcoord);
}



struct {
	common_shader_t shader;
	GLuint texcoord;
	GLuint color;
	GLint sampler;
//	GLint sampler2;
} fontShader;

void CreateFontShader()
{
	const GLchar vertexShaderSource[] = {
		"#version 150\n"
		"uniform mat4 matrix;\n"
		"in vec3 position;\n"
		"in vec2 texcoord;\n"
		"out vec2 uv;\n"
//		"out vec2 uv2;\n" // test
		"void main()\n"
		"{\n"
		"    gl_Position = matrix * vec4(position, 1.0);\n"
		"    uv = texcoord;\n"
//		"    uv2 = position.xy * vec2(0.5,-0.5) + vec2(0.5,0.5);\n" // test
		"}"
	};

	const GLchar fragmentShaderSource[] = {
		"#version 150\n"
		"uniform vec4 color;\n"
		"uniform sampler2D texUnit;\n"
//		"uniform sampler2D texUnit2;\n" // test
		"in vec2 uv;\n"
//		"in vec2 uv2;\n" // test
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"    vec4 texel = texture(texUnit, uv);\n"
		"    if (texel.a < 0.5) discard;\n"
//		"    texel = vec4(1,1,1,1) - texture(texUnit2, uv2);\n" // test
		"    outputColor = vec4(texel.rgb * color.rgb, 1);\n"
		"}"
	};

	CreateCommonShader(&fontShader.shader, vertexShaderSource, fragmentShaderSource);

	glUseProgram(fontShader.shader.program);

	fontShader.texcoord = glGetAttribLocation(fontShader.shader.program, "texcoord");
	fontShader.color = glGetUniformLocation(fontShader.shader.program, "color");
	fontShader.sampler = glGetUniformLocation(fontShader.shader.program, "texUnit");
//	fontShader.sampler2 = glGetUniformLocation(fontShader.shader.program, "texUnit2");
}

void UseFontShader(float * matrix, float * vertices, int blend, uint8_t color, GLuint texture, float * texcoord)
{
	UseCommonShader(&fontShader.shader, matrix, vertices);

	uint8_t u8[4];
	*(uint32_t*)u8 = GetColor(color);
	glUniform4f(fontShader.color, u8[0]/255.f, u8[1]/255.f, u8[2]/255.f, u8[3]/255.f);

	glUniform1i(fontShader.sampler, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

//	glUniform1i(fontShader.sampler2, 1);
//	glActiveTexture(GL_TEXTURE1);
//	glBindTexture(GL_TEXTURE_2D, fbo.texture[video_screenofs_fbo]);

	if (blend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	glVertexAttribPointer(fontShader.texcoord, 2, GL_FLOAT, GL_FALSE, 0, texcoord);
	glEnableVertexAttribArray(fontShader.texcoord);
}



GLuint fadeVertexShader;
GLuint fadeFragmentShader;
GLuint fadeProgram;
GLuint fadeTexcoordLocation;
GLuint fadePositionLocation;
GLuint fadeFadeLocation;
GLint fadeSamplerLocation;

void CreateFadeShader()
{
	GLint logLength;

	const GLchar *vertexShaderSource[] = {
		"#version 150\n"
		"in vec2 texcoord;\n"
		"in vec2 position;\n"
		"out vec2 uv;\n"
		"void main()\n"
		"{\n"
		"    gl_Position = vec4(position, 0.0, 1.0);\n"
		"    uv = texcoord;\n"
		"}"
	};

	const GLchar *fragmentShaderSource[] = {
		"#version 150\n"
		"uniform float fade;\n"
		"uniform sampler2D texUnit;\n"
		"in vec2 uv;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"    vec4 color = texture(texUnit, uv);\n"
		"    vec4 darken = color * min(fade,1.0);\n"
		"    vec4 brighten = (darken - vec4(1.0,1.0,1.0,1.0)) * (2 - max(fade,1.0)) + vec4(1.0,1.0,1.0,1.0);\n"
		"    outputColor = brighten;\n"
		"}"
	};

	fadeVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(fadeVertexShader, 1, vertexShaderSource, 0);
	glCompileShader(fadeVertexShader);
	glGetShaderiv(fadeVertexShader, GL_INFO_LOG_LENGTH , &logLength);
	if (logLength > 1)
	{
		GLchar* log = (GLchar*)alloca(logLength);
		glGetShaderInfoLog(fadeVertexShader, logLength, 0, log);
		SDL_Log(log);
		assert(0);
	}
 
	fadeFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fadeFragmentShader, 1, fragmentShaderSource, 0);
	glCompileShader(fadeFragmentShader);
	glGetShaderiv(fadeFragmentShader, GL_INFO_LOG_LENGTH , &logLength);
	if (logLength > 1)
	{
		GLchar* log = (GLchar*)alloca(logLength);
		glGetShaderInfoLog(fadeFragmentShader, logLength, 0, log);
		SDL_Log(log);
		assert(0);
	}

	fadeProgram = glCreateProgram();
	glAttachShader(fadeProgram, fadeVertexShader);
	glAttachShader(fadeProgram, fadeFragmentShader);
	glLinkProgram(fadeProgram);
	glUseProgram(fadeProgram);
 
	fadeTexcoordLocation = glGetAttribLocation(fadeProgram, "texcoord");
	fadePositionLocation = glGetAttribLocation(fadeProgram, "position");
	fadeSamplerLocation = glGetUniformLocation(fadeProgram, "texUnit");
	fadeFadeLocation = glGetUniformLocation(fadeProgram, "fade");
}






int CreateFrameBuffers(framebuffer_t * fbo, int width, int height)
{
	//glGenFramebuffers(NUMFRAMEBUFFERS, fbo->frameBuffer);
	//glGenTextures(NUMFRAMEBUFFERS, fbo->texture);
	//glGenRenderbuffers(NUMFRAMEBUFFERS, fbo->renderBuffer);
	fbo->width = width;
	fbo->height = height;

	for (int i = 0; i < NUMFRAMEBUFFERS; i++)
	{
		glGenFramebuffers(1, &fbo->frameBuffer[i]);
		glGenTextures(1, &fbo->texture[i]);
		glGenRenderbuffers(1, &fbo->renderBuffer[i]);

		// create texture
		glBindTexture(GL_TEXTURE_2D, fbo->texture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
 
		// create render buffer
		glBindRenderbuffer(GL_RENDERBUFFER, fbo->renderBuffer[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

		// attach everything
		glBindFramebuffer(GL_FRAMEBUFFER, fbo->frameBuffer[i]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo->texture[i], 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo->renderBuffer[i]);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			assert(0);
			return 0;
		}
	}

	Matrix4x4_SetScreen(&matProj2D, (float)width,  (float)height);

	return 1;
}




/* Source for the following is
http://commons.wikimedia.org/w/index.php?title=File:EGA_Table.PNG&oldid=39767054
*/
static uint32_t EGAPalette[64] = {
	0xFF000000, 0xFFAA0000, 0xFF00AA00, 0xFFAAAA00, //  0- 3
	0xFF0000AA, 0xFFAA00AA, 0xFF00AAAA, 0xFFAAAAAA, //  4- 7
	0xFF550000, 0xFFFF0000, 0xFF55AA00, 0xFFFFAA00, //  8-11
	0xFF5500AA, 0xFFFF00AA, 0xFF55AAAA, 0xFFFFAAAA, // 12-15

	0xFF005500, 0xFFAA5500, 0xFF00FF00, 0xFFAAFF00, // 16-19
	0xFF0055AA, 0xFFAA55AA, 0xFF00FFAA, 0xFFAAFFAA, // 20-23
	0xFF555500, 0xFFFF5500, 0xFF55FF00, 0xFFFFFF00, // 24-27
	0xFF5555AA, 0xFFFF55AA, 0xFF55FFAA, 0xFFFFFFAA, // 28-31

	0xFF000055, 0xFFAA0055, 0xFF00AA55, 0xFFAAAA55, // 32-35
	0xFF0000FF, 0xFFAA00FF, 0xFF00AAFF, 0xFFAAAAFF, // 36-39
	0xFF550055, 0xFFFF0055, 0xFF55AA55, 0xFFFFAA55, // 40-43
	0xFF5500FF, 0xFFFF00FF, 0xFF55AAFF, 0xFFFFAAFF, // 44-47

	0xFF005555, 0xFFAA5555, 0xFF00FF55, 0xFFAAFF55, // 48-51
	0xFF0055FF, 0xFFAA55FF, 0xFF00FFFF, 0xFFAAFFFF, // 52-55
	0xFF555555, 0xFFFF5555, 0xFF55FF55, 0xFFFFFF55, // 56-59
	0xFF5555FF, 0xFFFF55FF, 0xFF55FFFF, 0xFFFFFFFF, // 60-63
};

static int EGADefaultColors[16] = {0, 1, 2, 3, 4, 5, 20, 7, 56, 57, 58, 59, 60, 61, 62, 63};

unsigned int GetColor(int i)
{
	//uint32_t color = EGAPalette[CatacombColors[3][i&0xf]];
	return EGAPalette[EGADefaultColors[i&0xf]];
}

void IDLIBC_SDL_Init()
{
	SDL_Init(SDL_INIT_EVERYTHING);//SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER);

	int x = 0;
	int y = 0;
	int w = 1024;//960;//1280;
	int h = 768;//720;
	surfaceWidth = w;
	surfaceHeight = h;

	Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

	window = SDL_CreateWindow("Hovertank", x, y, w, h, flags);

	SDL_GLContext context = SDL_GL_CreateContext(window);

#ifdef __GLEW_H__
	glewExperimental = GL_TRUE;
 
	glewInit();
#else
	// shaders
	GLGETPROC(glCreateProgram);
	GLGETPROC(glCreateShader);
	GLGETPROC(glCompileShader);
	GLGETPROC(glGetShaderiv);
	GLGETPROC(glGetShaderInfoLog);
	GLGETPROC(glShaderSource);
	GLGETPROC(glAttachShader);
	GLGETPROC(glLinkProgram);
	GLGETPROC(glGetProgramiv);
	GLGETPROC(glGetProgramInfoLog);
	GLGETPROC(glUseProgram);
	GLGETPROC(glGetUniformLocation);
	GLGETPROC(glGetAttribLocation);
	GLGETPROC(glUniformMatrix4fv);
	GLGETPROC(glVertexAttribPointer);
	GLGETPROC(glEnableVertexAttribArray);
	GLGETPROC(glUniform1f);
	GLGETPROC(glUniform4f);
	GLGETPROC(glUniform1i);
	GLGETPROC(glActiveTexture);

	// frame buffer objects
	GLGETPROC(glGenFramebuffers);
	GLGETPROC(glGenRenderbuffers);
	GLGETPROC(glBindRenderbuffer);
	GLGETPROC(glRenderbufferStorage);
	GLGETPROC(glBindFramebuffer);
	GLGETPROC(glFramebufferTexture);
	GLGETPROC(glFramebufferRenderbuffer);
	GLGETPROC(glCheckFramebufferStatus);
#endif

	if (!CreateFrameBuffers(&fbo, 1024, 256))//SCREENWIDTH*8*2, 200))
	{
		SDL_Log("Error Creating Framebuffer");
		assert(0);
		exit(0);
	}
	CreateColorShader();
	CreateTextureShader();
	CreateFontShader();
	CreateFadeShader();

	glBindFramebuffer(GL_FRAMEBUFFER, fbo.frameBuffer[0]);
	glViewport(0,0,fbo.width,fbo.height);


	SDL_AudioSpec desired;
	desired.freq = SAMPLERATE;
	desired.format = AUDIO_S16;
	desired.channels = 1;
	desired.samples = SAMPLES*2;//1024;
	desired.callback = MyAudioCallback;
	desired.userdata = NULL;
	if (SDL_OpenAudio(&desired, NULL) != 0) {assert(0);}

	SDL_PauseAudio(0);
}



void SetScreenOfs()
{
	if (screenofs == video_oldscreenofs)
	{
		return;
	}
	video_oldscreenofs = screenofs;

	int baseofs;
	GetAddressInfo(screenofs, &baseofs, &video_screenofs_fbo);

	int ofs = screenofs - baseofs;
	video_screenofs_x = (ofs%video_line_width)*8;
	video_screenofs_y = (ofs/video_line_width);
	if (baseofs == video_baseofs)
	{
		return;
	}
	video_baseofs = baseofs;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo.frameBuffer[video_screenofs_fbo]);
	glViewport(0,0,fbo.width,fbo.height);
}

void VideoSetFrameBuffer(int crtc, int pel, video_fbo_t * vfbo)
{
	int baseofs;
	GetAddressInfo(crtc, &baseofs, &vfbo->index);
	int ofs = crtc - baseofs;
	vfbo->xoffset = (ofs*8+pel)%(video_line_width*8);
	vfbo->yoffset = ofs/video_line_width;
}

float video_color_border[4];
void ColorBorder (int color)
{
	*(unsigned int*)video_color_border = GetColor(color);
}

void VideoSync(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0,0,surfaceWidth,surfaceHeight);
	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float xoffset = (float)video_fbo.xoffset;
	float yoffset = (float)video_fbo.yoffset;

	float tsplit = video_split/(float)fbo.height;
	float vsplit = 1 - video_split/100.f;

	float twidth = 320.f/(float)fbo.width;
	float theight = 200.f/(float)fbo.height;

	float u0 = xoffset/(float)fbo.width;
	float u1 = u0 + twidth;
	float v0 = yoffset/(float)fbo.height;
	float v1 = v0 + tsplit;

	float texcoords[] = {
		u0, 1 - v0,
		u1, 1 - v0,
		u1, 1 - v1,
		u0, 1 - v1,
	};

	float vertices[] = {
		-1, 1,
		1, 1,
		1, vsplit,
		-1, vsplit,
	};

	glUseProgram(fadeProgram);

	glUniform1i(fadeSamplerLocation, 0);
	glUniform1f(fadeFadeLocation, video_fade);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fbo.texture[video_fbo.index]);

	glDisable(GL_BLEND);

	glVertexAttribPointer(fadeTexcoordLocation, 2, GL_FLOAT, GL_FALSE, 0, texcoords);
	glEnableVertexAttribArray(fadeTexcoordLocation);
	glVertexAttribPointer(fadePositionLocation, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(fadePositionLocation);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	if (video_split < 200)
	{
		glBindTexture(GL_TEXTURE_2D, fbo.texture[0]);

		texcoords[0] = 0;
		texcoords[1] = 1;

		texcoords[2] = twidth;
		texcoords[3] = 1;

		texcoords[4] = twidth;
		texcoords[5] = 1-(theight-tsplit);

		texcoords[6] = 0;
		texcoords[7] = 1-(theight-tsplit);

		vertices[1] = vsplit;
		vertices[3] = vsplit;
		vertices[5] = -1;
		vertices[7] = -1;
		glVertexAttribPointer(fadePositionLocation, 2, GL_FLOAT, GL_FALSE, 0, vertices);

		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	SDL_GL_SwapWindow( window );

	glBindFramebuffer(GL_FRAMEBUFFER, fbo.frameBuffer[video_screenofs_fbo]);
	glViewport(0,0,fbo.width,fbo.height);
}

void CopyEGA(int wide, int height, int source, int dest)
{
	video_fbo_t srcfbo;
	video_fbo_t dstfbo;
	VideoSetFrameBuffer(source, 0, &srcfbo);
	VideoSetFrameBuffer(dest, 0, &dstfbo);

	assert(srcfbo.index != dstfbo.index);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo.frameBuffer[dstfbo.index]);

	int width = wide * 8;

	float w = width / (float)fbo.width;
	float h = height / (float)fbo.height;

	float u0 = srcfbo.xoffset / (float)fbo.width;
	float v0 = srcfbo.yoffset / (float)fbo.height;
	float u1 = u0+w;
	float v1 = v0+h;

	GLfloat texcoord[] = {
		u0, 1-v0,
		u1, 1-v0,
		u1, 1-v1,
		u0, 1-v1,
	};

	float x0 = dstfbo.xoffset;
	float y0 = dstfbo.yoffset;
	float x1 = x0+width;
	float y1 = y0+height;

	GLfloat vertices[] = {
		x0,	y0, 0,
		x1,	y0, 0,
		x1,	y1, 0,
		x0,	y1, 0,
	};

	UseTextureShader(matProj2D.m, vertices, false, fbo.texture[srcfbo.index], texcoord);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo.frameBuffer[video_screenofs_fbo]);
	glViewport(0,0,fbo.width,fbo.height);
}

void EGASplitScreen (int linenum)	// splits a 200 line screen
{
	video_split = linenum;
}

void SetScreen(int crtc, int pel)
{
	video_crtc = crtc;
	video_pel = pel;
	VideoSetFrameBuffer(crtc, pel, &video_fbo);
	VideoSync();
}

void SetLineWidth (int width)
{
	video_line_width = width;
	linewidth = width;
}



void FadeOut (void)
{
	int i;

	for (i=3;i>=0;i--)
	{
	  video_fade = i/3.f;
	  WaitVBL(6);
	}
}

void FadeIn (void) 
{
	int i;

	for (i=0;i<4;i++)
	{
	  video_fade = i/3.f;
	  WaitVBL(6);
	}
}

void FadeUp (void) 
{
	int i;

	for (i=3;i<6;i++)
	{
	  video_fade = (i-3)/2.f + 1.f;
	  WaitVBL(6);
	}
}

void FadeDown (void) 
{
	int i;

	for (i=5;i>2;i--)
	{
	  video_fade = (i-3)/2.f + 1.f;
	  WaitVBL(6);
	}
}




void DrawPic (int x, int y, int picnum)
{
	gltexture * shape = (gltexture*)grsegs[picnum+STARTPICS];
	assert(shape);

	SetScreenOfs();

	float w = (float)(shape->width);
	float h = (float)(shape->height);
	float tx = (float)(x*8 + video_screenofs_x);
	float ty = (float)(y + video_screenofs_y);

	GLfloat texcoord[] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1,
	};

	GLfloat vertices[] = {
		tx,		ty, 0,
		w+tx,	ty, 0,
		w+tx,	h+ty, 0,
		tx,		h+ty, 0,
	};

	UseTextureShader(matProj2D.m, vertices, false, shape->texture, texcoord);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void DrawShape(int x,int y,unsigned scale, memptr shapeptr)
{
	gltexture *shape =  (gltexture*)shapeptr;

	SetScreenOfs();

	float sx = (float)(shape->width * scale/64.f + video_screenofs_x);
	float sy = (float)(shape->height * scale/64.f + video_screenofs_y);
	float tx = (float)x - sx/2.f;
	float ty = (float)y - sy/2.f;

	GLfloat texcoord[] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1,
	};

	GLfloat vertices[] = {
		tx,		ty, 0,
		sx+tx,	ty, 0,
		sx+tx,	sy+ty, 0,
		tx,		sy+ty, 0,
	};

	UseTextureShader(matProj2D.m, vertices, false, shape->texture, texcoord);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void IN_Poll();

void WaitVBL(int num)
{
	//for (int i = 0; i < num; i++)
	//{
		VideoSync();
	//}
	int starttime = inttime;
	while (inttime-starttime < num*2) {}
	if (processedevents)
	{
		processedevents = 0;
	}
	else
	{
		IN_Poll();
	}
}


#define	sc_None			0
#define	sc_Bad			0xff
#define	sc_Return		0x1c
#define	sc_Enter		sc_Return
#define	sc_Escape		0x01
#define	sc_Space		0x39
#define	sc_BackSpace	0x0e
#define	sc_Tab			0x0f
#define	sc_Alt			0x38
#define	sc_Control		0x1d
#define	sc_CapsLock		0x3a
#define	sc_LShift		0x2a
#define	sc_RShift		0x36
#define	sc_UpArrow		0x48
#define	sc_DownArrow	0x50
#define	sc_LeftArrow	0x4b
#define	sc_RightArrow	0x4d
#define	sc_Insert		0x52
#define	sc_Delete		0x53
#define	sc_Home			0x47
#define	sc_End			0x4f
#define	sc_PgUp			0x49
#define	sc_PgDn			0x51
#define	sc_F1			0x3b
#define	sc_F2			0x3c
#define	sc_F3			0x3d
#define	sc_F4			0x3e
#define	sc_F5			0x3f
#define	sc_F6			0x40
#define	sc_F7			0x41
#define	sc_F8			0x42
#define	sc_F9			0x43
#define	sc_F10			0x44
#define	sc_F11			0x57
#define	sc_F12			0x59

#define	sc_A			0x1e
#define	sc_B			0x30
#define	sc_C			0x2e
#define	sc_D			0x20
#define	sc_E			0x12
#define	sc_F			0x21
#define	sc_G			0x22
#define	sc_H			0x23
#define	sc_I			0x17
#define	sc_J			0x24
#define	sc_K			0x25
#define	sc_L			0x26
#define	sc_M			0x32
#define	sc_N			0x31
#define	sc_O			0x18
#define	sc_P			0x19
#define	sc_Q			0x10
#define	sc_R			0x13
#define	sc_S			0x1f
#define	sc_T			0x14
#define	sc_U			0x16
#define	sc_V			0x2f
#define	sc_W			0x11
#define	sc_X			0x2d
#define	sc_Y			0x15
#define	sc_Z			0x2c

static int translateKey(int Sym) {
	switch (Sym) {
	default: return 0; break;
	case SDLK_RETURN: return sc_Return; break;
	case SDLK_ESCAPE: return sc_Escape; break;
	case SDLK_SPACE: return sc_Space; break;
	case SDLK_BACKSPACE: return sc_BackSpace; break;
	case SDLK_TAB: return sc_Tab; break;
	case SDLK_LALT: return sc_Alt; break;
	case SDLK_RALT: return sc_Alt; break;
	case SDLK_LCTRL: return sc_Control; break;
	case SDLK_RCTRL: return sc_Control; break;
	case SDLK_LSHIFT: return sc_LShift; break;
	case SDLK_RSHIFT: return sc_RShift; break;
	case SDLK_CAPSLOCK: return sc_CapsLock; break;
	
	case SDLK_UP: return sc_UpArrow; break;
	case SDLK_DOWN: return sc_DownArrow; break;
	case SDLK_LEFT: return sc_LeftArrow; break;
	case SDLK_RIGHT: return sc_RightArrow; break;

	case SDLK_PAGEUP: return sc_PgUp; break;
	case SDLK_PAGEDOWN: return sc_PgDn; break;
	case SDLK_INSERT: return sc_Insert; break;
	case SDLK_DELETE: return sc_Delete; break;
	case SDLK_HOME: return sc_Home; break;
	case SDLK_END: return sc_End; break;
	case SDLK_F1: return sc_F1; break;
	case SDLK_F2: return sc_F2; break;
	case SDLK_F3: return sc_F3; break;
	case SDLK_F4: return sc_F4; break;
	case SDLK_F5: return sc_F5; break;
	case SDLK_F6: return sc_F6; break;
	case SDLK_F7: return sc_F7; break;
	case SDLK_F8: return sc_F8; break;
	case SDLK_F9: return sc_F9; break;
	case SDLK_F10: return sc_F10; break;
	case SDLK_F11: return sc_F11; break;
	case SDLK_F12: return sc_F12; break;
	case SDLK_1: return 2; break;
	case SDLK_2: return 3; break;
	case SDLK_3: return 4; break;
	case SDLK_4: return 5; break;
	case SDLK_5: return 6; break;
	case SDLK_6: return 7; break;
	case SDLK_7: return 8; break;
	case SDLK_8: return 9; break;
	case SDLK_9: return 10; break;
	case SDLK_0: return 11; break;
	case SDLK_a: return sc_A; break;
	case SDLK_b: return sc_B; break;
	case SDLK_c: return sc_C; break;
	case SDLK_d: return sc_D; break;
	case SDLK_e: return sc_E; break;
	case SDLK_f: return sc_F; break;
	case SDLK_g: return sc_G; break;
	case SDLK_h: return sc_H; break;
	case SDLK_i: return sc_I; break;
	case SDLK_j: return sc_J; break;
	case SDLK_k: return sc_K; break;
	case SDLK_l: return sc_L; break;
	case SDLK_m: return sc_M; break;
	case SDLK_n: return sc_N; break;
	case SDLK_o: return sc_O; break;
	case SDLK_p: return sc_P; break;
	case SDLK_q: return sc_Q; break;
	case SDLK_r: return sc_R; break;
	case SDLK_s: return sc_S; break;
	case SDLK_t: return sc_T; break;
	case SDLK_u: return sc_U; break;
	case SDLK_v: return sc_V; break;
	case SDLK_w: return sc_W; break;
	case SDLK_x: return sc_X; break;
	case SDLK_y: return sc_Y; break;
	case SDLK_z: return sc_Z; break;
	}
}

void ProcessEvent(SDL_Event * event)
{
	switch(event->type)
	{
	case SDL_QUIT:
		exit(0);
		break;
	case SDL_KEYDOWN:
		NBKscan = translateKey(event->key.keysym.sym);
		keydown[NBKscan] = 1;
		if (event->key.keysym.mod & KMOD_SHIFT) NBKscan |= 0x80;
		if (event->key.keysym.sym >= SDLK_0 && event->key.keysym.sym <= SDLK_9)
		{
			NBKascii = '0' + (event->key.keysym.sym - SDLK_0);
		}
		else if (event->key.keysym.sym >= SDLK_a && event->key.keysym.sym <= SDLK_z)
		{
			if (event->key.keysym.mod & KMOD_SHIFT)
			{
				NBKascii = 'A' + (event->key.keysym.sym - SDLK_a);
			}
			else
			{
				NBKascii = 'a' + (event->key.keysym.sym - SDLK_a);
			}
		}
		else
		{
			switch (event->key.keysym.sym)
			{
			case SDLK_RETURN: NBKascii = '\r'; break;
			case SDLK_BACKSPACE: NBKascii = 8; break;
			case SDLK_DELETE: NBKascii = 127; break;
			case SDLK_SPACE: NBKascii = ' '; break;
			case SDLK_ESCAPE: NBKascii = 27; break;
			}
		}
		break;
	case SDL_KEYUP:
		keydown[translateKey(event->key.keysym.sym)] = 0;
		break;
	}
}

void IN_Poll()
{
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		ProcessEvent(&event);
	}
}

void ClearKeys (void)
{
  NBKscan=NBKascii=0;
  memset (keydown,0,sizeof(keydown));
}


void Ack()
{
	VideoSync();
	while (1)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			ProcessEvent(&event);
			if (event.type == SDL_KEYDOWN)
			{
				return;
			}
		}
	}
}


int NoBiosKey(int x)
{
	processedevents = 1;
	if (x)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			ProcessEvent(&event);
			if (event.type == SDL_KEYDOWN)
			{
				return (NBKscan<<8)|NBKascii;
			}
		}
		return 0;
	}
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		ProcessEvent(&event);
		if (event.type == SDL_KEYUP)
		{
			break;
		}
		if (event.type == SDL_KEYDOWN)
		{
			break;
		}
	}
	return (NBKscan<<8)|NBKascii;
}



////////////////////////
//
// loadctrls
// Tries to load the control panel settings
// creates a default if not present
//
////////////////////////

void LoadCtrls (void)
{
  FILE *handle;

  if (!(handle = fopen("CTLPANEL."EXTENSION, "rb")))
  //
  // set up default control panel settings
  //
  {
    key[0] = 0x48;
    key[1] = 0x49;
    key[2] = 0x4d;
    key[3] = 0x51;
    key[4] = 0x50;
    key[5] = 0x4f;
    key[6] = 0x4b;
    key[7] = 0x47;
    keyB1 = 0x1d;
    keyB2 = 0x38;
  }
  else
  {
    fread(&key, 1, sizeof(key), handle);
	fread(&keyB1, 1, sizeof(keyB1), handle);
	fread(&keyB2, 1, sizeof(keyB2), handle);
	fread(&highscore, 1, sizeof(highscore), handle);
	fread(&bestlevel, 1, sizeof(bestlevel), handle);
	fclose(handle);
  }
}

void SaveCtrls (void)
{
  FILE * handle;

  if (!(handle = fopen("CTLPANEL."EXTENSION, "wb")))
	return;

  fwrite(&key, 1, sizeof(key), handle);
  fwrite(&keyB1, 1, sizeof(keyB1), handle);
  fwrite(&keyB2, 1, sizeof(keyB2), handle);
  fwrite(&highscore, 1, sizeof(highscore), handle);
  fwrite(&bestlevel, 1, sizeof(bestlevel), handle);

  fclose(handle);
}


ControlStruct ControlPlayer (int player)
{
	int xmove=0,
		ymove=0;
	ControlStruct action;

	IN_Poll();

	SDLK_LCTRL;
	SDL_SCANCODE_UP;
    //SDL_SCANCODE_RIGHT = 79,
    //SDL_SCANCODE_LEFT = 80,
    //SDL_SCANCODE_DOWN = 81,
    //SDL_SCANCODE_UP = 82,

    //key[0] = 72  0x48;
    //key[1] = 73  0x49;
    //key[2] = 77  0x4d;
    //key[3] = 81  0x51;
    //key[4] = 80  0x50;
    //key[5] = 79  0x4f;
    //key[6] = 75  0x4b;
    //key[7] = 71  0x47;
    //keyB1 = 29  0x1d;
    //keyB2 = 56  0x38;

	if (keydown [key[north]])
	ymove=-1;
	if (keydown [key[east]])
	xmove=1;
	if (keydown [key[south]])
	ymove=1;
	if (keydown [key[west]])
	xmove=-1;

	if (keydown [key[northeast]])
	{
	ymove=-1;
	xmove=1;
	}
	if (keydown [key[northwest]])
	{
	ymove=-1;
	xmove=-1;
	}
	if (keydown [key[southeast]])
	{
	ymove=1;
	xmove=1;
	}
	if (keydown [key[southwest]])
	{
	ymove=1;
	xmove=-1;
	}

	switch (ymove*3+xmove)
	{
	case -4: action.dir = northwest; break;
	case -3: action.dir = north; break;
	case -2: action.dir = northeast; break;
	case -1: action.dir = west; break;
	case  0: action.dir = nodir; break;
	case  1: action.dir = east; break;
	case  2: action.dir = southwest; break;
	case  3: action.dir = south; break;
	case  4: action.dir = southeast; break;
	}

	action.button1 = keydown [keyB1];
	action.button2 = keydown [keyB2];

	return (action);
}


/*
============================================================================

			MID LEVEL GRAPHIC ROUTINES

============================================================================
*/


int win_xl,win_yl,win_xh,win_yh;

int sx,sy,leftedge;

int screencenterx = 20,screencentery = 11;


//////////////////////////
//
// DrawWindow
// draws a bordered window and homes the cursor
//
//////////////////////////

void DrawWindow (int xl, int yl, int xh, int yh)
{
 int x,y;
 win_xl=xl;
 pxl = xl*8+8;
 win_yl=yl;
 win_xh=xh;
 pxh = xh*8;
 win_yh=yh;		// so the window can be erased

 DRAWCHAR (xl,yl,1);
 for (x=xl+1;x<xh;x++)
   DRAWCHAR (x,yl,2);
 DRAWCHAR (xh,yl,3);
 for (y=yl+1;y<yh;y++)
 {
   DRAWCHAR (xl,y,4);
   for (x=xl+1;x<xh;x++)
     DRAWCHAR (x,y,9);
   DRAWCHAR (xh,y,5);
 }
 DRAWCHAR (xl,yh,6);
 for (x=xl+1;x<xh;x++)
   DRAWCHAR (x,yh,7);
 DRAWCHAR (xh,yh,8);

 sx = leftedge = xl+1;
 sy = yl+1;
 px=sx*8;
 py=pyl=sy*8;
}

void EraseWindow (void)
{
 int x,y;

 for (y=win_yl+1;y<win_yh;y++)
   for (x=win_xl+1;x<win_xh;x++)
     DRAWCHAR (x,y,9);

 sx = leftedge = win_xl+1;
 sy = win_yl+1;
 px=sx*8;
 py=pyl=sy*8;
}

/////////////////////////////
//
// CenterWindow
// Centers a DrawWindow of the given size
//
/////////////////////////////

void CenterWindow (int width, int height)
{
  int xl = screencenterx-width/2;
  int yl = screencentery-height/2;

  DrawWindow (xl,yl,xl+width+1,yl+height+1);
}

///////////////////////////////
//
// ExpWin {h / v}
// Grows the window outward
//
///////////////////////////////
void ExpWin (int width, int height)
{
  if (width > 2)
  {
    if (height >2)
      ExpWin (width-2,height-2);
    else
      ExpWinH (width-2,height);
  }
  else
    if (height >2)
      ExpWinV (width,height-2);

  WaitVBL (1);
  CenterWindow (width,height);
}

void ExpWinH (int width, int height)
{
  if (width > 2)
    ExpWinH (width-2,height);

  WaitVBL (1);
  CenterWindow (width,height);
}

void ExpWinV (int width, int height)
{
  if (height >2)
    ExpWinV (width,height-2);

  WaitVBL (1);
  CenterWindow (width,height);
}

/*
===========================================================================

		 CHARACTER BASED PRINTING ROUTINES

===========================================================================
*/


/////////////////////////
//
// Print
// Prints a string at sx,sy.  No clipping!!!
//
/////////////////////////

void Print (const char *str)
{
  unsigned char ch;

  while ((ch=*str++) != 0)
    if (ch == '\n')
    {
      sy++;
      sx=leftedge;
    }
    else if (ch == '\r')
      sx=leftedge;
    else
      DRAWCHAR (sx++,sy,ch);
}






////////////////////////////////////////////////////////////////////
//
// Input unsigned
//
////////////////////////////////////////////////////////////////////
unsigned InputInt(void)
{
 char string[18]="",digit,hexstr[17]="0123456789ABCDEF";
 unsigned value,loop,loop1;

 Input(string,2);
 if (string[0]=='$')
   {
    int digits;

    digits=strlen(string)-2;
    if (digits<0) return 0;

    for (value=0,loop1=0;loop1<=digits;loop1++)
      {
       digit=toupper(string[loop1+1]);
       for (loop=0;loop<16;loop++)
	  if (digit==hexstr[loop])
	    {
	     value|=(loop<<(digits-loop1)*4);
	     break;
	    }
      }
   }
 else if (string[0]=='%')
   {
    int digits;

    digits=strlen(string)-2;
    if (digits<0) return 0;

    for (value=0,loop1=0;loop1<=digits;loop1++)
      {
       if (string[loop1+1]<'0' || string[loop1+1]>'1') return 0;
       value|=(string[loop1+1]-'0')<<(digits-loop1);
      }
   }
 else value=atoi(string);
 return value;
}




////////////////////////////////////////////////////////////////////
//
// line Input routine (PROPORTIONAL)
//
////////////////////////////////////////////////////////////////////
int Input(char *string,int max)
{
 char key;
 int count=0,loop;
 int pxt[90];
 pxt[0]=px;

 do {
     key=toupper(PGet()&0xff);
     if ((key==127 || key==8)&&count>0)
       {
	count--;
	px=pxt[count];
	//int oldfontcolor = fontcolor;
	//fontcolor = 0xf; // HACK to erase
	DrawPchar(string[count]);
	//fontcolor = oldfontcolor;
	px=pxt[count];
       }

     if (key>=' ' && key<='z' && count<max)
       {
	*(string+count++)=key;
	DrawPchar(key);
	pxt[count]=px;
       }

    } while (key!=27 && key!=13);

 for (loop=count;loop<max;loop++) *(string+loop)=0;

 if (key==13) return 1;
 return 0;
}

/*
===========================================================================

		 PROPORTIONAL PRINTING ROUTINES

===========================================================================
*/
unsigned px,py;
unsigned pxl,pxh,pyl,pyh;
unsigned fontcolor = 15;
unsigned pdrawmode = 0x18;


void DrawPchar (int Char)
{
	font_t *font = (font_t*)grsegs[STARTFONT];
	int width = font->width[Char];
	float total_width = (float)font->total_width;

	SetScreenOfs();

	float u0 = font->location[Char] / total_width;
	float u1 = width / total_width + u0;
	GLfloat texcoord[] = {
		u0, 0,
		u1, 0,
		u1, 1,
		u0, 1,
	};

	float x0 = (float)(px + video_screenofs_x);
	float y0 = (float)(py + video_screenofs_y);
	float x1 = x0 + width;
	float y1 = y0 + font->height;
	GLfloat vertices[] = {
		x0, y0, 0,
		x1, y0, 0,
		x1, y1, 0,
		x0, y1, 0,
	};

#if 0
	UseFontShader(matProj2D.m, vertices, false, fontcolor, font->texture, texcoord);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
#else
	UseFontShader(matProj2D.m, vertices, false, fontcolor, font->texture, texcoord);

	if (pdrawmode == 0x18)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
	}

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisable(GL_BLEND);
#endif

	px+=width;
}




/////////////////////////
//
// PPrint
// Prints a string at px,py.  No clipping!!!
//
/////////////////////////

void PPrint (const char *str)
{
  unsigned char ch;

  while ((ch=*str++) != 0)
    if (ch == '\n')
    {
      py+=10;
      px=pxl;
    }
    else if (ch == '')
      fontcolor=(*str++)-'A';	// set color A-P
    else
      DrawPchar (ch);
}



/////////////////////////
//
// PGet
// Flash a cursor at px,py and waits for a user NoBiosKey
//
/////////////////////////

int PGet (void)
{
 int oldx;
 
 oldx=px;

 ClearKeys();
 while (!(NoBiosKey(1)&0xff))
 {
   DrawPchar ('_');
   WaitVBL (5);
   px=oldx;
   //int oldfontcolor = fontcolor;
   //fontcolor = 0xf; // HACK to erase
   DrawPchar ('_');
   //fontcolor = oldfontcolor;
   px=oldx;
   if (NoBiosKey(1)&0xff)		// slight response improver
     break;
   WaitVBL (5);
 }
 px=oldx;
 return NoBiosKey(0);		// take it out of the buffer
}


/////////////////////////
//
// PSize
// Return the pixels required to proportionaly print a string
//
/////////////////////////

int PSize (const char *str)
{
  int length=0;
  unsigned char ch;

  while ((ch=*str++) != 0)
  {
    if (ch=='')	// skip color changes
    {
      str++;
      continue;
    }
    length += ((font_t *)grsegs[STARTFONT])->width[ch];
  }

  return length;
}


/////////////////////////
//
// CPPrint
// Centers the string between pxl/pxh
//
/////////////////////////

void CPPrint (const char *str)
{
  int width;

  width = PSize(str);
  px=pxl+(int)(pxh-pxl-width)/2;
  PPrint (str);
}


void PPrintUnsigned (unsigned val)
{
  char str[512];
  ltoa((long)val,str,10);
  PPrint (str);
}

void PPrintInt (int val)
{
  char str[512];
  itoa(val,str,10);
  PPrint (str);
}



void DrawChar (int x, int y, int charnum)
{
	memptr * tiles = (memptr*)grsegs[STARTTILE8];
	assert(charnum >= 0 && charnum < NUMTILE8);
	gltexture * shape = (gltexture*)tiles[charnum];
	assert(shape);

	SetScreenOfs();

	float w = (float)(shape->width);
	float h = (float)(shape->height);
	float tx = (float)(x*8 + video_screenofs_x);
	float ty = (float)(y + video_screenofs_y);

	GLfloat texcoord[] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1,
	};

	GLfloat vertices[] = {
		tx,		ty, 0,
		w+tx,	ty, 0,
		w+tx,	h+ty, 0,
		tx,		h+ty, 0,
	};

	UseTextureShader(matProj2D.m, vertices, false, shape->texture, texcoord);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}


void XPlot (int x, int y, int color)
{
	if (color == 0)
	{
		return;
	}

	SetScreenOfs();

	float x0 = (float)(x + video_screenofs_x);
	float y0 = (float)(y + video_screenofs_y);
	float x1 = x0 + 1.f;
	float y1 = y0 + 1.f;

	GLfloat vertices[] = {
		x0,	y0, 0,
		x1,	y0, 0,
		x1,	y1, 0,
		x0,	y1, 0,
	};

#if 0
	UseColorShader(matProj2D.m, vertices, color);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
#else
	UseColorShader(matProj2D.m, vertices, 0xf);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisable(GL_BLEND);
#endif
}



void Block (int x, int y, int color)
{
	SetScreenOfs();

	float x0 = (float)(x*8 + video_screenofs_x);
	float y0 = (float)(y*8 + video_screenofs_y);
	float x1 = x0 + 8.f;
	float y1 = y0 + 8.f;

	GLfloat vertices[] = {
		x0,	y0, 0,
		x1,	y0, 0,
		x1,	y1, 0,
		x0,	y1, 0,
	};

	UseColorShader(matProj2D.m, vertices, color);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}


void DrawLine (int xl, int xh, int y,int color)
{
	SetScreenOfs();

	float x0 = (float)(xl + video_screenofs_x);
	float y0 = (float)(y + video_screenofs_y);
	float x1 = (float)(xh + 1 + video_screenofs_x);
	float y1 = y0 + 1.f;

	GLfloat vertices[] = {
		x0,	y0, 0,
		x1,	y0, 0,
		x1,	y1, 0,
		x0,	y1, 0,
	};

	UseColorShader(matProj2D.m, vertices, color);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void DrawRect (int x, int y, int width, int height, int color)
{
	SetScreenOfs();

	float x0 = (float)(x + video_screenofs_x);
	float y0 = (float)(y + video_screenofs_y);
	float x1 = x0 + (float)width;
	float y1 = y0 + (float)height;

	GLfloat vertices[] = {
		x0,	y0, 0,
		x1,	y0, 0,
		x1,	y1, 0,
		x0,	y1, 0,
	};

	UseColorShader(matProj2D.m, vertices, color);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void DrawLineZ (int xl, int xh, int y, int zl, int zh,int color)
{
	SetScreenOfs();

	float x0 = (float)(xl + video_screenofs_x);
	float y0 = (float)(y + video_screenofs_y);
	float x1 = (float)(xh + 1 + video_screenofs_x);
	float y1 = y0 + 1.f;

	// calculated 1/50th of the reciprical of scale
	const float rscale = (1.4 * 320.f) * 0.02f;
	// inverse for opengl negative z
	float z0 = rscale / (float)zl - 1.f;
	float z1 = rscale / (float)zh - 1.f;

	GLfloat vertices[] = {
		x0,	y0, z0,
		x1,	y0, z1,
		x1,	y1, z1,
		x0,	y1, z0,
	};

	UseColorShader(matProj2D.m, vertices, color);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

}

float z_viewangle;

void EnableZ(int x, int y, int width, int height, float viewx, float viewz, float viewangle)
{
	SetScreenOfs();
	glViewport(x, fbo.height - height - y, width, height);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	static float fov = 40.f*3.141593f/180.f;
	static float n = 1.f;
	static float f = 40.f;

	float aspect = width/(float)height;
	float Hfov = fov*0.5f;
	float Vfov = Hfov/aspect;
	Matrix4x4_SetProject(&matProj,Vfov,Vfov,Hfov,Hfov,n,f);

	z_viewangle = (viewangle-90) * 3.141593f / 180.f;

//	Matrix4x4_SetRotate(&matProj, 0, 1, 0, z_viewangle);
	Matrix4x4_FrontRotate(&matProj, 0, 1, 0, z_viewangle);
	Matrix4x4_FrontTranslate(&matProj, -viewx, 0, -viewz);
}

void DisableZ()
{
	glDisable(GL_DEPTH_TEST);
	glViewport(0,0,fbo.width,fbo.height);
}

void DrawPlaneZ(float width, float length, float y, int color)
{
	SetScreenOfs();

	GLfloat vertices[] = {
		0,	y, 0,
		width,	y, 0,
		width, y, length,
		0, y, length,
	};

	UseColorShader(matProj.m, vertices, color);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void DrawWallZ(float x0, float z0, float x1, float z1, int color)
{
	SetScreenOfs();

	GLfloat vertices[] = {
		x0,	0.5, z0,
		x1,	0.5, z1,
		x1,-0.5, z1,
		x0,-0.5, z0,
	};

	UseColorShader(matProj.m, vertices, color);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void DrawShapeZ(float x, float z, memptr shapeseg)
{
	gltexture * shape = (gltexture*)shapeseg;
	assert(shape);

	SetScreenOfs();

	GLfloat texcoord[] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1,
	};

	float hw = 0.5f * shape->width / (float)shape->height;
	float rx = cosf(-z_viewangle) * hw;
	float rz = sinf(-z_viewangle) * hw;
	float x0 = x - rx;
	float z0 = z - rz;
	float x1 = x + rx;
	float z1 = z + rz;

	GLfloat vertices[] = {
		x0,	0.5, z0,
		x1,	0.5, z1,
		x1,-0.5, z1,
		x0,-0.5, z0,
	};

	UseTextureShader(matProj.m, vertices, false, shape->texture, texcoord);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}




int *UpdateIntTime()
{
	static int lastTime = 0;
	static int _inttime = 0;
	int newTime = SDL_GetTicks()*140/1000;
	_inttime += (newTime - lastTime);
	lastTime = newTime;
	return &_inttime;
}


int SPKfreq;

void _sound(int _frequency)
{
	SPKfreq = _frequency;
}

void _nosound()
{
	SPKfreq = 0;
}

void sound(int _frequency)
{
	SDL_LockAudio();
	_sound(_frequency);
	SDL_UnlockAudio();
}

void nosound(void)
{
	SDL_LockAudio();
	_sound(0);
	SDL_UnlockAudio();
}

int dontplay = 0;
int soundmode = 1; // 0=nosound, 1=SPKR, 2= adlib...
int soundblaster = 1;
byte SndPriority;
short * SndPtr;

void _PlaySound(int soundnum)
{
	spksndtype * sound = ((spksndtype*)soundseg) + soundnum;

	if (dontplay || soundmode == 0) return;
	if (sound->priority < SndPriority) return;

	SndPriority = sound->priority;

	if (soundblaster)
	{
		jmPlaySample(soundnum);
		return;
	}

	SndPtr = (short*)(((char*)soundseg) + sound->start);
}

void PlaySound(int soundnum)
{
	SDL_LockAudio();
	_PlaySound(soundnum);
	SDL_UnlockAudio();
}

void _StopSound(void)
{
	if (dontplay) return;

	SndPriority = 0;

	if (soundblaster)
	{
		jmStopSample();
		return;
	}
	
	SndPtr = 0;
	_nosound();
}

void StopSound(void)
{
	SDL_LockAudio();
	StopSound();
	SDL_UnlockAudio();
}

int SoundPlaying (void)
{
  if (soundblaster)
    return jmSamplePlaying();
  else
	return SndPtr != 0;
}

void UpdateSPKR()
{
	if (!SndPtr)
	{
		_nosound();
		return;
	}
	short freq = *SndPtr;
	SndPtr += 1;
	if (freq == 0)
	{
		_nosound();
		return;
	}
	if (freq == -1)
	{
		_StopSound();
		return;
	}
	if (soundmode == 0)
	{
		_nosound();
		return;
	}
	_sound(freq);
}

float SPKangle;
float SPKtime; // time in seconds

static void MyAudioCallback(void *userdata, Uint8 *stream, int len)
{
	assert(len == SAMPLES*2*2);

	memset(stream, 0, SAMPLES*2*2);

	if (soundmode != 1)
	{
		return;
	}

	short * stream16 = (short*)stream;

	if (soundblaster && jmData)
	{
		for (int i = 0; i < SAMPLES*2; i++)
		{
#if 0
			int offset = (int)(jmDataTime*jmDataFreq);
			if (offset >= jmDataLength)
			{
				jmStopSample();
				break;
			}
			int sample = (((int)jmData[offset]) - 127)*256;
#else
			float foffset = jmDataTime*jmDataFreq;
			int offset = foffset;
			if (offset+1 >= jmDataLength)
			{
				jmStopSample();
				break;
			}
			float finterp = fmod(foffset, 1.f)*256.f;
			int sample1 = ((float)jmData[offset]) - 128.f;
			int sample2 = ((float)jmData[offset+1]) - 128.f;
			int sample = (int)(sample1*(256.f-finterp) + sample2*finterp);
#endif
			*stream16 = (short)sample;
			jmDataTime += SAMPLESTEP;
			stream16++;
		}
	}

	for (int i = 0; i < 2; i++)
	{
		if (SndPtr)
		{
			UpdateSPKR();
		}
		if (SPKfreq)
		{
			for (int i = 0; i < SAMPLES; i++)
			{
				int sample = (int)(sinf(SPKangle)*32767);
				if (sample > 0) sample = 32767;
				else sample = -32767;
				sample = sample / 8 + (int)(*stream16);
				if (sample > 32767) sample = 32767;
				if (sample < -32767) sample = -32767;
				*stream16 += sample;
				SPKangle += SAMPLESTEP*SPKfreq*2.f*3.14156f;
				stream16++;
			}
		}
		else
		{
			stream16+=SAMPLES;
		}
	}
}
