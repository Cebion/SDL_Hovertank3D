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

#define CATALOG

#include "HOVERDEF.H"

#include <direct.h> // _chdir

/*

NOTICE TO ANYONE READING THIS:

This is the last gasp of our old routines!  Everything is being rewritten
from scratch to work with new graphic modes and utilities.  This code
stinks!

*/


/*
=============================================================================

			       GLOBALS

=============================================================================
*/

int SNDstarted,KBDstarted;      // whether int handlers were started

//int grhandle,levelhandle,soundhandle;

#pragma pack(push, 1)
typedef struct
{
  short headersize;
  long dictionary;
  long dataoffsets;
} grheadtype;
#pragma pack(pop)

grheadtype	*grhead;	// sets grhuffman and grstarts from here
huffnode	*grhuffman;	// huffman dictionary for egagraph
long            *grstarts;      // array of offsets in egagraph, -1 for sparse
FILE*           grhandle;       // handle to egagraph, kept open allways
long            grposition;     // current seek position in file
long            chunkcomplen;   // compressed length of a chunk


//int soundblaster;               // present?

FILE* levelhandle,*soundhandle;

#define BUFFERSIZE      1024
memptr  bufferseg;              // small general purpose memory block

memptr	levelseg;

int tedlevel,ingame,resetgame;

memptr scalesegs[NUMPICS];

LevelDef *levelheader;

/*
=============================================================================
*/


/*
=============
=
= DoHelpText
=
=============
*/

void DoHelpText(void)
{
  CenterWindow (38,14);

  fontcolor = 13;
  CPPrint ("HoverTank Commands\n");
  fontcolor = 15;
  py+=6;
  PPrint (
"F2  : Sound on / off\n"
"F3  : Keyboard mode / custom\n"
"F4  : Joystick mode\n"
"F5  : Reset game\n"
"ESC : Quit\n");

  py+=6;
  PPrint (
"UP / DOWN : forward/reverse\n"
"LEFT / RIGHT : turn\n");

  py+=6;
  CPPrint ("<MORE>");
  Ack();
  EraseWindow ();

  CPPrint ("Button 1 / Ctrl\n");
  CPPrint ("---------------\n\n");

  PPrint (
/*.....................................*/
"Charge cannon.  A fully charged cannon\n"
"can shoot through multiple targets.\n"
"While the cannon is charging, your\n"
"tanks turning speed is halved for fine\n"
"aiming adjustments.\n\n");

  CPPrint ("<MORE>");
  Ack();
  EraseWindow ();

  CPPrint ("Button 2 / Alt\n");
  CPPrint ("---------------\n\n");
  PPrint (
/*.....................................*/
"Afterburner thrusting.  Doubles your\n"
"forward or backwards speed, but does\n"
"not effect your turning speed.\n");

  Ack();

}


/*
==================
=
= DebugMemory
=
==================
*/

void DebugMemory (void)
{
  CenterWindow (16,7);

  CPPrint ("Memory Usage\n");
  CPPrint ("------------");
  PPrint ("\nTotal     :");
  PPrintUnsigned (totalmem/64);
  PPrint ("k\nFree      :");
  PPrintUnsigned (MMUnusedMemory()/64);
  PPrint ("k\nWith purge:");
  PPrintUnsigned (MMTotalFree()/64);
  PPrint ("k\n");
  CPPrint ("");
  PGet();
}




/*
================
=
= DebugKeys
=
================
*/
void DebugKeys (void)
{
  int i;

  if (keydown[0x22])            // G = god mode
  {
    ExpWin (12,1);
    if (godmode)
      CPPrint ("God mode off");
    else
      CPPrint ("God mode on");
    Ack();
    godmode ^= 1;
  }
  else if (keydown[0x32])       // M = memory info
  {
	DebugMemory();
  }
  if (keydown[0x19])            // P = pause with no screen disruptioon
  {
	singlestep=1;
  }
  if (keydown[0x1f])            // S = shield point
  {
	screenofs = 0;
	HealPlayer();
  }
  else if (keydown[0x14])       // T = free time
  {
	if (timestruct.min<9)
	  timestruct.min++;
	screenofs = 0;
	DrawPic (6,48,DIGIT0PIC+timestruct.min);
  }
  else if (keydown[0x11])       // W = warp to level
  {
    ExpWin(26,1);
    PPrint("Warp to which level(1-20):");
    i = InputInt();
    if (i>=1 && i<=21)
    {
      level = i-1;
      leveldone=1;
    }
  }

}

/*=========================================================================*/

/*
=============
=
= CheckKeys
=
= Checks to see if an F-key is being pressed and handles it
=
=============
*/

