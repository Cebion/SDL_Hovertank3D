
#include "JM_SB.H"
#include "IDLIB.H"

#define SAMPLERATE 44100
#define SAMPLES (SAMPLERATE/70)
const float SAMPLESTEP = 1.f/SAMPLERATE;

static void MyAudioCallback(void *userdata, Uint8 *stream, int len);

SDL_Window * window;
int surfaceWidth, surfaceHeight;

int processedevents = 0;

int NBKscan,NBKascii;
char keydown[128];
char key[8],keyB1,keyB2;
long highscore;
int level,bestlevel;

#define DRAWCHAR(x,y,n) DrawChar(x,(y)*8,n)

memptr soundseg;

void IDLIBC_GL_Init();

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

	IDLIBC_GL_Init();

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

void WaitVBL(int num)
{
	int starttime = inttime;
	VideoSync();
	num*=2;
	while (inttime-starttime < num) {}
	if (processedevents)
	{
		processedevents = 0;
	}
	else
	{
		IN_Poll();
	}
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
			for (int i = 0; i < SAMPLES/4; i++)
			{
				int sample = (1&(int)(SPKtime*SPKfreq*2))*65535 - 32768;
				SPKtime += SAMPLESTEP;
				*stream16 += sample;
				stream16++;
			}
		}
		else
		{
			stream16+=SAMPLES;
		}
	}
}
