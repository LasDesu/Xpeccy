#ifndef _XPECTR
#define _XPECTR

#ifdef __cplusplus
extern "C" {
#endif

#include "cpu.h"
#include "memory.h"
#include "video.h"
#include "input.h"
#include "tape.h"
#include "bdi.h"
#include "ayym.h"		// AY/YM/TS sound
#include "saa1099.h"
#include "soundrive.h"		// covox/soundrive
#include "gs.h"
#include "hdd.h"
#include "sdcard.h"

// hw reset rompage
#define	RES_DEFAULT	0
#define	RES_48		1
#define	RES_128		2
#define	RES_DOS		3
#define	RES_SHADOW	4

typedef struct {
	unsigned char l;
	unsigned char h;
	unsigned char x;
} DMAaddr;

typedef struct {
	int fetches;
	int frmSize;
	unsigned char* frmData;
} RZXFrame;

typedef struct {
	unsigned char flag;
	unsigned char page;
} memEntry;

struct ZXComp {
	unsigned brk:1;			// breakpoint
	unsigned debug:1;		// dont' do breakpoints
	unsigned frmStrobe:1;		// new frame started
	unsigned intStrobe:1;		// int front
	unsigned nmiRequest:1;		// Magic button pressed
	unsigned beeplev:1;		// beeper level
	unsigned rzxPlay:1;		// rzx is playing
	unsigned firstRun:1;
	unsigned dosen:1;

	unsigned scrpWait:1;		// scorpion wait mode
	unsigned contMem:1;		// contended mem
	unsigned contIO:1;		// contended IO

	struct HardWare *hw;
	CPU* cpu;
	Memory* mem;
	Video* vid;
	Keyboard* keyb;
	Joystick* joy;
	Mouse* mouse;
	Tape* tape;
	BDI* bdi;
	IDE* ide;
	SDCard* sdc;
	GSound* gs;
	TSound* ts;
	saaChip* saa;
	SDrive* sdrv;
	int rzxSize;
	RZXFrame* rzxData;
	unsigned long rzxFrame;
	unsigned int rzxPos;
	int rzxFetches;
	float cpuFrq;
	unsigned char intVector;

	unsigned long tickCount;
	int nsPerTick;
	int nsPerFrame;
//	unsigned long nsCount;

	memEntry memMap[16];		// memory map for ATM2, PentEvo
	unsigned char prt0;		// 7ffd value
	unsigned char prt1;		// extend port value
	unsigned char prt2;		// scorpion ProfROM layer (0..3)

	unsigned char brkIOMap[0x10000];	// brk by port

	unsigned short padr;
	unsigned char pval;

	struct {
		unsigned char evoBF;		// PentEvo rw ports
		unsigned char evo2F;
		unsigned char evo4F;
		unsigned char evo6F;
		unsigned char evo8F;
		int hiTrig;			// trigger for hi-low byte in IDE
		unsigned char blVer[16];	// bootloader info
		unsigned char bcVer[16];	// baseconf info
	} evo;
	struct {
		DMAaddr src;
		DMAaddr dst;
		unsigned char len;
		unsigned char num;
	} dma;
	struct {
		int flag;
		unsigned short tsMapAdr;	// adr for altera mapping
		unsigned char Page0;
		unsigned char p00af;		// ports to be updated from next line
		unsigned char p01af;
		unsigned char p02af;
		unsigned char p03af;
		unsigned char p04af;
		unsigned char p05af;
		unsigned char p07af;
		unsigned char pwr_up;		// 1 on 1st run, 0 after reading 00AF
		unsigned char vdos;
	} tsconf;
	CMOS cmos;
	int resbank;			// rompart active after reset
	int tapCount;
};

typedef struct ZXComp ZXComp;

#include "hardware/hardware.h"

ZXComp* zxCreate();
void zxDestroy(ZXComp*);
void zxReset(ZXComp*,int);
//void zxOut(ZXComp*,Z80EX_WORD,Z80EX_BYTE);
int zxExec(ZXComp*);
void zxSetFrq(ZXComp*,float);
void zxSetLayout(ZXComp*, int, int, int, int, int, int, int, int, int);
void zxSetHardware(ZXComp*,const char*);

// read-write cmos
unsigned char cmsRd(ZXComp*);
void cmsWr(ZXComp*,unsigned char);

void rzxClear(ZXComp*);

#ifdef __cplusplus
}
#endif

#endif