int CheckKeys(void)
{
  if (!NBKscan)
    return 0;

  int ch;
  switch (NBKscan&0x7f)
  {
    case 0x3b:                  // F1 = help
      ClearKeys ();
      DoHelpText ();
      break;
    case 0x3c:                  // F2 = sound on/off
      ClearKeys ();
      ExpWin (13,1);
      PPrint ("Sound (Y/N)?");
	  ch=toupper(PGet()&0xff);
      if (ch=='N')
	soundmode = 0;
      else if (ch=='Y')
	soundmode = 1;
      break;
#if 0
    case 0x3d:                  // F3 = keyboard mode
      ClearKeys ();
      calibratekeys ();
      break;
    case 0x3e:                  // F4 = joystick mode
      ClearKeys ();
      CalibrateJoy (1);
      break;
#endif
    case 0x3f:                  // F5 = reset game
      ClearKeys ();
      ExpWin (18,1);
      PPrint ("RESET GAME (Y/N)?");
      ch=toupper(PGet()&0xff);
      if (ch=='Y')
      {
	resetgame = 1;
	leveldone = -99;
      }
      break;

    case 0x58:                  // F12 + ? = debug keys
      DebugKeys();
      break;
	 case 1:                        // ESC = quit
      ClearKeys ();
      ExpWin (12,1);
		PPrint ("QUIT (Y/N)?");
      ch=toupper(PGet()&0xff);
      if (ch=='Y')
	Quit ("");
      break;


    default:
      return 0;
  }


  ClearKeys ();
  return 1;
}


//==========================================================================

/*
============================
=
= GetChunkLength
=
= Seeks into the igrab data file at the start of the given chunk and
= reads the uncompressed length (first four bytes).  The file pointer is
= positioned so the compressed data can be read in next.
= ChunkCompLen is set to the calculated compressed length
=
============================
*/

long GetChunkLength (int chunk)
{
  long len;

  fseek(grhandle,grstarts[chunk],SEEK_SET);
  fread(&len,1,sizeof(len),grhandle);
  chunkcomplen = grstarts[chunk+1]-grstarts[chunk]-4;

  return len;
}

//==========================================================================

/*
============================
=
= LoadNearData
=
= Load stuff into data segment before memory manager is
= started (which takes all available memory, near and far)
=
============================
*/

void LoadNearData (void)
{
  FILE *handle;
  long length;

//
// load egahead.ext (offsets and dictionary for graphics file)
//
  if (!(handle = fopen("EGAHEAD."EXTENSION, "rb")))
	 Quit ("Can't open EGAHEAD."EXTENSION"!");

  length = filelength(handle);
  grhead = (grheadtype*)malloc(length);

  fread(grhead, 1, length, handle);

  fclose(handle);


}

//==========================================================================

/*
==========================
=
= SegRead
=
= Read from a file to a segment pointer
=
==========================
*/

void SegRead (FILE * handle, memptr dest, long length)
{
  if (length>0xffffl)
	 Quit ("SegRead doesn't support 64K reads yet!");

#if 0
asm             push    ds
asm             mov     bx,[handle]
asm             mov     cx,[WORD PTR length]
asm             mov     dx,0                    // segment aligned
asm             mov     ds,[dest]
asm             mov     ah,3fh                  // READ w/handle
asm             int     21h
asm             pop     ds
#endif

  fread (dest,1,length,handle);

}

//==========================================================================



/////////////////////////////////////////////////////////
//
// InitGrFile
//
/////////////////////////////////////////////////////////

void InitGrFile (void)
{
  memptr buffer;

//
// calculate some offsets in the header
//
  grhuffman = (huffnode *)( ((char *)grhead)+grhead->dictionary);
  grstarts = (long *)( ((char *)grhead)+grhead->dataoffsets);

  OptimizeNodes (grhuffman);

//
// Open the graphics file, leaving it open until the game is finished
//
  grhandle = fopen("EGAGRAPH."EXTENSION, "rb");
  if (!grhandle)
	 Quit ("Cannot open EGAGRAPH."EXTENSION"!");


//
// load the pic and sprite headers into the data segment
//
#if NUMPICS>0
  needgr[STRUCTPIC] = 1;                // make sure this chunk never reloads
  grsegs[STRUCTPIC] = (memptr)0xffff;
  GetChunkLength(STRUCTPIC);            // position file pointer
  MMGetPtr(&buffer, chunkcomplen);
  SegRead (grhandle,buffer,chunkcomplen);
  HuffExpand ((unsigned char *)buffer, (unsigned char *)pictable,
    sizeof(pictable),grhuffman);
  MMFreePtr(&buffer);
#endif

#if NUMPICM>0
  needgr[STRUCTPICM] = 1;               // make sure this chunk never reloads
  grsegs[STRUCTPICM] = (memptr)0xffff;
  GetChunkLength(STRUCTPICM);           // position file pointer
  MMGetPtr(&buffer, chunkcomplen);
  SegRead (grhandle,buffer,chunkcomplen);
  HuffExpand (buffer, (unsigned char *)picmtable,
    sizeof(picmtable),grhuffman);
  MMFreePtr(&buffer);
#endif

#if NUMSPRITES>0
  needgr[STRUCTSPRITE] = 1;             // make sure this chunk never reloads
  grsegs[STRUCTSPRITE] = (memptr)0xffff;
  GetChunkLength(STRUCTSPRITE); // position file pointer
  MMGetPtr(&buffer, chunkcomplen);
  SegRead (grhandle,buffer,chunkcomplen);
  HuffExpand (buffer, (unsigned char huge *)spritetable,
    sizeof(spritetable),grhuffman);
  MMFreePtr(&buffer);
#endif


}


