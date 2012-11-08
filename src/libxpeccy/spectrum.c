#include <stdio.h>
#include <stdlib.h>
//#include <assert.h>

#include "spectrum.h"

double ltk;
int res1 = 0;
int res2 = 0;
int res3 = 0;	// tick in op, wich has last OUT/MWR (and vidSync)
int res4 = 0;	// save last res3 (vidSync on OUT/MWR process do res3-res4 ticks)
int res5 = 0;	// ticks ated by slow mem?
int vflg = 0;
Z80EX_WORD pcreg;

/*
 * ProfROM switch:
 * 	page 2,6,10,14
 * 	PC = E4B5 : ld l,(hl)
 * 	HL = 0110..0113
 * ProfROM table :
 *  adr | 0 1 2 3 <- current layer
 * -----+---------
 * 0110 | 0 1 2 3 <- result layers
 * 0111 | 3 3 3 2
 * 0112 | 2 2 0 1
 * 0113 | 1 0 1 0
 */

unsigned char ZSLays[4][4] = {
	{0,1,2,3},
	{3,3,3,2},
	{2,2,0,1},
	{1,0,1,0}
};

// layout for +2a extend memory mode

unsigned char plus2Lays[4][4] = {
	{0,1,2,3},
	{4,5,6,7},
	{4,5,6,3},
	{4,7,6,3}
};

#include <assert.h>

// port decoding tables

int zxGetPort(ZXComp* comp, Z80EX_WORD port) {
	switch (comp->hw->type) {
		case HW_ZX48:
			if ((port & 0x0001) == 0x0000) return 0xfe;
			if (comp->dosen & 1) {
				if ((port & 0x0083) == 0x0083) return 0xff;
				if ((port & 0x00e3) == 0x0003) return 0x1f;
				if ((port & 0x00e3) == 0x0023) return 0x3f;
				if ((port & 0x00e3) == 0x0043) return 0x5f;
				if ((port & 0x00e3) == 0x0063) return 0x7f;
			} else {
				if ((port & 0x0021) == 0x0001) return 0x1f;
			}
			break;
		case HW_PENT:
			if ((port & 0x0003) == 0x0002) return 0xfe;
			if ((port & 0x00ff) == 0x001f) return 0x1f;
			if ((port & 0x8002) == 0x0000) return 0x7ffd;
			if ((port & 0xc002) == 0x8000) return 0xbffd;
			if ((port & 0xc002) == 0xc000) return 0xfffd;
			if (comp->dosen & 1) {
				if ((port & 0x0083) == 0x0083) return 0xff;
				if ((port & 0x00e3) == 0x0003) return 0x1f;
				if ((port & 0x00e3) == 0x0023) return 0x3f;
				if ((port & 0x00e3) == 0x0043) return 0x5f;
				if ((port & 0x00e3) == 0x0063) return 0x7f;
			} else {
				if ((port & 0x05a3) == 0x0083) return 0xfadf;
				if ((port & 0x05a3) == 0x0183) return 0xfbdf;
				if ((port & 0x05a3) == 0x0583) return 0xffdf;
			}
			break;
		case HW_P1024:
			if ((port & 0x0003) == 0x0002) return 0xfe;
			if ((port & 0xf008) == 0xe008) return 0xeff7;
			if ((port & 0x8002) == 0x0000) return 0x7ffd;
			if ((port & 0xc002) == 0x8000) return 0xbffd;
			if ((port & 0xc002) == 0xc000) return 0xfffd;
			if (comp->dosen & 1) {
				if ((port & 0x0083) == 0x0083) return 0xff;
				if ((port & 0x00e3) == 0x0003) return 0x1f;
				if ((port & 0x00e3) == 0x0023) return 0x3f;
				if ((port & 0x00e3) == 0x0043) return 0x5f;
				if ((port & 0x00e3) == 0x0063) return 0x7f;
			} else {
				if ((port & 0x05a3) == 0x0083) return 0xfadf;
				if ((port & 0x05a3) == 0x0183) return 0xfbdf;
				if ((port & 0x05a3) == 0x0583) return 0xffdf;
			}
			break;
		case HW_SCORP:
			if ((port & 0x0023) == 0x0022) return 0xfe;
			if ((port & 0xc023) == 0x0021) return 0x1ffd;
			if ((port & 0xc023) == 0x4021) return 0x7ffd;
			if ((port & 0xc023) == 0x8021) return 0xbffd;
			if ((port & 0xc023) == 0xc021) return 0xfffd;
			if ((port & 0x0023) == 0x0001) return 0xdd;
			if (comp->dosen & 1) {
				if ((port & 0x0083) == 0x0083) return 0xff;
				if ((port & 0x00e3) == 0x0003) return 0x1f;
				if ((port & 0x00e3) == 0x0023) return 0x3f;
				if ((port & 0x00e3) == 0x0043) return 0x5f;
				if ((port & 0x00e3) == 0x0063) return 0x7f;
			} else {
				if ((port & 0x00ff) == 0x001f) return 0x1f;
				if ((port & 0x0523) == 0x0003) return 0xfadf;
				if ((port & 0x0523) == 0x0103) return 0xfbdf;
				if ((port & 0x0523) == 0x0503) return 0xffdf;
			}
			break;
		case HW_PLUS2:
			if ((port & 0x0003) == 0x0002) return 0xfe;
			if ((port & 0xc002) == 0x4000) return 0x7ffd;
			if ((port & 0xf002) == 0x1000) return 0x1ffd;
			if ((port & 0xc002) == 0x8000) return 0xbffd;
			if ((port & 0xc002) == 0xc000) return 0xfffd;
			break;
		case HW_PLUS3:
			if ((port & 0x0003) == 0x0002) return 0xfe;
			if ((port & 0xc002) == 0x4000) return 0x7ffd;
			if ((port & 0xf002) == 0x1000) return 0x1ffd;
			if ((port & 0xf002) == 0x0000) return 0x0ffd;
			if ((port & 0xf002) == 0x2000) return 0x2ffd;
			if ((port & 0xf002) == 0x3000) return 0x3ffd;
			if ((port & 0xc002) == 0x8000) return 0xbffd;
			if ((port & 0xc002) == 0xc000) return 0xfffd;
			break;
		case HW_ATM1:
			if ((port & 0x0007) == 0x0006) return 0xfe;	// (to read: a8..15, to write: a3.5.6.7)
			if ((port & 0x8202) == 0x8000) return 0xfdfd;	// mem.ext
			if ((port & 0x8202) == 0x0200) return 0x7ffd;	// mem
			if ((port & 0x8202) == 0x0000) return 0x7dfd;	// digital.rd / palete.wr
			if ((port & 0xc202) == 0x8200) return 0xbffd;	// ay
			if ((port & 0xc202) == 0xc200) return 0xfffd;
			if ((port & 0x0007) == 0x0002) return 0xfa;	// interface port
			if ((port & 0x0007) == 0x0003) return 0xfb;	// digital.wr/printer
			if (comp->dosen & 1) {
				if ((port & 0x009f) == 0x009f) return 0xff;				// bdi: ff
				if ((port & 0x00ff) == 0x001f) return 0x1f;
				if ((port & 0x00ff) == 0x003f) return 0x3f;
				if ((port & 0x00ff) == 0x005f) return 0x5f;
				if ((port & 0x00ff) == 0x007f) return 0x7f;
			}
			break;
		case HW_ATM2:
			if ((port & 0x0007) == 0x0006) return 0xfe;
			if ((port & 0x0007) == 0x0002) return 0xfa;	// interface port
			if ((port & 0x0007) == 0x0003) return 0xfb;	// covox
			if ((port & 0x8202) == 0x0200) return 0x7ffd;	// mem.main
			if ((port & 0x8202) == 0x0000) return 0x7dfd;	// digital.rd
			if ((port & 0xc202) == 0x8200) return 0xbffd;	// ay
			if ((port & 0xc202) == 0xc200) return 0xfffd;
			if (comp->dosen & 1) {
				if ((port & 0x009f) == 0x009f) return 0xff;				// bdi: ff
				if ((port & 0x00ff) == 0x001f) return 0x1f;
				if ((port & 0x00ff) == 0x003f) return 0x3f;
				if ((port & 0x00ff) == 0x005f) return 0x5f;
				if ((port & 0x00ff) == 0x007f) return 0x7f;
				if ((port & 0x009f) == 0x0097) return 0xf7;				// F7: mem.manager (a14.15)
				if ((port & 0x009f) == 0x0017) return 0x77;				// 77: system (a8.a9.a14)
				if ((port & 0x001f) == 0x000f) return 0xef;				// EF: hdd (a8..11)
			}
			break;
		default:
			printf("zxGetPort : unknown hardware type %i\n",comp->hw->type);
			assert(0);
			break;
	}
	return 0xff;
}

