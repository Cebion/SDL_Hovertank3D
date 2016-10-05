


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

/*
=============================================================================

			   HOVERTANK MAIN

=============================================================================
*/
#include "IDLIB.H"

#define OLDDRAW

//#define PROFILE

#ifndef PROFILE
#define ADAPTIVE
#endif


//#define TESTCASE

#define MAXTICS	8

#define NUMLEVELS	20


typedef struct { short width;
		 short height;
		 short planes;
		 short screenx;
		 short screeny;
		 short screenw;
		 short screenh;
		 unsigned short planesize;
	       } LevelDef;

LevelDef extern	*levelheader;


/*
=============================================================================

			      REFRESH

=============================================================================
*/

#define VIEWX		0		// corner of view window
#define VIEWY		0
#define VIEWWIDTH	(40*8)		// size of view window
#define VIEWHEIGHT	(18*8)
#define VIEWXH		(VIEWX+VIEWWIDTH-1)
#define VIEWYH		(VIEWY+VIEWHEIGHT-1)

#define CENTERX		(VIEWX+VIEWWIDTH/2)	// middle of view window
#define CENTERY		(VIEWY+VIEWHEIGHT/2)

#define STATUSLINES	(9*8)		// dash board

#define GLOBAL1		(1l<<16)
#define TILEGLOBAL      GLOBAL1
#define TILESHIFT	16

#define MINDIST		(2*GLOBAL1/5)
#define FOCALLENGTH	(TILEGLOBAL)	// in global coordinates

#define ANGLES		360		// must be divisable by 4

#define MAPSIZE		64		// maps are 64*64 max
#define MAXOBJECTS	100		// max number of tanks, etc / map

//
// 1  sign bit
// 15 bits units
// 16 bits fractional
//
#define SIGNBIT	error//0x80000000l

typedef int32_t fixed;

#define NORTH	0
#define EAST	1
#define SOUTH	2
#define WEST	3

/*
=============================================================================

		      DASH INSTRUMENTS

=============================================================================
*/

#define TIMESECX	3
#define TIMESECY	54
#define TIMEMINX	1
#define TIMEMINY	54

#define RADARX          284	// center of radar
#define RADARY          36
#define RADARSIZE	26		// each way
#define RADARRANGE	(TILEGLOBAL*18)	// each way
#define RADARSCALE	(RADARRANGE/RADARSIZE)

/*
=============================================================================

			     HOVMAIN

=============================================================================
*/

extern int tedlevel;
extern memptr scalesegs[NUMPICS];

//==========================================================================

int CheckKeys(void);
void CachePic (int picnum);
void LoadLevel(void);
void CacheDrawPic(int picnum);
int SoundPlaying (void);

/*
=============================================================================

			     HOVDRAW

=============================================================================
*/

void FollowWalls (void);

/*
=============================================================================

			     HOVDRAW

=============================================================================
*/


typedef struct {int x,y;} tilept;
typedef struct {fixed x,y;} globpt;

typedef struct
{
  int	x1,x2,leftclip,rightclip;// first pixel of wall (may not be visable)
  unsigned	height1,height2,color;
  int wx0,wz0,wx1,wz1;
} walltype;

#define MAXWALLS	100
#define DANGERHIGH	90
#define DANGERLOW	10

#define	MIDWALL		(MAXWALLS/2)


//==========================================================================

#if 0
extern	tilept	tile,lasttile,focal,left,mid,right;

extern	globpt	edge,view;

extern	unsigned screenloc[3];

extern	int screenpage,tics;

extern	long lasttimecount;
#define SHIFTFRAMES	256
extern	int yshift[SHIFTFRAMES];	// screen sliding variables

extern	int firstangle,lastangle;

extern	fixed prestep;

extern	int traceclip,tracetop;

extern	fixed sintable[ANGLES+ANGLES/4],*costable;

extern	fixed	viewx,viewy,viewsin,viewcos;			// the focal point
extern	int	viewangle;

extern	fixed scale,scaleglobal;
extern	unsigned slideofs;