//==========================================================================

/*
==========================
=
= CacheGrFile
=
= Goes through grneeded and grsegs, and makes sure
= everything needed is in memory
=
==========================
*/

// base tile sizes for EGA mode
#define BLOCK           32
#define MASKBLOCK       40

void ExpandGrChunk(int chunk)
{
	if (grsegs[chunk])
	{
		return;
	}

	int expanded = GetChunkLength(chunk);
	int compressed = grstarts[chunk+1]-grstarts[chunk]-4;

	memptr bigbufferseg;          // for compressed

	MMGetPtr(&grsegs[chunk],expanded);
	MMGetPtr(&bigbufferseg,compressed);
	fread(bigbufferseg,1,compressed,grhandle);
	HuffExpand ((unsigned char*)bigbufferseg, (unsigned char*)grsegs[chunk], expanded,grhuffman);
	MMValidatePtr(&grsegs[chunk]);
	MMFreePtr(&bigbufferseg);
}

void CacheGrFile (void)
{
  int i;
  long filepos,newpos;          // current seek position in file
  long expanded,compressed;     // chunk lengths
  memptr bigbufferseg;          // for compressed

//
// make unneeded chunks purgable
//
  for (i=0;i<NUMCHUNKS;i++)
    if (grsegs[i] && !needgr[i])
      MMSetPurge(&grsegs[i],3);

  MMSortMem();

//
// load new stuff
//
  fseek(grhandle,0,SEEK_SET);
  filepos = 0;

  for (i=0;i<NUMCHUNKS;i++)
    if (!grsegs[i] && needgr[i])
    {
      newpos = grstarts[i];
      if (newpos!=filepos)
	fseek(grhandle,newpos-filepos,SEEK_CUR);

      compressed = grstarts[i+1]-grstarts[i]-4;

      if (i>=STARTTILE8)
      {
      //
      // tiles are of a known size
      //
	if (i<STARTTILE8M)              // tile 8s are all in one chunk!
	  expanded = BLOCK*NUMTILE8;
	else if (i<STARTTILE16)
	  expanded = MASKBLOCK*NUMTILE8M;
	else if (i<STARTTILE16M)        // all other tiles are one/chunk
	  expanded = BLOCK*4;
	else if (i<STARTTILE32)
	  expanded = MASKBLOCK*4;
	else if (i<STARTTILE32M)
	  expanded = BLOCK*16;
	else
	  expanded = MASKBLOCK*16;

	compressed = grstarts[i+1]-grstarts[i];
      }
      else
      {
		//
      // other things have a length header at start of chunk
      //
	fread(&expanded,1,sizeof(expanded),grhandle);
	compressed = grstarts[i+1]-grstarts[i]-4;
      }

      //
      // allocate space for expanded chunk
      //
      MMGetPtr(&grsegs[i],expanded);

      //
      // if the entire compressed length can't fit in the general purpose
		// buffer, allocate a temporary memory block for it
      //
      if (compressed<=BUFFERSIZE)
      {
	SegRead(grhandle,bufferseg,compressed);
	HuffExpand ((unsigned char*)bufferseg, (unsigned char*)grsegs[i], expanded,grhuffman);
      }
      else
      {
	MMGetPtr(&bigbufferseg,compressed);
	SegRead(grhandle,bigbufferseg,compressed);
	HuffExpand ((unsigned char*)bigbufferseg, (unsigned char*)grsegs[i], expanded,grhuffman);
	MMFreePtr(&bigbufferseg);
      }

      filepos = grstarts[i+1];  // file pointer is now at start of next one
    }

}

//==========================================================================


/*
=====================
=
= CachePic
=
= Make sure a graphic chunk is in memory
=
=====================
*/