#define PRT0	comp->prt0
#define	PRT1	comp->prt1
#define	PRT2	comp->prt2

void atmSetBank(ZXComp* comp, int bank, unsigned char val) {
	unsigned char page = ~val & 0x3f;	// inversed b0..5
	if (val & 0x80) page = (page & 0x38) | (PRT0 & 7);	// mix with 7FFD bank
	if (~val & 0x40) page &= 3;
	memSetBank(comp->mem,bank,(val & 0x40) ? MEM_RAM : MEM_ROM, page);
}

void zxMapMemory(ZXComp* comp) {
	Z80EX_BYTE rp;
	int adr;
	switch (comp->hw->type) {
		case HW_ZX48:
			memSetBank(comp->mem,MEM_BANK0,MEM_ROM,(comp->dosen) ? 3 : 1);
			memSetBank(comp->mem,MEM_BANK3,MEM_RAM,0);
			break;
		case HW_PENT:
			memSetBank(comp->mem,MEM_BANK0,MEM_ROM,((comp->dosen & 1) << 1) | ((PRT0 & 0x10) >> 4));
			memSetBank(comp->mem,MEM_BANK3,MEM_RAM,(PRT0 & 7) | ((PRT0 & 0xc0) >> 3));
			break;
		case HW_P1024:
			if (PRT1 & 8) {
				memSetBank(comp->mem,MEM_BANK0,MEM_RAM,0);
			} else {
				memSetBank(comp->mem,MEM_BANK0,MEM_ROM,((comp->dosen & 1) << 1) | ((PRT0 & 0x10) >> 4));
			}
			memSetBank(comp->mem,MEM_BANK3,MEM_RAM,(PRT0 & 7) | ((PRT1 & 4) ? 0 : ((PRT0 & 0x20) | ((PRT0 & 0xc0) >> 3))));
			break;
		case HW_SCORP:
			if (PRT1 & 0x01) {
				memSetBank(comp->mem,MEM_BANK0,MEM_RAM,0);
			} else {
				rp = (PRT1 & 0x02) ? 2 : (((comp->dosen & 1) << 1) | ((PRT0 & 0x10) >> 4));
				rp |= ((PRT2 & 3) << 2);
				memSetBank(comp->mem,MEM_BANK0,MEM_ROM,rp);
			}
			memSetBank(comp->mem,MEM_BANK3,MEM_RAM,(PRT0 & 7) | ((PRT1 & 0x10) >> 1) | ((PRT1 & 0xc0) >> 2));
			break;
		case HW_PLUS3:
		case HW_PLUS2:
			if (PRT1 & 1) {
				// extend mem mode
				rp = ((PRT1 & 0x06) >> 1);	// b1,2 of 1ffd
				memSetBank(comp->mem,MEM_BANK0,MEM_RAM,plus2Lays[rp][0]);
				memSetBank(comp->mem,MEM_BANK1,MEM_RAM,plus2Lays[rp][1]);
				memSetBank(comp->mem,MEM_BANK2,MEM_RAM,plus2Lays[rp][2]);
				memSetBank(comp->mem,MEM_BANK3,MEM_RAM,plus2Lays[rp][3]);
			} else {
				// normal mem mode
				memSetBank(comp->mem,MEM_BANK0,MEM_ROM,((PRT0 & 0x10) >> 4) | ((PRT1 & 0x04) >> 1));
				memSetBank(comp->mem,MEM_BANK1,MEM_RAM,5);
				memSetBank(comp->mem,MEM_BANK2,MEM_RAM,2);
				memSetBank(comp->mem,MEM_BANK3,MEM_RAM,PRT0 & 7);
			}
			break;
		case HW_ATM2:
			if (~PRT1 & 1) {			// pen = 0: last rom page in every bank
				memSetBank(comp->mem,MEM_BANK0,MEM_ROM,3);
				memSetBank(comp->mem,MEM_BANK1,MEM_ROM,3);
				memSetBank(comp->mem,MEM_BANK2,MEM_ROM,3);
				memSetBank(comp->mem,MEM_BANK3,MEM_ROM,3);
			} else {
				adr = (PRT0 & 0x10) ? 4 : 0;
				atmSetBank(comp,MEM_BANK0,comp->memMap[adr]);
				atmSetBank(comp,MEM_BANK1,comp->memMap[adr+1]);
				atmSetBank(comp,MEM_BANK2,comp->memMap[adr+2]);
				atmSetBank(comp,MEM_BANK3,comp->memMap[adr+3]);

			}
			break;
	}
}

