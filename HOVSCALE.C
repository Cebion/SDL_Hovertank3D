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

#include "HOVERDEF.H"

#define scaleshape gltexture

#define MAXPICHEIGHT	256	// tallest pic to be scaled

#define BASESCALE	64	// normal size
#define MAXSCALE	256	// largest scale possible
#define SCALESTEP	3
#define DISCREETSCALES	(MAXSCALE/SCALESTEP)

memptr	basetableseg,screentableseg;	// segment of basetables and screentables
unsigned basetables[DISCREETSCALES],	// offsets in basetableseg
	screentables[DISCREETSCALES];	// offsets in screentableseg

int	 	scalexl = 0,
		scalexh = 319,
		scaleyl = 0,
		scaleyh = 144;

unsigned	scaleblockwidth,
		scaleblockheight,
		scaleblockdest;

void SC_Setup(void)
{
#ifdef OLDDRAW

  unsigned mask,i,step,scale,space;
  unsigned char *baseptr, *screenptr;
  unsigned offset1,offset2,size;

  //
  // fast ploting tables
  //
#if 0
  mask = 128;
  for (i=0;i<320;i++)
  {
    bytetable[i]=i/8;
    masktable[i]=mask;
    if (!(mask>>=1))
      mask = 128;
  }
#endif

  //
  // fast scaling tables
  //

  offset1 = offset2 = 0;

  for (step=0;step<DISCREETSCALES;step++)
  {
    screentables [step] = offset1;
    scale = (step+1)*SCALESTEP;
    size = scale*MAXPICHEIGHT/BASESCALE;
    offset1 += size+1;
    basetables [step] = offset2;
    offset2 += MAXPICHEIGHT;
  }

  MMGetPtr(&basetableseg,offset2);
  MMGetPtr(&screentableseg,offset1);

  for (step=0;step<DISCREETSCALES;step++)
  {
    baseptr = (unsigned char*)basetableseg + basetables[step];
    screenptr = (unsigned char*)screentableseg + screentables[step];

    scale = (step+1)*SCALESTEP;
    size = scale*MAXPICHEIGHT/BASESCALE;

    for (i=0;i<MAXPICHEIGHT;i++)
      *baseptr++ = scale*i/BASESCALE;			// basetable

    for (i=0;i<=size;i++)
      *screenptr++ = i*BASESCALE/scale;			// screentable
  }
#endif
}

void SC_MakeShape (memptr src,int width,int height, memptr *shapeseg)
{
	*shapeseg = src;
}

int SC_ScaleShape (int x,int y,unsigned scale, memptr shape)
{
#ifdef OLDDRAW
  int scalechop;
  unsigned short fullwidth,fullheight,scalewidth,scaleheight;
  unsigned short screencorner;
  int shapex;
  int xl,xh,yl,yh,sx,sy,sxl,sxh,syl,syh;
  unsigned char *basetoscreenptr, *screentobaseptr;

  scalechop = scale/SCALESTEP - 1;

  if (scalechop<0)
    return 0;		// can't scale this size

  if (scalechop>=DISCREETSCALES)
    scalechop = DISCREETSCALES-1;

  basetoscreenptr = (unsigned char*)basetableseg + basetables[scalechop];
  screentobaseptr = (unsigned char*)screentableseg + screentables[scalechop];

//
// figure bounding rectangle for scaled image
//
  fullwidth = ((scaleshape*)shape)->width;
  fullheight = ((scaleshape*)shape)->height;

  scalewidth = fullwidth*((scalechop+1)*SCALESTEP)/BASESCALE; //basetoscreenptr[fullwidth-1];
  scaleheight = basetoscreenptr[fullheight-1];

  xl=x-scalewidth/2;
  xh=xl+scalewidth-1;
  yl=y-scaleheight/2;
  yh=yl+scaleheight-1;


// off screen?

  if (xl>scalexh || xh<scalexl || yl>scaleyh || yh<scaleyl)
    return 0;

//
// clip to sides of screen
//
  if (xl<scalexl)
    sxl=scalexl;
  else
    sxl=xl;
  if (xh>scalexh)
    sxh=scalexh;
  else
    sxh=xh;

//
// clip both sides to zbuffer
//
  sx=sxl;
  while (zbuffer[sx]>scale && sx<=sxh)
    sx++;
  sxl=sx;

  sx=sxh;
  while (zbuffer[sx]>scale && sx>sxl)
    sx--;
  sxh=sx;

  if (sxl>sxh)
    return 0;   		// behind a wall

//
// save block info for background erasing
//
  screencorner = screenofs+yl*linewidth;

  scaleblockdest = screencorner + sxl/8;
  scaleblockwidth = sxh/8-sxl/8+1;
  scaleblockheight = yh-yl+1;


//
// start drawing
//

  DrawScaledShape(sxl, yl, sxh, yh,
	  screentobaseptr[sxl-xl], screentobaseptr[sxh-xl],
	  shape);


#else
	DrawShape(x, y, scale, shape);
#endif
	return 1;
}

void SC_FreeShape (memptr shapeptr)
{
}