void CachePic (int picnum)
{
	long expanded,compressed;     // chunk lengths
	memptr bigbufferseg;          // for compressed

	if (grsegs[picnum])
		return;

	fseek(grhandle,grstarts[picnum],SEEK_SET);

	compressed = grstarts[picnum+1]-grstarts[picnum]-4;

	int width,height;

	if (picnum>=STARTTILE8)
	{
	//
	// tiles are of a known size
	//
		int numtiles = 0;
		int tilesize = 0;
		if (picnum<STARTTILE8M)             // tile 8s are all in one chunk!
		{
			numtiles = NUMTILE8;
			tilesize = BLOCK;
			width = 1;
			height = 8;
		}
		else if (picnum<STARTTILE16)
		{
			numtiles = NUMTILE8M;
			tilesize = MASKBLOCK;
			assert(0);
		}
		else if (picnum<STARTTILE16M)       // all other tiles are one/chunk
		{
			numtiles = 4;
			tilesize = BLOCK;
			assert(0);
		}
		else if (picnum<STARTTILE32)
		{
			numtiles = 4;
			tilesize = MASKBLOCK;
			assert(0);
		}
		else if (picnum<STARTTILE32M)
		{
			numtiles = 16;
			tilesize = BLOCK;
			assert(0);
		}
		else
		{
			numtiles = 16;
			tilesize = MASKBLOCK;
			assert(0);
		}

		expanded = numtiles * tilesize;

		compressed = grstarts[picnum+1]-grstarts[picnum];
		if (compressed < 0 || compressed > expanded)
		{
			compressed = expanded;
		}
		//
		// allocate space for expanded chunk
		//
		memptr planes;
		MMGetPtr(&planes,expanded);
		MMGetPtr(&bigbufferseg,compressed);
		fread(bigbufferseg,1,compressed,grhandle);
		HuffExpand ((unsigned char*)bigbufferseg, (unsigned char*)planes, expanded,grhuffman);

		memptr *tiles;
		MMGetPtr((memptr*)&tiles, numtiles*sizeof(memptr*));
		for (int i = 0; i < numtiles; i++)
		{
			tiles[i] = CreateTexture(width,height,(char*)planes+i*tilesize, 5);
		}
		grsegs[picnum] = tiles;
		MMValidatePtr((memptr*)&tiles);

		MMFreePtr(&bigbufferseg);
		MMFreePtr(&planes);
		return;
	}
	else
	{
	//
	// other things have a length header at start of chunk
	//
		fread(&expanded,1,sizeof(expanded),grhandle);
		compressed = grstarts[picnum+1]-grstarts[picnum]-4;
		width = pictable[picnum-STARTPICS].width;
		height = pictable[picnum-STARTPICS].height;
	}

	assert(width*height*4 == expanded);
	//
	// allocate space for expanded chunk
	//
	memptr planes;
	MMGetPtr(&planes,expanded);
	MMGetPtr(&bigbufferseg,compressed);
	fread(bigbufferseg,1,compressed,grhandle);
	HuffExpand ((unsigned char*)bigbufferseg, (unsigned char*)planes, expanded,grhuffman);
	grsegs[picnum] = CreateTexture(width,height,planes, 5);
	MMValidatePtr(&grsegs[picnum]);
	MMFreePtr(&bigbufferseg);
	MMFreePtr(&planes);
}

#if 0
void TestSave(int picnum)
{
	memptr src = grsegs[STARTPICS+picnum];
	int plane_width = pictable[picnum].width;
	int width = plane_width*8;
	int height = pictable[picnum].height;
	int size = width * height;

	unsigned char *data;
	MMGetPtr((memptr*)&data,size);	// larger than needed buffer
	memset(data,0,size);

	unsigned char *plane = (unsigned char*)src;

	for (int p = 0; p < 4; p++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < plane_width; x++)
			{
				int offset = y*width + x*8;
				data[offset + 0] |= (((*plane)>>7)&1) << p;
				data[offset + 1] |= (((*plane)>>6)&1) << p;
				data[offset + 2] |= (((*plane)>>5)&1) << p;
				data[offset + 3] |= (((*plane)>>4)&1) << p;
				data[offset + 4] |= (((*plane)>>3)&1) << p;
				data[offset + 5] |= (((*plane)>>2)&1) << p;
				data[offset + 6] |= (((*plane)>>1)&1) << p;
				data[offset + 7] |= (((*plane)>>0)&1) << p;
				plane++;
			}
		}
	}

	FILE *f = fopen("_test.raw","wb");
	fwrite(data, 1, size, f);
	fclose(f);

	MMFreePtr((memptr*)&data);
}
#endif



//==========================================================================