Z80EX_BYTE memrd(Z80EX_CONTEXT* cpu,Z80EX_WORD adr,int m1,void* ptr) {
	Z80EX_BYTE res;
	ZXComp* comp = (ZXComp*)ptr;
	if ((comp->hw->type == HW_SCORP) && ((comp->mem->crom & 3) == 2) && ((adr & 0xfff3) == 0x0100)) {
		comp->prt2 = ZSLays[(adr & 0x000c) >> 2][comp->prt2 & 3] & comp->mem->profMask;
		zxMapMemory(comp);
	}
	if (m1 == 1) {
		if (comp->rzxPlay) comp->rzxFetches--;
		if (comp->bdi->fdc->type == FDC_93) {
			if ((comp->dosen == 0) && ((adr & 0xff00) == 0x3d00) && (comp->prt0 & 0x10)) {
				comp->dosen = 1;
				zxMapMemory(comp);
			}
			if ((comp->dosen == 1) && (adr > 0x3fff)
					&& !((comp->hw->type == HW_ATM2) && (~PRT1 & 2))		// ATM2 cpm mode - don't off dosen
					) {
				comp->dosen = 0;
				zxMapMemory(comp);
			}
		}
	}
	res3 = res2 + z80ex_op_tstate(cpu);
	vflg |= vidSync(comp->vid,comp->dotPerTick * (res3 - res4));
	res4 = res3;
	if (((adr & 0xc000) == 0x4000) && (comp->flags & ZX_CONTMEM)) {
		res5 = vidGetWait(comp->vid);
		if (res5 != 0) {
			vflg |= vidSync(comp->vid, comp->dotPerTick * res5);
			res1 += res5;
		}
	}
	res = memRd(comp->mem,adr);
	return res;
}

// NOTE (2012.07.14)
// real OUT or MWR must be AFTER vidSync()

