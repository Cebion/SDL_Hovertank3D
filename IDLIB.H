/* Hovertank 3-D Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#define EXTENSION	"HOV"

#include "GRAPHHOV.H"
#include "SNDSHOV.H"

#ifndef __jm__
typedef unsigned short boolean;
typedef unsigned char byte;
#endif

#define SIGN(x) ((x)>0?1:-1)
#define ABS(x) ((int)(x)>0?(x):-(x))
#define LABS(x) ((long)(x)>0?(x):-(x))




int extern level,bestlevel;

/*
============================================================================

			MEMORY MANAGER

============================================================================
*/

#include "MEMMGR.H"

/*
============================================================================

** Sound routines
** Ties into INT 8, with a timer tic at 8 * normal rate (144/sec)

============================================================================
*/

//typedef enum {off,spkr,adlib} soundtype;

#pragma pack(push, 1)
typedef struct {unsigned short start;
		byte priority;
		byte samplerate;
		char name[12];} spksndtype;
#pragma pack(pop)

#if 0
typedef struct {char id[4];
		unsigned filelength;
		unsigned filler[5];
		spksndtype sounds[63];
		unsigned freqdata[];} SPKRtable;
#endif


int extern soundmode;
int extern soundblaster;

//extern	unsigned timerspeed;
//extern	int	dontplay;
//extern	unsigned inttime;
//extern	long	timecount;

//extern	unsigned int8hook;	// address of function to call every tic

extern	memptr	soundseg;
//extern	unsigned sndptr;

//void StartupSound (void);
//void CallTimer (void);
//void ShutdownSound (void);
void PlaySound (int sound);
//void PauseSound (void);
//void ContinueSound (void);
//void StopSound (void);
//void WaitEndSound (void);


/*
============================================================================

** Control routines
** Ties into INT 9 to intercept all key presses, but passes on to BIOS
** The control panel handles all this stuff!

============================================================================
*/

typedef enum {north,northeast,east,southeast,south,southwest,west,
	      northwest,nodir} dirtype;

typedef struct {dirtype dir;
		boolean button1,button2;} ControlStruct;

extern char keydown[128];
void LoadCtrls (void);
void SaveCtrls (void);
ControlStruct ControlPlayer (int player);

void ClearKeys (void);
void Ack(void);

//int NoBiosKey(int parm);
extern int	NBKscan, NBKascii;

/*
===========================================================================

** Miscellaneous library routines

===========================================================================
*/

typedef struct
{
  unsigned short bit0,bit1;	// 0-255 is a character, > is a pointer to a node
} huffnode;

#define StopDrive() {} //void StopDrive (void);

void BloadinMM (char *filename,memptr *spot);

void OptimizeNodes (huffnode *table);

void HuffExpand (unsigned char *source, unsigned char *dest,
  long length,huffnode *hufftable);

void RLEWExpand (unsigned short *source, unsigned short *dest);

//void RLEBExpand (unsigned char far *source, unsigned char far *dest);

//void RLEExpand (char far *surce,char far *dest,long length);
//unsigned RLECompress (char far *source, unsigned Length, char far *dest);

void InitRnd (boolean randomize);				/* ASM */
int Rnd (int max);                                              /* ASM */
void InitRndT (boolean randomize);                              /* ASM */
int RndT (void);                                                /* ASM */

/*
============================================================================

** Graphic routines
** Edge graphic file not needed
**
** Many of these #defines are duplicates as EQUs in EDGEASM, and must be ==

============================================================================
*/

#define SCREENWIDTH	40

//void SetScreenMode (grtype mode);	// sets graphic mode
void WaitVBL (int num);			// waits for no sync, then sync

//void LoadPage(char *filename,unsigned dest);

//void EGAPlane (int plane);		// read / write from plane 0-4
void EGASplitScreen (int linenum);	// splits a 200 line screen
//void CRTCstart (unsigned start);	// set crtc high/low registers
//void EGAVirtualScreen (int width);	// sets screen width
void ColorBorder (int color);		// sets overscan color

//cardtype VideoID (void);		// returns the display adapter installed

//void SetDefaultColors(void);		// full bright palete
void FadeOut (void);			// EGA 16 color palette fade
void FadeIn (void);
void FadeUp (void);
void FadeDown (void);

//void XorBar(int xl,int yl,int width,int height);
//void Bar (int xl,int yl,int width,int height,int fill);

/*
============================================================================

		    IGRAB Graphic file routines

	 Based on number reported in GRAPHEXT.H header file!

============================================================================
*/