/*
=====================
==
== Quit
==
=====================
*/

void Quit (char *error)
{
//char extern far PIRACY;

  if (!(*error))
  {
	 SaveCtrls ();
  }

  MMShutdown();
#if 0
  if (KBDstarted)
	 ShutdownKbd ();        // shut down the interrupt driven stuff if needed
  if (SNDstarted)
	 ShutdownSound ();
#endif
  if (soundblaster)
	 jmShutSB ();

  if (grhandle>0)
	 fclose(grhandle);
  if (levelhandle>0)
	 fclose(levelhandle);
  if (soundhandle>0)
	 fclose(soundhandle);

#if 0
  _AX = 3;
  geninterrupt (0x10);  // text mode
#endif

  if (!(*error))
  {
#if 0
	 movedata (FP_SEG(&PIRACY),FP_OFF(&PIRACY),0xb800,0,4000);
	 bioskey (0);
	 clrscr();
#endif

#ifndef CATALOG
	_argc = 2;
	_argv[1] = "LAST.SHL";
	_argv[2] = "ENDSCN.SCN";
	_argv[3] = NULL;
	if (execv("LOADSCN.EXE", _argv) == -1)
	{
		clrscr();
		puts("Couldn't find executable LOADSCN.EXE.\n");
		exit(1);
	}
#endif

  }
  else
	 puts (error);


  exit (0);             // quit to DOS
}


//==========================================================================

/*
======================
=
= LoadLevel
=
= Loads LEVEL00.EXT (00 = global variable level)
=
======================
*/

void LoadLevel(void)
{
  unsigned short *planeptr;
  int loop,x,y,i,j;
  unsigned length;
  char filename[30];
  char num[3];
  memptr bufferseg;


//
// load the new level in and decompress
//
  if (level<10)
  {
    itoa (level,num,10);
    strcpy (filename,"LEVEL0");
  }
  else
  {
    itoa (level,num,10);
    strcpy (filename,"LEVEL");
  }

  strcat (filename,num);
  strcat (filename,"."EXTENSION);

  BloadinMM(filename,&bufferseg);

  length = *(unsigned short*)bufferseg;

  if (levelseg)
    MMFreePtr (&levelseg);

  MMGetPtr (&levelseg,length);

  RLEWExpand ((unsigned short *)bufferseg,(unsigned short *)levelseg);

  MMFreePtr (&bufferseg);

  levelheader = (LevelDef *)levelseg;

//
// copy plane 0 to tilemap
//
  planeptr= (unsigned short *)((char *)levelseg+32);
  for (y=0;y<levelheader->height;y++)
    for (x=0;x<levelheader->width;x++)
      tilemap[x][y]=*planeptr++;


//
// spawn tanks
//
  planeptr= (unsigned short *)((char *)levelseg+32+levelheader->planesize);
  StartLevel (planeptr);

  MMFreePtr (&levelseg);

}

//==========================================================================


/*
=================
=
= CacheDrawPic
=
=================
*/

void CacheDrawPic(int picnum)
{
  int i;

  CachePic (STARTPICS+picnum);

  EGASplitScreen(200);
  SetScreen(0,0);
  SetLineWidth(80);
  screenofs = 0;

//EGAWRITEMODE(0);
  DrawPic (0,0,picnum);

//EGAWRITEMODE(1);
//EGAMAPMASK(15);
  CopyEGA(80,200,0,0x4000);
//EGAWRITEMODE(0);

  MMSetPurge (&grsegs[STARTPICS+picnum],3);

}


//==========================================================================

#if 0
int SoundPlaying (void)
{
  if (soundblaster)
    return jmSamplePlaying();
  else
    return sndptr;
}
#endif

#if 0

/*
=====================
=
= PlaySound
=
= Dispatches to either pc speaker sound routines or sound blaster
= digitized routines
=
=====================
*/

void PlaySound (int num)
{
  if (soundblaster)
    jmPlaySample(num);
  else
    PlaySoundSPK(num);
}

#endif

//==========================================================================



/*
=====================
=
= Intro
=
=====================
*/

