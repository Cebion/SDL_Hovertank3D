
#include "IDLIB.H"

#include <stdint.h>
#include <assert.h>

#if NUMPICS>0
pictype	pictable[NUMPICS];
#endif

memptr	grsegs[NUMCHUNKS];
char	needgr[NUMCHUNKS];	// for caching

/*
===============
=
= OptimizeNodes
=
= Goes through a huffman table and changes the 256-511 node numbers to the
= actular address of the node.  Must be called before HuffExpand
=
===============
*/

void OptimizeNodes (huffnode *table)
{
	// only works on older systems
}



/*
======================
=
= HuffExpand
=
======================
*/

void HuffExpand (unsigned char *source, unsigned char *dest,
  long length,huffnode *hufftable)
{
	unsigned bit,byte,code;
	huffnode *nodeon,*headptr;

	headptr = hufftable+254;	// head node is allways node 254
	nodeon = headptr;

	bit = 1;
	byte = *source++;

	while (length)
	{
		if (byte&bit)
			code = nodeon->bit1;
		else
			code = nodeon->bit0;

		bit<<=1;
		if (bit==256)
		{
			bit=1;
			byte = *source++;
		}

		if (code<256)
		{
			*dest++=code;
			nodeon=headptr;
			length--;
		}
		else
		{
			nodeon = &hufftable[code-256];
		}
	}

#if 0
	huffnode *headptr;
	headptr = Hufftable+254;	// head node is allways node 254

	huffnode *curhuffnode = headptr;
	int i, j;
	int written = 0;
	i = 0;
	while (written < length) {
		uint8_t srcbyte = Source[i++];
		for (j = 0; j < 8; j++) {
			unsigned short b = curhuffnode->bit0;
			if (srcbyte&1) {
				b = curhuffnode->bit1;
			}
			srcbyte = srcbyte>>1;
			if (b < 256) {
				Dest[written++] = (uint8_t)b;
				curhuffnode = headptr;
				if (written == length) {
					break;
				}
			} else {
				assert(b-256 >= 0);
				curhuffnode = &Hufftable[b-256];
			}
		}
	}
#endif
}

/*========================================================================*/


/*
======================
=
= RLEWexpand
=
======================
*/
#define RLETAG 0xFEFE

void RLEWExpand (unsigned short *source, unsigned short *dest)
{
  long length;
  unsigned short value,count,i;
  unsigned short *start, *end;

  length = *(long*)source;
  end = dest + (length)/2;

  source+=2;		// skip length words
//
// expand it
//
  do
  {
    value = *source++;
    if (value != RLETAG)
    //
    // uncompressed
    //
      *dest++=value;
    else
    {
    //
    // compressed string
    //
      count = *source++;
      value = *source++;
      if (dest+count>end)
	Quit("RLEWExpand error!");

      for (i=1;i<=count;i++)
	*dest++ = value;
    }
  } while (dest<end);

}


/*
=============================================================================
**
** Miscellaneous library routines
**
=============================================================================
*/

long filelength(FILE * handle)
{
	long cur,end;
	cur = ftell(handle);
	fseek(handle,0,SEEK_END);
	end = ftell(handle);
	fseek(handle,cur,SEEK_SET);
	return end;
}

//==========================================================================


/*
====================================
=
= BloadinMM
=
====================================
*/

void BloadinMM (char *filename,memptr *spot)
{
  FILE * handle;
  long length;
  char *location;
  char error[80];

  if ( handle = fopen (filename,"rb") )
  {
    length = filelength (handle);
    MMGetPtr (spot,length);
	fread (*spot, 1, length, handle);
    fclose (handle);
    //LoadFile (filename,*spot);
  }
  else
  {
    strcpy (error,"BloadinMM: Can't find file ");
    strcat (error,filename);
    Quit (error);
  }
}



// id_us_a.asm

static unsigned rndindex=0;

static unsigned rndtable[] = {0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66,
	74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36,
	95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188,
	52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224,
	149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242,
	145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0,
	175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235,
	25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113,
	94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75,
	136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196,
	135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113,
	80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241,
	24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224,
	145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95,
	28, 139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226,
	71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36,
	17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106,
	197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136,
	120, 163, 236, 249 };

void InitRnd (boolean randomize)
{
}

int Rnd (int max)
{
	return rand() % max;
}

void InitRndT (boolean randomize) {
	if (randomize) {
		rndindex = rand();//time(NULL);
	} else {
		rndindex = 0;
	}
}

int RndT(void) {
	return rndtable[(rndindex++)&0xFF];
}