void memwr(Z80EX_CONTEXT* cpu, Z80EX_WORD adr, Z80EX_BYTE val, void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	res3 = res2 + z80ex_op_tstate(cpu);
	vflg |= vidSync(comp->vid,comp->dotPerTick * (res3 - res4));
	res4 = res3;
	if (((adr & 0xc000) == 0x4000) && (comp->flags & ZX_CONTMEM)) {
		res5 = vidGetWait(comp->vid);
		if (res5 != 0) {
			vflg |= vidSync(comp->vid,comp->dotPerTick * res5);
			res1 += res5;
		}
	}
	memWr(comp->mem,adr,val);
	if (comp->mem->flags & MEM_BRK_WRITE) {
		comp->flags |= ZX_BREAK;
	}
}

Z80EX_BYTE iord(Z80EX_CONTEXT* cpu, Z80EX_WORD port, void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	Z80EX_BYTE res = 0xff;
// video sync
	res3 = res2 + z80ex_op_tstate(cpu);
	vflg |= vidSync(comp->vid,comp->dotPerTick * (res3 - res4));
	res4 = res3;
	if (comp->flags & ZX_CONTIO) {
		res5 = vidGetWait(comp->vid);
		if (res5 != 0) {
			vflg |= vidSync(comp->vid,comp->dotPerTick * res5);
			res1 += res5;
		}
	}

	gsSync(comp->gs,comp->gsCount);
	comp->gsCount = 0;
// play rzx
	if (comp->rzxPlay) {
		if (comp->rzxPos < comp->rzxData[comp->rzxFrame].frmSize) {
			res = comp->rzxData[comp->rzxFrame].frmData[comp->rzxPos];
			comp->rzxPos++;
			return res;
		} else {
			return 0xff;
		}
	}
// request to external devices
	if (ideIn(comp->ide,port,&res,comp->dosen & 1)) return res;
	if (gsIn(comp->gs,port,&res) == GS_OK) return res;
// request to internal devices
	int bdiz = ((comp->dosen & 1) && (comp->bdi->fdc->type == FDC_93)) ? 1 : 0;
	int ptype = zxGetPort(comp,port);
	switch (comp->hw->type) {
		case HW_ZX48:
			switch (ptype) {
				case 0x1f: res = bdiz ? bdiIn(comp->bdi,FDC_STATE) : joyInput(comp->joy); break;
				case 0x3f: res = bdiz ? bdiIn(comp->bdi,FDC_TRK) : 0xff; break;
				case 0x5f: res = bdiz ? bdiIn(comp->bdi,FDC_SEC) : 0xff; break;
				case 0x7f: res = bdiz ? bdiIn(comp->bdi,FDC_DATA) : 0xff; break;
				case 0xff: res = bdiz ? bdiIn(comp->bdi,BDI_SYS) : 0xff; break;
				case 0xfe: res = keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00); break;
			}
			break;
		case HW_PENT:
			switch (ptype) {
				case 0x1f: res = bdiz ? bdiIn(comp->bdi,FDC_STATE) : joyInput(comp->joy); break;
				case 0x3f: res = bdiz ? bdiIn(comp->bdi,FDC_TRK) : 0xff; break;
				case 0x5f: res = bdiz ? bdiIn(comp->bdi,FDC_SEC) : 0xff; break;
				case 0x7f: res = bdiz ? bdiIn(comp->bdi,FDC_DATA) : 0xff; break;
				case 0xff: res = bdiz ? bdiIn(comp->bdi,BDI_SYS) : 0xff; break;
				case 0xfe: res = keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00); break;
				case 0xfadf: res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->buttons : 0xff; break;
				case 0xfbdf: res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->xpos : 0xff; break;
				case 0xffdf: res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->ypos : 0xff; break;
				case 0xfffd: res = tsIn(comp->ts,port); break;
			}
			break;
		case HW_P1024:
			switch (ptype) {
				case 0x1f: res = bdiz ? bdiIn(comp->bdi,FDC_STATE) : 0xff; break;
				case 0x3f: res = bdiz ? bdiIn(comp->bdi,FDC_TRK) : 0xff; break;
				case 0x5f: res = bdiz ? bdiIn(comp->bdi,FDC_SEC) : 0xff; break;
				case 0x7f: res = bdiz ? bdiIn(comp->bdi,FDC_DATA) : 0xff; break;
				case 0xff: res = bdiz ? bdiIn(comp->bdi,BDI_SYS) : 0xff; break;
				case 0xfe: res = keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00); break;
				case 0xfadf: res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->buttons : 0xff; break;
				case 0xfbdf: res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->xpos : 0xff; break;
				case 0xffdf: res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->ypos : 0xff; break;
				case 0xfffd: res = tsIn(comp->ts,port); break;
			}
			break;
		case HW_SCORP:
			switch (ptype) {
				case 0x1f: res = bdiz ? bdiIn(comp->bdi,FDC_STATE) : 0xff; break;
				case 0x3f: res = bdiz ? bdiIn(comp->bdi,FDC_TRK) : 0xff; break;
				case 0x5f: res = bdiz ? bdiIn(comp->bdi,FDC_SEC) : 0xff; break;
				case 0x7f: res = bdiz ? bdiIn(comp->bdi,FDC_DATA) : 0xff; break;
				case 0xff: res = bdiz ? bdiIn(comp->bdi,BDI_SYS) : 0xff; break;
				case 0xfe: res = keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00); break;
				case 0x1ffd: zxSetFrq(comp,3.5); break;
				case 0x7ffd: zxSetFrq(comp,7.0); break;
				case 0xfadf: res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->buttons : 0xff; break;
				case 0xfbdf: res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->xpos : 0xff; break;
				case 0xffdf: res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->ypos : 0xff; break;
				case 0xfffd: res = tsIn(comp->ts,port); break;
			}
			break;
		case HW_PLUS2:
			switch (ptype) {
				case 0xfe: res = keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00); break;
				case 0xfffd: res = tsIn(comp->ts,port); break;
			}
			break;
		case HW_PLUS3:
			switch (ptype) {
				case 0xfe: res = keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00); break;
				case 0x2ffd: res = fdcRd(comp->bdi->fdc,FDC_STATE); break;
				case 0x3ffd: res = fdcRd(comp->bdi->fdc,FDC_DATA); break;
				case 0xfffd: res = tsIn(comp->ts,port); break;
			}
			break;
		case HW_ATM2:
			switch (ptype) {
				case 0x1f: res = bdiz ? bdiIn(comp->bdi,FDC_STATE) : 0xff; break;
				case 0x3f: res = bdiz ? bdiIn(comp->bdi,FDC_TRK) : 0xff; break;
				case 0x5f: res = bdiz ? bdiIn(comp->bdi,FDC_SEC) : 0xff; break;
				case 0x7f: res = bdiz ? bdiIn(comp->bdi,FDC_DATA) : 0xff; break;
				case 0xef: res = 0xff; break;
				case 0xff: res = bdiz ? bdiIn(comp->bdi,BDI_SYS) : 0xff; break;
				case 0xfe: res = keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00); break;
				case 0x7ffd: res = 0xc0; break;
				case 0x7dfd: res = 0xff; break;
				default:
					printf("ATM2: in %.4X (%.4X)\n",port,ptype);
					assert(0);
					break;
			}
			break;
		default:
			printf("iord: unknown hardware id %i\n",comp->hw->type);
			assert(0);
			break;
	}
	return res;
}