void Intro (void)
{
  memptr shapeseg;
  int i,f,sx,sy,page;
  unsigned pageptr[2],pagewidth[2],pageheight[2];
  float x,y,z,angle,step,sn,cs,maxz,sizescale,maxy,coordscale,scale;
  float ytop,xmid,minz,worldycenter,worldxcenter;

	FadeOut();

	SetLineWidth(SCREENWIDTH);

	screenofs=0;

	CacheDrawPic (STARSPIC);
	pxl=0;
	pxh=320;
	py=180;
#ifndef CATALOG
	CPPrint ("Copyright (c) 1991-93 Softdisk Publishing\n");
//	  CPPrint ("'I' for information");
#endif
	//EGAWRITEMODE(1);
	//EGAMAPMASK(15);
#if 0
	CopyEGA(40,200,0,0x4000);
	CopyEGA(40,200,0,0x8000);
	CopyEGA(40,200,0,0xc000);
#endif
	StopDrive();

	CachePic (STARTPICS+LOGOPIC);

#if 0
  SC_MakeShape(
    grsegs[STARTPICS+LOGOPIC],
    0,
    0,
    &shapeseg);

//	SC_MakeShape(
//		grsegs[STARTPICS+LOGOPIC],
//		pictable[LOGOPIC].width,
//		pictable[LOGOPIC].height,
//		&shapeseg);

  //MMFreePtr(&grsegs[STARTPICS+LOGOPIC]);
#endif


	FadeIn();
	sx=160;
	sy=180;

//	memset (zbuffer,0,sizeof(zbuffer));

/*
=============================================================================

		  SCALED PICTURE DIRECTOR

=============================================================================
*/

#define PICHEIGHT       64      // full size height of scaled pic
#define NUMFRAMES       300.0f
#define MAXANGLE        (3.141592657f*0.6f)       // go from 0 to this in numframes
#define RADIUS          1000.0f  // world coordinates
#define DISTANCE        1000.0f  // center point z distance

#if 0
	minz = cosf(MAXANGLE)*RADIUS;  // closest point
	minz += DISTANCE;
	sizescale = 256*minz;         // closest point will be full size
	ytop = 80 - (PICHEIGHT/2)*(sizescale/DISTANCE)/256;
	z = sizescale/(DISTANCE*256);
	ytop = ytop/z;        // world coordinates
	worldycenter=ytop-RADIUS;
	xmid=sinf(MAXANGLE)*RADIUS/2;
	worldxcenter=-xmid;
#endif

	f=1;
	page = inttime = screenofs = pagewidth[0] = pagewidth[1] = 0;
	do
	{
		step = f/NUMFRAMES;
		angle=MAXANGLE*step;
		sn=sin(angle);
#if 0
		cs=cos(angle);
		x=worldxcenter+sn*RADIUS/2;
		y=worldycenter+sn*RADIUS;
		z=DISTANCE+cs*RADIUS;
		scale = sizescale/z;
		sx=160+ (int)(x*scale/256);
		sy=100- (int)(y*scale/256);
#endif

		inttime=0;
		sound((int)(sn*1500));

#if 0
//
// erase old position
//
	 if (pagewidth[page])
	 {
		//EGAWRITEMODE(1);
		//EGAMAPMASK(15);
		CopyEGA(pagewidth[page],pageheight[page],
		pageptr[page]+0x8000,pageptr[page]);
	 }

//
// draw new position
//
	 //EGAWRITEMODE(2);
	 if (SC_ScaleShape(sx,sy,(int)scale<40 ? 10 : scale/4,shapeseg))
	 {
		pagewidth[page]=scaleblockwidth;
		pageheight[page]=scaleblockheight;
		pageptr[page]=scaleblockdest;
	 }
	 else
		pagewidth[page]=0;

	 //EGAWRITEMODE(0);
	 //EGABITMASK(255);

//
// display it
//
#endif
	 SetScreen(screenofs,0);

#if 0
	 page^=1;
	 screenofs = 0x4000*page;
#endif

		f++;

		if (f<NUMFRAMES)
		{
			f+=inttime;
			if (f>NUMFRAMES)
				f=NUMFRAMES;
		}
		else
			f++;  // last frame is shown

	 if (NBKscan>0x7f)
		break;
	} while (f<=NUMFRAMES);
  nosound();

  for (i=0;i<200;i++)
  {
	 WaitVBL(1);
	 if (NBKscan>0x7f)
	 {
#if 0
		if (NBKscan==0x97)              //'I' for info
		{
	screenofs^=0x4000;
  CenterWindow(24,10);
	py+=2;
  CPPrint ("Hovertank v1.17\n\n");
	CPPrint ("Softdisk Publishing delivers a\n");
	CPPrint ("high quality EGA game to\n");
	CPPrint ("your door every month!\n");
	CPPrint ("Call 1-800-831-2694 for\n");
	CPPrint ("subscription rates and\n");
	CPPrint ("back issues.\n");
	ClearKeys();
	Ack();
		}
		ClearKeys();
		break;
#endif

	 }
  }

#if 0
	SC_FreeShape(shapeseg);
#endif
}

//==========================================================================