extern	int zbuffer[VIEWXH+1];

extern	walltype	walls[MAXWALLS],*leftwall,*rightwall;
#else
extern	tilept	tile,focal,right;

extern	unsigned screenloc[3];

extern	int screenpage,tics;

extern	int32_t lasttimecount;

extern	fixed sintable[ANGLES+ANGLES/4],*costable;

extern	fixed	viewx,viewy,viewsin,viewcos;			// the focal point

extern	int zbuffer[VIEWXH+1];

extern	walltype	walls[MAXWALLS],*rightwall;
#endif


//==========================================================================

#if 0
void	DrawLine (int xl, int xh, int y,int color);
void	DrawWall (walltype *wallptr);
void	TraceRay (unsigned angle);
fixed	FixedByFrac (fixed a, fixed b);
fixed	FixedAdd (fixed a, fixed b);
void	TransformPoint (fixed gx, fixed gy, int *screenx, unsigned *screenheight);
fixed	TransformX (fixed gx, fixed gy);
void StartView (void);
void FinishView (void);
int	FollowTrace (fixed tracex, fixed tracey, long deltax, long deltay, int max);
void	ForwardTrace (void);
int	FinishWall (void);
int	TurnClockwise (void);
int	TurnCounterClockwise (void);
void	FollowWall (void);

void	NewScene (void);
void	BuildTables (void);
#else

fixed	FixedByFrac (fixed a, fixed b);
void	TransformPoint (fixed gx, fixed gy, int *screenx, unsigned *screenheight);
fixed	TransformX (fixed gx, fixed gy);
void StartView (void);
void FinishView (void);

void	BuildTables (void);
#endif


/*
=============================================================================

			     HOVLOOP

=============================================================================
*/

//
// temp stuff
//
extern int statx,staty;


extern unsigned tilemap[MAPSIZE][MAPSIZE];

typedef enum
  {nothing,playerobj,refugeeobj,droneobj,tankobj,mutantobj,warpobj,
  pshotobj,pbigshotobj,mshotobj,inertobj,shieldobj} classtype;

typedef struct
{
  int		active;
  classtype	_class;
  fixed 	x,y;
  fixed 	viewx,viewy;	// x,y in view coordinate space (NOT pixels!)
  int 		angle;
  int		hitpoints;
  int		radarx,radary,radarcolor;
  int32_t		speed;
  unsigned	size;		// global radius for hit rect calculation
  fixed		xl,xh,yl,yh;	// hit rectangle
  int		ticcount;
  int		shapenum,stage;
  union {
  int		temp1;
  void*		temp1ptr;
  };
  int		temp2;
  dirtype	dir;
  void 		(*think)();
} objtype;

typedef	struct {int min,sec;} timetype;
extern	timetype timestruct;

extern	ControlStruct c;

extern	int guncount,bordertime;

extern	fixed warpx,warpy;	// where to spawn warp gate
extern	fixed xmove,ymove;


extern int godmode,singlestep,leveldone,resetgame;
extern int numrefugees,totalrefugees,savedcount,killedcount;

extern objtype objlist[MAXOBJECTS],obon,*_new,*obj,*lastobj,*check;

void FindFreeObj (void);
void CalcBoundsNew (void);

void CalcBounds (void);
void TransformObon (void);

void PlayerThink (void);

void PlayLoop (void);
void PlayGame (void);

void ClipMove (void);

void StartLevel (unsigned short *plane1);

/*
=============================================================================

			     HOVSCALE

=============================================================================
*/

extern unsigned	scaleblockwidth,
		scaleblockheight,
		scaleblockdest;

int SC_ScaleShape (int x,int y,unsigned scale, memptr shape);
void SC_Setup (void);
void SC_MakeShape (memptr src,int width,int height, memptr *shapeseg);

/*
=============================================================================

			     HOVACTS

=============================================================================
*/

void DamagePlayer (void);
void HealPlayer (void);
void Thrust (void);
void SpawnShield (fixed gx, fixed gy);



// ===========================================================