void zxOut(ZXComp *comp, Z80EX_WORD port, Z80EX_BYTE val) {
	gsSync(comp->gs,comp->gsCount);
	comp->gsCount = 0;
// request to external devices
	if (ideOut(comp->ide,port,val,comp->dosen & 1)) return;
	if (gsOut(comp->gs,port,val) == GS_OK) return;
// request to internal devices
	int ptype = zxGetPort(comp,port);
	int bdiz = ((comp->dosen & 1) && (comp->bdi->fdc->type == FDC_93)) ? 1 : 0;
	int adr = 0;
	switch (comp->hw->type) {
		case HW_ZX48:
			switch (ptype) {
				case 0x1f: if (bdiz) bdiOut(comp->bdi,FDC_COM,val); break;
				case 0x3f: if (bdiz) bdiOut(comp->bdi,FDC_TRK,val); break;
				case 0x5f: if (bdiz) bdiOut(comp->bdi,FDC_SEC,val); break;
				case 0x7f: if (bdiz) bdiOut(comp->bdi,FDC_DATA,val); break;
				case 0xff: if (bdiz) bdiOut(comp->bdi,BDI_SYS,val); break;
				case 0xfe:
					comp->vid->nextbrd = val & 0x07;
					if (!(comp->vid->flags & VID_BORDER_4T)) comp->vid->brdcol = val & 0x07;
					comp->beeplev = val & 0x10;
					comp->tape->outsig = (val & 0x08) ? 1 : 0;
					break;
			}
			break;
		case HW_PENT:
			sdrvOut(comp->sdrv,port & 0x00ff,val);
			switch (ptype) {
				case 0x1f: if (bdiz) bdiOut(comp->bdi,FDC_COM,val); break;
				case 0x3f: if (bdiz) bdiOut(comp->bdi,FDC_TRK,val); break;
				case 0x5f: if (bdiz) bdiOut(comp->bdi,FDC_SEC,val); break;
				case 0x7f: if (bdiz) bdiOut(comp->bdi,FDC_DATA,val); break;
				case 0xff: if (bdiz) bdiOut(comp->bdi,BDI_SYS,val); break;
				case 0xfe:
					comp->vid->nextbrd = val & 0x07;
					if (!(comp->vid->flags & VID_BORDER_4T)) comp->vid->brdcol = val & 0x07;
					comp->beeplev = val & 0x10;
					comp->tape->outsig = (val & 0x08) ? 1 : 0;
					break;
				case 0x7ffd:
					if (comp->prt0 & 0x20) break;
					comp->prt0 = val;
					comp->vid->curscr = (val & 0x08) ? 1 : 0;
					zxMapMemory(comp);
					break;
				case 0xbffd:
				case 0xfffd: tsOut(comp->ts,ptype,val); break;
			}
			break;
		case HW_P1024:
			switch (ptype) {
				case 0x1f: if (bdiz) bdiOut(comp->bdi,FDC_COM,val); break;
				case 0x3f: if (bdiz) bdiOut(comp->bdi,FDC_TRK,val); break;
				case 0x5f: if (bdiz) bdiOut(comp->bdi,FDC_SEC,val); break;
				case 0x7f: if (bdiz) bdiOut(comp->bdi,FDC_DATA,val); break;
				case 0xff: if (bdiz) bdiOut(comp->bdi,BDI_SYS,val); break;
				case 0xfe:
					comp->vid->nextbrd = val & 0x07;
					if (!(comp->vid->flags & VID_BORDER_4T)) comp->vid->brdcol = val & 0x07;
					comp->beeplev = val & 0x10;
					comp->tape->outsig = (val & 0x08) ? 1 : 0;
					break;
				case 0x7ffd:
					if (comp->prt0 & 0x20) break;
					comp->prt0 = val;
					comp->vid->curscr = (val & 0x08) ? 1 : 0;
					zxMapMemory(comp);
					break;
				case 0xeff7:
					comp->prt1 = val;
					comp->vid->mode = (val & 0x01) ? VID_ALCO : VID_NORMAL;
					zxSetFrq(comp, (val & 0x10) ? 7.0 : 3.5);
					zxMapMemory(comp);
					break;
				case 0xbffd:
				case 0xfffd: tsOut(comp->ts,ptype,val); break;
			}
			break;
		case HW_SCORP:
			switch (ptype) {
				case 0x1f: if (bdiz) bdiOut(comp->bdi,FDC_COM,val); break;
				case 0x3f: if (bdiz) bdiOut(comp->bdi,FDC_TRK,val); break;
				case 0x5f: if (bdiz) bdiOut(comp->bdi,FDC_SEC,val); break;
				case 0x7f: if (bdiz) bdiOut(comp->bdi,FDC_DATA,val); break;
				case 0xff: if (bdiz) bdiOut(comp->bdi,BDI_SYS,val); break;
				case 0xdd: sdrvOut(comp->sdrv,0xfb,val); break;
				case 0xfe:
					comp->vid->nextbrd = val & 0x07;
					if (!(comp->vid->flags & VID_BORDER_4T)) comp->vid->brdcol = val & 0x07;
					comp->beeplev = val & 0x10;
					comp->tape->outsig = (val & 0x08) ? 1 : 0;
					break;
				case 0x7ffd:
					if (comp->prt0 & 0x20) break;
					comp->prt0 = val;
					comp->vid->curscr = (val & 0x08) ? 1 : 0;
					zxMapMemory(comp);
					break;
				case 0x1ffd:
					comp->prt1 = val;
					zxMapMemory(comp);
					break;
				case 0xbffd:
				case 0xfffd: tsOut(comp->ts,ptype,val); break;
			}
			break;
		case HW_PLUS2:
			switch (ptype) {
				case 0xfe:
					comp->vid->nextbrd = val & 0x07;
					if (!(comp->vid->flags & VID_BORDER_4T)) comp->vid->brdcol = val & 0x07;
					comp->beeplev = val & 0x10;
					comp->tape->outsig = (val & 0x08) ? 1 : 0;
					break;
				case 0x7ffd:
					if (comp->prt0 & 0x20) break;
					comp->prt0 = val;
					comp->vid->curscr = (val & 0x08) ? 1 : 0;
					zxMapMemory(comp);
					break;
				case 0x1ffd:
					comp->prt1 = val;
					zxMapMemory(comp);
					break;
				case 0xbffd:
				case 0xfffd: tsOut(comp->ts,ptype,val); break;
			}
			break;
		case HW_PLUS3:
			switch (ptype) {
				case 0xfe:
					comp->vid->nextbrd = val & 0x07;
					if (!(comp->vid->flags & VID_BORDER_4T)) comp->vid->brdcol = val & 0x07;
					comp->beeplev = val & 0x10;
					comp->tape->outsig = (val & 0x08) ? 1 : 0;
					break;
				case 0x7ffd:
					if (comp->prt0 & 0x20) break;
					comp->prt0 = val;
					comp->vid->curscr = (val & 0x08) ? 1 : 0;
					zxMapMemory(comp);
					break;
				case 0x1ffd:
					comp->prt1 = val;
					zxMapMemory(comp);
					break;
				case 0x2ffd:
					if (comp->bdi->fdc->type != FDC_765) break;
					if (val & 1) comp->bdi->fdc->flop[0]->flag |= FLP_MOTOR; else comp->bdi->fdc->flop[0]->flag &= ~FLP_MOTOR;
					if (val & 2) comp->bdi->fdc->flop[1]->flag |= FLP_MOTOR; else comp->bdi->fdc->flop[1]->flag &= ~FLP_MOTOR;
					if (val & 4) comp->bdi->fdc->flop[2]->flag |= FLP_MOTOR; else comp->bdi->fdc->flop[2]->flag &= ~FLP_MOTOR;
					if (val & 8) comp->bdi->fdc->flop[3]->flag |= FLP_MOTOR; else comp->bdi->fdc->flop[3]->flag &= ~FLP_MOTOR;
					break;
				case 0x3ffd:
					if (comp->bdi->fdc->type == FDC_765) fdcWr(comp->bdi->fdc,FDC_DATA,val);
					break;
				case 0xbffd:
				case 0xfffd: tsOut(comp->ts,ptype,val); break;
			}
			break;
		case HW_ATM2:
			switch (ptype) {
				case 0x1f: if (bdiz) bdiOut(comp->bdi,FDC_COM,val); break;
				case 0x3f: if (bdiz) bdiOut(comp->bdi,FDC_TRK,val); break;
				case 0x5f: if (bdiz) bdiOut(comp->bdi,FDC_SEC,val); break;
				case 0x7f: if (bdiz) bdiOut(comp->bdi,FDC_DATA,val); break;
				case 0xff: if (bdiz) {
						bdiOut(comp->bdi,BDI_SYS,val);
						if (comp->prt1 & 0x40) {
							// write palette item
						}
					}
				case 0xfe:
					comp->vid->nextbrd = (val & 0x07) | (~port & 8);
					if (!(comp->vid->flags & VID_BORDER_4T)) comp->vid->brdcol = comp->vid->nextbrd;
					comp->beeplev = val & 0x10;
					comp->tape->outsig = (val & 0x08) ? 1 : 0;
					break;
				case 0x77:
					switch (val & 7) {
						case 0: comp->vid->mode = VID_ATM_EGA; break;
						case 2: comp->vid->mode = VID_ATM_HWM; break;
						case 3: comp->vid->mode = VID_NORMAL; break;
						case 6: comp->vid->mode = VID_ATM_TEXT; break;
						default: comp->vid->mode = VID_ATM_UNDEF; break;
					}
					zxSetFrq(comp,(val & 0x08) ? 7.0 : 3.5);
					comp->prt1 = (port >> 8) & 0xff;
					zxMapMemory(comp);
					break;
				case 0xf7:
					adr = ((comp->prt0 & 0x10) ? 4 : 0) | ((port & 0xc000) >> 14);	// rom2.a15.a14
					comp->memMap[adr] = val;
					zxMapMemory(comp);
					break;
				case 0xef:
					break;
				case 0x7ffd:
					if (comp->prt0 & 0x20) break;
					comp->prt0 = val;
					comp->vid->curscr = (val & 0x08) ? 1 : 0;
					zxMapMemory(comp);
					break;
				case 0xbffd:
				case 0xfffd: tsOut(comp->ts,ptype,val); break;
				default:
					printf("ATM2: out %.4X (%.4X.%i),%.2X\n",port,ptype,bdiz,val);
					assert(0);
					break;
			}
			break;
		default:
			printf("zxOut: unknown hardware type %i\n",comp->hw->type);
			assert(0);
			break;
	}
}

