#ifndef _SAA1099_H
#define _SAA1099_H

#include "sndcommon.h"

#define SAA_OFF		0
#define SAA_MONO	1
#define	SAA_STEREO	2

typedef struct {
	unsigned lev:1;
	unsigned freqEn:1;
	unsigned noizEn:1;
	int ampLeft;
	int ampRight;
	int octave;
	int freq;
	int period;
	int count;
} saaChan;

typedef struct {
	struct {
		unsigned update:1;
		unsigned extCLK:1;
		unsigned invRight:1;
		int form;
	} buf;
	unsigned enable:1;
	unsigned invRight:1;	// inverse right channel
	unsigned lowRes:1;	// 3-bit env control;
	unsigned extCLK:1;	// external frq control (8MHz)
	unsigned busy:1;
	int form;
	int period;
	int count;
	int pos;
	unsigned char vol;
} saaEnv;

typedef struct {
	unsigned lev:1;
	int period;
	int count;
	int pos;
} saaNoise;

typedef struct {
	unsigned enabled:1;	// options : present/not present
	unsigned mono:1;
	unsigned off:1;		// software control : Reg 1C bit 0
	int curReg;
	saaChan chan[6];	// 6 8-bit channels
	saaNoise noiz[2];	// 2 noize channels (mix ch 0,1,2 & 3,4,5; generators from ch 0,3)
	saaEnv env[2];		// 2 envelope (aff ch 2 & 5, generators from ch 1, 4)
} saaChip;

saaChip* saaCreate();
void saaDestroy(saaChip*);
void saaReset(saaChip*);
int saaWrite(saaChip*, int, unsigned char);

void saaSync(saaChip*, int);
sndPair saaGetVolume(saaChip*);

#endif
