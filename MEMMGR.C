
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include "MEMMGR.H"



unsigned totalmem;		// total paragraphs available


//==========================================================================

//
// public prototypes
//

#define MAX_TRACKED 1024
static memptr allocation[MAX_TRACKED];

void MMStartup (void) { }

void MMShutdown (void) {
	MMValidateAll();
}

#ifdef GUARDBYTES
void _MMGetPtr (memptr *baseptr,long size, const char * fname, int line)
{
	assert(size>0);
	size += GUARDBYTES*2+8;
	unsigned char * bytes = (unsigned char*)malloc(size);
	memset(bytes, 0xFFFFFFFF, size);
	*(int*)(bytes+GUARDBYTES) = size;
	*(int*)(bytes+size-GUARDBYTES-4) = size;
	*baseptr = bytes + GUARDBYTES + 4;

	int i = 0;
	for (i = 0; i < MAX_TRACKED; i++)
	{
		if (!allocation[i])
		{
			allocation[i] = *baseptr;
			break;
		}
	}
	assert(i < MAX_TRACKED);
}
#else
void MMGetPtr (memptr *baseptr,long size)
{
	*baseptr = malloc(size);
}
#endif

void MMFreePtr (memptr *baseptr)
{
#ifdef GUARDBYTES
	MMValidatePtr(baseptr);

	unsigned char * bytes = (unsigned char*)*baseptr;
	bytes -= GUARDBYTES+4;

	int i;
	for (i = 0; i < MAX_TRACKED; i++)
	{
		if (allocation[i] == *baseptr)
		{
			allocation[i] = 0;
			break;
		}
	}
	assert(i < MAX_TRACKED);

	free(bytes);
	*baseptr = 0;
#else
	free(*baseptr);
	*baseptr = 0;
#endif
}

void MMSetPurge (memptr *baseptr, int purge) { }

void MMSortMem (void) { }

unsigned MMUnusedMemory (void) { return 0; }

unsigned MMTotalFree (void) { return 0; }


#ifdef GUARDBYTES
void MMValidatePtr (memptr *baseptr)
{
	unsigned char * bytes = (unsigned char*)*baseptr;
	bytes -= GUARDBYTES+4;

	int size = *(int*)(bytes + GUARDBYTES);
	assert(size > GUARDBYTES*2);
	int size2 = *(int*)(bytes+size-GUARDBYTES-4);
	assert(size==size2);

	for (int i = 0; i < GUARDBYTES; i++)
	{
		assert(bytes[i] == 0xff);
		assert(bytes[size-i-1] == 0xff);
	}
}

void MMValidateAll()
{
	for (int i = 0; i < MAX_TRACKED; i++)
	{
		if (allocation[i])
		{
			MMValidatePtr(&allocation[i]);
		}
	}
}
#endif
