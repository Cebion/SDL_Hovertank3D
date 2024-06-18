
#include "IDLIB.H"
#include "IDLIB_SDL.H"

void glActiveTexture(int x);

#define glActiveTexture _glActiveTexture 

// to use macro convenience
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

// use the convenient macros
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
_GLDEFINE(glActiveTexture);

typedef struct
{
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint program;
	GLuint matrix;
	GLuint position;
} common_shader_t;

int CreateCommonShader(common_shader_t * shader, const GLchar * vertexShaderSource, const GLchar * fragmentShaderSource);
void UseCommonShader(const common_shader_t * shader, const float * matrix, const float * vertices);

void CreateColorShader();
void CreateTextureShader();
void CreateFontShader();
void CreateFadeShader();





void SHADERS_Init(void)
{
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
	_GLGETPROC(glActiveTexture);

	CreateColorShader();
	CreateTextureShader();
	CreateFontShader();
	CreateFadeShader();
}



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
		"#version 320 es\n"
		"precision mediump float;\n"
		"uniform mat4 matrix;\n"
		"in vec3 position;\n"
		"void main()\n"
		"{\n"
		"    gl_Position = matrix * vec4(position, 1.0);\n"
		"}"
	};



	const GLchar fragmentShaderSource[] = {
		"#version 320 es\n"
		"precision mediump float;\n"
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

void UseColorShader(float *matrix, float *vertices, int use_xor, uint8_t color)
{
	UseCommonShader(&colorShader.shader, matrix, vertices);

	uint8_t u8[4];
	*(uint32_t*)u8 = GetColor(color);
	if (use_xor)
	{
		u8[0] = (u8[0] > 127)? 255 : 0;
		u8[1] = (u8[1] > 127)? 255 : 0;
		u8[2] = (u8[2] > 127)? 255 : 0;
	}
	glUniform4f(colorShader.color, u8[0]/255.f, u8[1]/255.f, u8[2]/255.f, u8[3]/255.f);

	if (use_xor)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}




struct {
	common_shader_t shader;
	GLuint texcoord;
	GLint sampler;
} textureShader;

void CreateTextureShader()
{
	const GLchar vertexShaderSource[] = {
		"#version 320 es\n"
		"precision mediump float;\n"
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
		"#version 320 es\n"
		"precision mediump float;\n"
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
		"#version 320 es\n"
		"precision mediump float;\n"
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
		"#version 320 es\n"
		"precision mediump float;\n"
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
		"#version 320 es\n"
		"precision mediump float;\n"
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
		"#version 320 es\n"
		"precision mediump float;\n"
		"uniform float fade;\n"
		"uniform sampler2D texUnit;\n"
		"in vec2 uv;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"    vec4 color = texture(texUnit, uv);\n"
		"    vec4 darken = color * min(fade, 1.0);\n"
		"    vec4 brighten = (darken - vec4(1.0, 1.0, 1.0, 1.0)) * (2.0 - max(fade, 1.0)) + vec4(1.0, 1.0, 1.0, 1.0);\n"
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

void UseFadeShader(float fade, GLuint texture, float * vertices, float * texcoords)
{
	glUseProgram(fadeProgram);

	glUniform1i(fadeSamplerLocation, 0);
	glUniform1f(fadeFadeLocation, fade);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glDisable(GL_BLEND);

	glVertexAttribPointer(fadeTexcoordLocation, 2, GL_FLOAT, GL_FALSE, 0, texcoords);
	glEnableVertexAttribArray(fadeTexcoordLocation);
	glVertexAttribPointer(fadePositionLocation, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(fadePositionLocation);
}

