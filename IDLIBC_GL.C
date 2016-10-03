
#include "IDLIB.H"
#include "IDLIB_SDL.H"

// to use macro convenience
#define PFNglGenFramebuffersPROC PFNGLGENFRAMEBUFFERSPROC
#define PFNglGenRenderbuffersPROC PFNGLGENRENDERBUFFERSPROC
#define PFNglBindRenderbufferPROC PFNGLBINDRENDERBUFFERPROC
#define PFNglRenderbufferStoragePROC PFNGLRENDERBUFFERSTORAGEPROC
#define PFNglBindFramebufferPROC PFNGLBINDFRAMEBUFFERPROC
#define PFNglFramebufferTexturePROC PFNGLFRAMEBUFFERTEXTUREPROC
#define PFNglFramebufferRenderbufferPROC PFNGLFRAMEBUFFERRENDERBUFFERPROC
#define PFNglCheckFramebufferStatusPROC PFNGLCHECKFRAMEBUFFERSTATUSPROC

// use the convenient macros
GLDEFINE(glGenFramebuffers);
GLDEFINE(glGenRenderbuffers);
GLDEFINE(glBindRenderbuffer);
GLDEFINE(glRenderbufferStorage);
GLDEFINE(glBindFramebuffer);
GLDEFINE(glFramebufferTexture);
GLDEFINE(glFramebufferRenderbuffer);
GLDEFINE(glCheckFramebufferStatus);

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
	return EGAPalette[EGADefaultColors[i&0xf]];
}

void IDLIBC_GL_Init()
{
	SDL_GLContext context = SDL_GL_CreateContext(window);

	SHADERS_Init();

	GLGETPROC(glGenFramebuffers);
	GLGETPROC(glGenRenderbuffers);
	GLGETPROC(glBindRenderbuffer);
	GLGETPROC(glRenderbufferStorage);
	GLGETPROC(glBindFramebuffer);
	GLGETPROC(glFramebufferTexture);
	GLGETPROC(glFramebufferRenderbuffer);
	GLGETPROC(glCheckFramebufferStatus);

	if (!CreateFrameBuffers(&fbo, 1024, 256))//SCREENWIDTH*8*2, 200))
	{
		SDL_Log("Error Creating Framebuffer");
		assert(0);
		exit(0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, fbo.frameBuffer[0]);
	glViewport(0,0,fbo.width,fbo.height);
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

float video_color_border[3];
void ColorBorder (int color)
{
	char border[4];
	*(unsigned int*)border = GetColor(color);
	video_color_border[0] = border[0]/256.f;
	video_color_border[1] = border[1]/256.f;
	video_color_border[2] = border[2]/256.f;
}

void VideoSync(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(viewportX,viewportY,viewportWidth,viewportHeight);
	glClearColor(
		video_color_border[0],
		video_color_border[1],
		video_color_border[2],
		1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(viewportX,viewportY,viewportWidth,viewportHeight);

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

	UseFadeShader(video_fade, fbo.texture[video_fbo.index], vertices, texcoords);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	if (video_split < 200)
	{
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

		UseFadeShader(video_fade, fbo.texture[0], vertices, texcoords);

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

void DrawScaledShape(int sxl, int yl, int sxh, int yh, int shapexl, int shapexh, memptr shapeptr)
{
	gltexture *shape =  (gltexture*)shapeptr;

	SetScreenOfs();

	float u0 = shapexl / (float)shape->width;
	float u1 = shapexh / (float)shape->width;

	GLfloat texcoord[] = {
		u0, 0,
		u1, 0,
		u1, 1,
		u0, 1,
	};

	float x0 = (float)(sxl + video_screenofs_x);
	float y0 = (float)(yl + video_screenofs_y);
	float x1 = (float)(sxh + video_screenofs_x);
	float y1 = (float)(yh + video_screenofs_y);

	GLfloat vertices[] = {
		x0, y0, 0,
		x1, y0, 0,
		x1, y1, 0,
		x0, y1, 0,
	};

	UseTextureShader(matProj2D.m, vertices, false, shape->texture, texcoord);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
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

	UseFontShader(matProj2D.m, vertices, false, fontcolor, font->texture, texcoord);

	if (pdrawmode == 0x18)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
	}

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisable(GL_BLEND);

	px+=width;
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

	UseColorShader(matProj2D.m, vertices, 0xf);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisable(GL_BLEND);
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

void Bar (int x, int y, int wide, int height, int color)
{
	SetScreenOfs();

	float x0 = (float)(x*8 + video_screenofs_x);
	float y0 = (float)(y + video_screenofs_y);
	float x1 = x0 + (float)(wide*8);
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