typedef struct
{
 short width,height;
} pictype;

extern unsigned screenofs;		// adjustment for panning and buffers

unsigned extern linewidth;

//
// cachable locations
//

extern memptr	grsegs[NUMCHUNKS];
extern char	needgr[NUMCHUNKS];	// for caching

#if NUMPICS>0
extern pictype	pictable[NUMPICS];
#endif

//
// proportional font stuff
//
#if NUMFONT+NUMFONTM>0
extern unsigned fontcolor,pdrawmode;
extern unsigned px,py;
extern unsigned pxl,pxh,pyl,pyh;
//extern fontstruct * fontseg;

void DrawPchar (int Char);
void DrawMPchar (int Char);
#endif

void SetScreen(int crtc, int pel);

//
// base drawing routines, x in bytes, y in lines
//

void XPlot (int x, int y, int color);
void Block (int x, int y, int color);
void DrawLine (int xl, int xh, int y,int color);
void DrawRect (int x, int y, int width, int height, int color);
void DrawLineZ (int xl, int xh, int y, int zl, int zh,int color);
void DrawChar (int x, int y, int picnum);
//void MaskChar (int x, int y, int picnum);
//void CopyChar (int x, int y);
void DrawPic (int x, int y, int picnum);
void DrawShape(int x,int y,unsigned scale, memptr shapeptr);
void SetLineWidth (int width);
//void DrawSprite (int xcoord, int ycoord, int spritenum);
//void DrawSpriteT (unsigned wide,unsigned height,unsigned source,
//  unsigned dest, unsigned plsize);

//void SetBitMask(unsigned int mask);
//void ScaleLine (int pixels, void *scaleptr, void *picptr, int screen);

void CopyEGA(int wide, int height, int source, int dest);

/*
============================================================================

** Mid level graphic routines

============================================================================
*/

#if 0
int extern sx,sy,leftedge, screencenterx ,screencentery, segoffset;
#else
int extern sx,sy,screencenterx ,screencentery;
#endif

//int Get (void);
int Input(char *string,int max);
unsigned InputInt(void);
void Print (const char *str);
//void Printxy(int x,int y,char *string);
//void PrintC(char *string);
//void PrintHexB(unsigned char value);
//void PrintHex(unsigned value);
//void PrintBin(unsigned value);
//void PrintInt (int val);
//void PrintLong (long val);

int PGet (void);
void PPrint (const char *str);
void CPPrint (const char *str);
void PPrintInt (int val);
void PPrintUnsigned (unsigned val);

void DrawWindow (int xl, int yl, int xh, int yh);
void EraseWindow (void);
//void CharBar (int xl,int yl, int xh, int yh, int ch);
void CenterWindow (int width, int height);
void ExpWin (int width, int height);
void ExpWinH (int width, int height);
void ExpWinV (int width, int height);
//void DrawFrame(int x1,int y1,int x2,int y2,int type);


/*
============================================================================

** Game level routines

============================================================================
*/

long extern score,highscore;

/*
============================================================================

** Needed non library routines

============================================================================
*/

void Quit (char *);

/*
============================================================================

** New stuff

============================================================================
*/

#include <SDL.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "matrix.h"

#include "JM_SB.H"

typedef struct
{
	int width;
	int height;
	unsigned int texture;
} gltexture;

extern SDL_Window * window;

long filelength(FILE * handle);

extern matrix4x4_t matProj2D;

void IDLIBC_SDL_Init();

unsigned int GetColor(int i);

extern int video_screenofs_x, video_screenofs_y;
void SetScreenOfs(void);

void UseTextureShader(float * matrix, float * vertices, int blend, unsigned int texture, float * texcoord);

memptr CreateTexture(int width, int height, const memptr planes, unsigned char background);

void CreateFont();

void VideoSync(void);

#define timecount (SDL_GetTicks()*140/1000)

int *UpdateIntTime();
#define inttime (*(UpdateIntTime()))



void EnableZ(int x, int y, int width, int height, float viewx, float viewz, float viewangle);
void DisableZ();
void DrawPlaneZ(float width, float length, float y, int color);
void DrawWallZ(float x0, float z0, float x1, float z1, int color);
void DrawShapeZ(float x, float z, memptr shapeseg);


// Enables the PC speaker at the given frequency. The argument _frequency should be given in Hertz units.
void sound(int _frequency);
// Disable the PC speaker.
void nosound(void);