/*
=====================
==
== DemoLoop
==
=====================
*/
#define PAUSE   300
void DemoLoop (void)
{
  int i,originx;
  ControlStruct c;

  FadeOut();

  CacheDrawPic (TITLEPIC);
  StopDrive();  // make floppy motors turn off

  FadeIn ();

  originx=0;
  i=100;
  while (1)
  {
    if (i>PAUSE && i<=PAUSE+80)
      originx+=4;

    if (i>PAUSE*2 && i<=PAUSE*2+80)
      originx-=4;

    if (i>PAUSE*2+80)
      i=0;

    SetScreen(originx/8,originx%8);

    i++;

    screenofs = originx/8;
    if (CheckKeys())
    {
//      EGAWRITEMODE(1);
//      EGAMAPMASK(15);
      CopyEGA(80,200,0x4000,0);
    }
    c=ControlPlayer(1);
    if (c.button1 || c.button2)
      break;
    if (keydown[0x39])
      break;
  }

  ClearKeys();
}


//==========================================================================

/*
====================
=
= SetupGraphics
=
====================
*/
void SetupGraphics (void)
{
  int i;

  InitGrFile ();        // load the graphic file header

	CachePic(STARTTILE8);

	for (int i = 0; i < NUMPICS; i++)
	{
		CachePic(i+STARTPICS);
	}

	ExpandGrChunk(STARTFONT);
	CreateFont();

//
// go through the pics and make scalable shapes, the discard the pic
//
  for (i=MAN1PIC;i<DASHPIC;i++)
  {
    CachePic (STARTPICS+i);
    SC_MakeShape(
      grsegs[STARTPICS+i],
      pictable[i].width,
      pictable[i].height,
      &scalesegs[i]);
    //MMFreePtr (&grsegs[STARTPICS+i]);
  }

//
// load the basic graphics
//

	needgr[STARTFONT] = 1;
	needgr[STARTTILE8] = 1;

	for (i=DASHPIC;i<ENDPIC;i++)
		needgr[STARTPICS+i]=1;

#if 0
	CacheGrFile ();       // load all graphics now (no caching)

	fontseg = (fontstruct*)grsegs[STARTFONT];
#endif
}

//==========================================================================

//==========================================================================

//////////////////////////////////////////////////////
//
// Hardware Error Handler - called only by MS-DOS
//
//////////////////////////////////////////////////////

#if 0
#define IGNORE  0
#define RETRY   1
#define ABORT   2

int ErrorHandler(int errval,int ax,int bx,int si)
{
  unsigned key;

  key=ax+bx+si+errval;

//  screenofs=screenorigin=0;
//  SetScreen(0,0);
  CenterWindow(32,3);
  py++;
  CPPrint("Disk I/O error! Press ENTER to\n");
  CPPrint("resume, or ESC to abort:");
  SetNormalPalette();

  ClearKeys();

  do{
	 key=(PGet()&0xff);
	} while(key!=27 && key!=13);

  if (key!=27)
	hardresume(RETRY);

  _AX = 3;
  geninterrupt (0x10);  // text mode

  if (KBDstarted)
	ShutdownKbd (); // shut down the interrupt driven stuff if needed
  if (SNDstarted)
	ShutdownSound ();

  return ABORT;
}
#endif



/*=========================================================================*/

////////////////////////////////////////////////////////////
//
// Allocate memory and load file in
//
////////////////////////////////////////////////////////////
void LoadIn(char *filename,char **baseptr)
{
 FILE *handle;
 long len;
 unsigned datapage;


 if (!(handle=fopen(filename,"rb")))
   {
	printf("Error loading file '%s'!\n",filename);
	exit(1);
   }

 len=filelength(handle);
 *baseptr=(char *)malloc(len);

 fread(*baseptr,1,len,handle);
 fclose(handle);
 //LoadFile(filename,*baseptr);
}

///////////////////////////////////////////////////////////////////////////
//
//      US_CheckParm() - checks to see if a string matches one of a set of
//              strings. The check is case insensitive. The routine returns the
//              index of the string that matched, or -1 if no matches were found
//
///////////////////////////////////////////////////////////////////////////
int
US_CheckParm(char *parm,char **strings)
{
	char    cp,cs,
			*p,*s;
	int             i;

	while (!isalpha(*parm)) // Skip non-alphas
		parm++;

	for (i = 0;*strings && **strings;i++)
	{
		for (s = *strings++,p = parm,cs = cp = 0;cs == cp;)
		{
			cs = *s++;
			if (!cs)
				return(i);
			cp = *p++;

			if (isupper(cs))
				cs = tolower(cs);
			if (isupper(cp))
				cp = tolower(cp);
		}
	}
	return(-1);
}
///////////////////////////////////////////////////////////////////////////

/*
=================
=
= main
=
=================
*/