void iowait(ZXComp* comp, int ticks) {
	res5 = vidGetWait(comp->vid);
	if (res5 != 0) {
		vflg |= vidSync(comp->vid, comp->dotPerTick * res5);
		res1 += res5;
	}
	if (ticks != 0)
		vflg |= vidSync(comp->vid, comp->dotPerTick * ticks);
}

void iowr(Z80EX_CONTEXT* cpu, Z80EX_WORD port, Z80EX_BYTE val, void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	res3 = res2 + z80ex_op_tstate(cpu) - 1;		// start of OUT cycle
	vflg |= vidSync(comp->vid,comp->dotPerTick * (res3 - res4));
	res4 = res3;
	// if there is contended io, get wait and wait :)
	if (comp->flags & ZX_CONTIO) {
		switch(port & 0x4001) {
			case 0x0000:
				zxOut(comp,port,val);
				vflg |= vidSync(comp->vid,comp->dotPerTick);	// N:1
				iowait(comp,3);					// C:3
				break;
			case 0x0001:
				zxOut(comp,port,val);
				vflg |= vidSync(comp->vid, comp->dotPerTick * 4);	// N:4
				break;
			case 0x4000:
				zxOut(comp,port,val);
				iowait(comp,1);			// C:1
				iowait(comp,3);			// C:3
				break;
			case 0x4001:
				zxOut(comp,port,val);
				iowait(comp,1);			// C:1	4 times
				iowait(comp,1);
				iowait(comp,1);
				iowait(comp,1);
				break;
		}
		res4 += 4;
	} else {
		vflg |= vidSync(comp->vid, comp->dotPerTick * 3);
		zxOut(comp,port,val);
		res4 += 3;
	}
}

