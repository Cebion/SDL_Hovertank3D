
#include "IDLIB.H"
#include "IDLIB_SDL.H"

#include <stdint.h>

#define BLANKCHAR	9

int JoyXlow [3], JoyXhigh [3], JoyYlow [3], JoyYhigh [3], buttonflip;

#if NUMPICS>0
pictype	pictable[NUMPICS];
#endif

memptr	grsegs[NUMCHUNKS];
char	needgr[NUMCHUNKS];	// for caching

inputtype playermode[3];

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

////////////////
//
// CalibrateJoy
// Brings up a dialog and has the user calibrate
// either joystick1 or joystick2
//
////////////////

void CalibrateJoy (int joynum)
{
  int stage,dx,dy,xl,yl,xh,yh;
  ControlStruct ctr;

  ExpWin (34,11);


  fontcolor=13;
  CPPrint("Joystick Configuration\n");
  py+=6;
  fontcolor=15;
  PPrint("Hold the joystick in the UPPER LEFT\n");
  PPrint("corner and press a button:");
  stage=15;
  sx=(px+7)/8;
  do				// wait for a button press
  {
    DrawChar (sx,py,stage);
    WaitVBL (3);
    if (++stage==23)
      stage=15;
    ReadJoystick (joynum,&xl,&yl);
    ctr = ControlJoystick(joynum);
    if (keydown[1])
      return;
  } while (ctr.button1!= 1 && ctr.button2!=1);
   DrawChar(sx,py,BLANKCHAR);
  do                  		// wait for the button release
  {
    ctr = ControlJoystick(joynum);
  } while (ctr.button1);
  WaitVBL (4);			// so the button can't bounce

  py+=6;
  PPrint("\nHold the joystick in the LOWER RIGHT\n");
  PPrint("corner and press a button:");
  do				// wait for a button press
  {
    DrawChar (sx,py,stage);
    WaitVBL (3);
    if (++stage==23)
      stage=15;
    ReadJoystick (joynum,&xh,&yh);
    ctr = ControlJoystick(joynum);
    if (keydown[1])
      return;
  } while (ctr.button1!= 1 && ctr.button2!=1);
  DrawChar (sx,py,BLANKCHAR);
  do                  		// wait for the button release
  {
    ctr = ControlJoystick(joynum);
  } while (ctr.button1);

  //
  // figure out good boundaries
  //

  dx=(xh-xl) / 6;
  dy=(yh-yl) / 6;
  JoyXlow[joynum]=xl+dx;
  JoyXhigh[joynum]=xh-dx;
  JoyYlow[joynum]=yl+dy;
  JoyYhigh[joynum]=yh-dy;
  if (joynum==1)
    playermode[1]=joystick1;
  else
    playermode[1]=joystick2;

  py+=6;
  PPrint ("\n(F)ire or (A)fterburn with B1 ?");
  char ch = PGet();
  if ( ch == 'A' || ch == 'a')
    buttonflip = 1;
  else
    buttonflip = 0;
}

