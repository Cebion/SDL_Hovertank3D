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

typedef void * memptr;

extern unsigned totalmem;		// total paragraphs available

//#define GUARDBYTES 16

//==========================================================================

//
// public prototypes
//
void MMStartup (void);
void MMShutdown (void);
#ifdef GUARDBYTES
void _MMGetPtr (memptr *baseptr,long size,const char * fname, int line);
#define MMGetPtr(baseptr,size) _MMGetPtr(baseptr,size,__FILE__,__LINE__)
#else
void MMGetPtr (memptr *baseptr,long size);
#endif
void MMFreePtr (memptr *baseptr);
void MMSetPurge (memptr *baseptr, int purge);
void MMSortMem (void);
unsigned MMUnusedMemory (void);
unsigned MMTotalFree (void);

#ifdef GUARDBYTES
void MMValidatePtr (memptr *baseptr);
void MMValidateAll ();
#else
#define MMValidatePtr(baseptr) 
#define MMValidateAll() 
#endif