Z80EX_BYTE intrq(Z80EX_CONTEXT* cpu, void* ptr) {
	return 0xff;
}

void rzxClear(ZXComp* zx) {
	int i;
	for (i = 0; i < zx->rzxSize; i++) {
		if (zx->rzxData[i].frmData) free(zx->rzxData[i].frmData);
	}
	if (zx->rzxData) free(zx->rzxData);
	zx->rzxSize = 0;
	zx->rzxData = NULL;
}

ZXComp* zxCreate() {
	ZXComp* comp = (ZXComp*)malloc(sizeof(ZXComp));
	void* ptr = (void*)comp;
	comp->flags = ZX_JUSTBORN;
	comp->cpu = z80ex_create(&memrd,ptr,&memwr,ptr,&iord,ptr,&iowr,ptr,&intrq,ptr);
	zxSetFrq(comp,3.5);
	comp->mem = memCreate();
	comp->vid = vidCreate(comp->mem);
	comp->keyb = keyCreate();
	comp->joy = joyCreate();
	comp->mouse = mouseCreate();
	comp->tape = tapCreate();
	comp->bdi = bdiCreate();
	comp->ide = ideCreate(IDE_NONE);
	comp->ts = tsCreate(TS_NONE,SND_AY,SND_NONE);
	comp->gs = gsCreate();
	comp->sdrv = sdrvCreate(SDRV_105_2);
	comp->rzxSize = 0;
	comp->rzxData = NULL;
	comp->gsCount = 0;
	comp->resbank = RES_48;
	comp->tickCount = 0;
	gsReset(comp->gs);
	return comp;
}