/////////////////////////////
//
// print a representation of the scan code key
//
////////////////////////////
void printscan (int sc)
{
 char static chartable[128] =
 {'?','?','1','2','3','4','5','6','7','8','9','0','-','+','?','?',
  'Q','W','E','R','T','Y','U','I','O','P','[',']','|','?','A','S',
  'D','F','G','H','J','K','L',';','"','?','?','?','Z','X','C','V',
  'B','N','M',',','.','/','?','?','?','?','?','?','?','?','?','?',
  '?','?','?','?','?','?','?','?', 15,'?','-', 21,'5', 17,'+','?',
   19,'?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
  '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
  '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?'};
 char str[512];

 sc = sc & 0x7f;

 if (sc==1)
   PPrint ("ESC");
 else if (sc==0xe)
   PPrint ("BKSP");
 else if (sc==0xf)
   PPrint ("TAB");
 else if (sc==0x1d)
   PPrint ("CTRL");
 else if (sc==0x2A)
   PPrint ("LSHIFT");
 else if (sc==0x39)
   PPrint ("SPACE");
 else if (sc==0x3A)
   PPrint ("CAPSLK");
 else if (sc>=0x3b && sc<=0x44)
 {
   char str[3];
   PPrint ("F");
   itoa (sc-0x3a,str,10);
   PPrint (str);
 }
 else if (sc==0x57)
   PPrint ("F11");
 else if (sc==0x59)
   PPrint ("F12");
 else if (sc==0x46)
   PPrint ("SCRLLK");
 else if (sc==0x1c)
   PPrint ("ENTER");
 else if (sc==0x36)
   PPrint ("RSHIFT");
 else if (sc==0x37)
   PPrint ("PRTSC");
 else if (sc==0x38)
   PPrint ("ALT");
 else if (sc==0x47)
   PPrint ("HOME");
 else if (sc==0x49)
   PPrint ("PGUP");
 else if (sc==0x4f)
   PPrint ("END");
 else if (sc==0x51)
   PPrint ("PGDN");
 else if (sc==0x52)
   PPrint ("INS");
 else if (sc==0x53)
   PPrint ("DEL");
 else if (sc==0x45)
   PPrint ("NUMLK");
 else if (sc==0x48)
   PPrint ("UP");
 else if (sc==0x50)
   PPrint ("DOWN");
 else if (sc==0x4b)
   PPrint ("LEFT");
 else if (sc==0x4d)
   PPrint ("RIGHT");
 else
 {
   str[0]=chartable[sc];
   str[1]=0;
   PPrint (str);
 }
}

/////////////////////////////
//
// calibratekeys
//
////////////////////////////
void calibratekeys (void)
{
  char ch;
  int hx,hy,i,select,_new;

  ExpWin (22,12);
  fontcolor=13;
  CPPrint ("Keyboard Configuration");
  fontcolor=15;
  PPrint ("\n1 north");
  PPrint ("\n2 east");
  PPrint ("\n3 south");
  PPrint ("\n4 west");
  PPrint ("\n5 button1");
  PPrint ("\n6 button2");
  PPrint ("\nModify which action:");
  hx=(px+7)/8;
  hy=py;
  for (i=0;i<4;i++)
  {
    px=pxl+8*12;
    py=pyl+10*(1+i);
    PPrint(":");
    printscan (key[i*2]);
  }
  px=pxl+8*12;
  py=pyl+10*5;
  PPrint(":");
  printscan (keyB1);
  px=pxl+8*12;
  py=pyl+10*6;
  PPrint(":");
  printscan (keyB2);

  do
  {
    px=hx*8;
    py=hy;
    DrawChar (hx,hy,BLANKCHAR);
    ch=PGet() % 256;
    if (ch<'1' || ch>'6')
      continue;
    select = ch - '1';
    DrawPchar (ch);
    PPrint ("\nPress the new key:");
    ClearKeys ();
    _new=-1;
    while (!keydown[++_new])
      if (_new==0x79)
	_new=-1;
      else if (_new==0x29)
	_new++;				// skip STUPID left shifts!
    Bar(leftedge,py,22,10,0xff);
    if (select<4)
      key[select*2]=_new;
    if (select==4)
      keyB1=_new;
    if (select==5)
      keyB2=_new;
    px=pxl+8*12;
	py=pyl+(select+1)*10;
    Bar(px/8,py,9,10,0xff);
    PPrint (":");
    printscan (_new);
    ClearKeys ();
    ch='0';				// so the loop continues
  } while (ch>='0' && ch<='9');
  playermode[1]=keyboard;
}


/*
============================================================================

			MID LEVEL GRAPHIC ROUTINES

============================================================================
*/

#define DRAWCHAR(x,y,n) DrawChar(x,(y)*8,n)

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
	DrawPchar(string[count]);
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
   DrawPchar ('_');
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