static  char                    *EntryParmStrings[] = {"detour",0};
static  char                    *SBlasterStrings[] = {"NOBLASTER",0};

int main(int argc, char **argv)
{
  int i,x,xl,xh,y,plane,size;
  SampledSound *samples;

	FILE * ftest = fopen("EGAGRAPH.HOV", "rb");
	if (ftest)
	{
		fclose(ftest);
	}
	else
	{
		_chdir("HOVERTAN");
		FILE * ftest = fopen("EGAGRAPH.HOV", "rb");
		if (ftest)
		{
			fclose(ftest);
		}
		else
		{
			SDL_Log("Unable to find game files\n");
		}
	}

	boolean LaunchedFromShell = false;

//	textbackground(0);
//	textcolor(7);
	if (argc > 1 && stricmp(argv[1], "/VER") == 0)
	{
		printf("HOVERTANK 3-D\n");
		printf("Copyright 1991-93 Softdisk Publishing\n");
		printf("Version 1.17\n");
		exit(0);
	}

	for (i = 1;i < argc;i++)
	{
		switch (US_CheckParm(argv[i],EntryParmStrings))
		{
		case 0:
			LaunchedFromShell = true;
			break;
		}
	}
#ifndef CATALOG
	if (!LaunchedFromShell)
	{
		//clrscr();
		puts("You must type START at the DOS prompt to run HOVERTANK 3-D.");
		exit(0);
	}
#endif

//  puts("HoverTank 3-D is executing...");


	IDLIBC_SDL_Init();

#if 0
//
// detect video
//
  videocard = VideoID ();

  if (videocard == EGAcard) {}
//    puts ("EGA card detected");
  else if (videocard == VGAcard) {}
//    puts ("VGA card detected");
  else
  {
	clrscr();
	 puts ("Hey, I don't see an EGA or VGA card here!  Do you want to run the program ");
	 puts ("anyway (Y = go ahead, N = quit to dos) ?");
	 ClearKeys ();
	 i = toupper(bioskey(0) & 0xff);
	 if (i!='Y')
		exit (1);
  }

  grmode = EGAgr;
#endif

//
// setup for sound blaster
//
	soundblaster = 1;
	for (i = 1;i < argc;i++)
	{
		switch (US_CheckParm(argv[i],SBlasterStrings))
		{
		case 0:
			soundblaster = 0;
			break;
		}
	}

	if (soundblaster)
		soundblaster = jmDetectSoundBlaster(-1);

#if 0
	if (stricmp(_argv[1], "NOBLASTER") == 0)
	 soundblaster = 0;
  else
	 soundblaster = jmDetectSoundBlaster(-1);

#endif




  if (soundblaster)
  {
//       puts ("Sound Blaster detected! (HOVER NOBLASTER to void detection)");
	 LoadIn ("DSOUND.HOV",(char**)&samples);
	 jmStartSB ();
	 jmSetSamplePtr (samples);
  }
//  else
//       puts ("Sound Blaster not detected");

  LoadNearData ();      // load some stuff before starting the memory manager

  MMStartup ();
  MMGetPtr(&bufferseg,BUFFERSIZE);      // small general purpose buffer

  BloadinMM ("SOUNDS."EXTENSION,&soundseg);
  assert(sizeof(spksndtype) == 16);
  spksndtype * sounds = (spksndtype*)soundseg;

  for (int i = 0; i < 20; i++)
  {
	  SDL_Log(sounds[i+1].name);
  }

//  harderr(ErrorHandler);        // critical error handler

	//StartupKbd ();
	//KBDstarted = 1;

#ifdef ADAPTIVE
//  timerspeed = 0x2147;  // 140 ints / second (2/VBL)
//  StartupSound ();      // interrupt handlers that must be removed at quit
  SNDstarted = 1;
#endif

	SetupGraphics ();

	InitRndT (1);         // setup random routines
	InitRnd (1);

	LoadCtrls ();

//	puts ("Calculating...");
	BuildTables();
	SC_Setup();

//	SetScreenMode(grmode);
	SetLineWidth (SCREENWIDTH);

	screencenterx=19;
	screencentery=12;

#if !(defined (PROFILE) || defined (TESTCASE))
	if (!keydown[1])              // hold ESC to bypass intro
		Intro ();
#endif

#ifdef PROFILE
JoyXlow[1]=JoyYlow[1]=16;
JoyXhigh[1]=JoyYhigh[1]=70;
playermode[1] = joystick1;
#endif

	while (1)
	{
#if !(defined (PROFILE) || defined (TESTCASE))
		DemoLoop ();                // do title, demo, etc
#endif
		PlaySound (STARTGAMESND);
		PlayGame();
	}

	return 0;
}