void zxDestroy(ZXComp* comp) {
	z80ex_destroy(comp->cpu);
	memDestroy(comp->mem);
	vidDestroy(comp->vid);
	keyDestroy(comp->keyb);
	joyDestroy(comp->joy);
	mouseDestroy(comp->mouse);
	tapDestroy(comp->tape);
	bdiDestroy(comp->bdi);
	ideDestroy(comp->ide);
	tsDestroy(comp->ts);
	gsDestroy(comp->gs);
	sdrvDestroy(comp->sdrv);
	if (comp->rzxData) free(comp->rzxData);
	free(comp);
}

void zxReset(ZXComp* comp,int wut) {
	int resto = comp->resbank;
	comp->rzxPlay = 0;
	switch (wut) {
		case RES_48: resto = 1; break;
		case RES_128: resto = 0; break;
		case RES_DOS: resto = 3; break;
		case RES_SHADOW: resto = 2; break;
	}
	comp->prt2 = 0;
	comp->prt1 = 0;
	comp->prt0 = ((resto & 1) << 4);
	memSetBank(comp->mem,MEM_BANK0,MEM_ROM,resto);
	memSetBank(comp->mem,MEM_BANK1,MEM_RAM,5);
	memSetBank(comp->mem,MEM_BANK2,MEM_RAM,2);
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,0);
	rzxClear(comp);
	comp->rzxPlay = 0;
	z80ex_reset(comp->cpu);
	comp->vid->curscr = 0;
	comp->vid->mode = VID_NORMAL;
	comp->dosen = 0;
	switch (comp->hw->type) {
		case HW_ATM2:
			if (resto & 1) comp->dosen = 1;
			comp->prt1 = 1;		// PEN (memory manager off)
			break;
		default:
			if (resto & 2) comp->dosen = 1;
			break;
	}
//	if (resto == 3) comp->bdi->flag |= BDI_ACTIVE;
	bdiReset(comp->bdi);
	if (comp->gs->flag & GS_RESET) gsReset(comp->gs);
	tsReset(comp->ts);
	ideReset(comp->ide);
}

void zxSetFrq(ZXComp* comp, float frq) {
	comp->cpuFrq = frq;
	comp->dotPerTick = 7.0 / frq;
}

double zxExec(ZXComp* comp) {
	res1 = res2 = res3 = res4 = res5 = 0;
	comp->vid->drawed = 0;
	vflg = 0;
	do {
		res2 += z80ex_step(comp->cpu);
	} while (z80ex_last_op_type(comp->cpu) != 0);
	pcreg = z80ex_get_reg(comp->cpu,regPC);
	vflg |= vidSync(comp->vid,(res2 - res4) * comp->dotPerTick);
	res1 += res2;
	if (comp->rzxPlay) {
		comp->intStrobe = (comp->rzxFetches < 1);
	} else {
		comp->intStrobe = (vflg & VID_INT) ? 1 : 0;
	}
	comp->frmStrobe = (vflg & VID_FRM) ? 1 : 0;
	if ((pcreg > 0x3fff) && comp->nmiRequest && !comp->rzxPlay) {
		res2 = res3 = res4 = res5 = 0;
		res2 = z80ex_nmi(comp->cpu);
		res1 += res2;
		if (res2 != 0) {
			comp->dosen = 1;
			comp->prt0 |= 0x10;
			zxMapMemory(comp);
			vidSync(comp->vid,(res2 - res4) * comp->dotPerTick);
		}
	}
	if (comp->intStrobe) {
		res2 = res3 = res4 = res5 = 0;
		res2 = z80ex_int(comp->cpu);
		res1 += res2;
		vidSync(comp->vid,(res2 - res4) * comp->dotPerTick);
		if (comp->rzxPlay) {
			comp->rzxFrame++;
			if (comp->rzxFrame >= comp->rzxSize) {
				comp->rzxPlay = 0;
				comp->rzxSize = 0;
				if (comp->rzxData) free(comp->rzxData);
				comp->rzxData = NULL;
			} else {
				comp->rzxFetches = comp->rzxData[comp->rzxFrame].fetches;
				comp->rzxPos = 0;
			}
		}
	}
	pcreg = z80ex_get_reg(comp->cpu,regPC);
	if (memGetCellFlags(comp->mem,pcreg) & MEM_BRK_FETCH) {
		comp->flags |= ZX_BREAK;
	}

	ltk = comp->vid->drawed; // res1 * comp->dotPerTick;
	comp->tickCount += res1;
	tapSync(comp->tape,ltk);

	if (comp->gs->flag & GS_ENABLE) comp->gsCount += ltk;
	if (comp->bdi->fdc->type != FDC_NONE) bdiSync(comp->bdi,ltk);
	return ltk;
}
