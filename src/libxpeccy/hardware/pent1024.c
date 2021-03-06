#include "../spectrum.h"

void p1mMapMem(ZXComp* comp) {
	if (comp->prt1 & 8) {
		memSetBank(comp->mem,MEM_BANK0,MEM_RAM,0);
	} else {
		memSetBank(comp->mem,MEM_BANK0,MEM_ROM,(comp->dosen ? 2 : 0) | ((comp->prt0 & 0x10) ? 1 : 0));
	}
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,(comp->prt0 & 7) | ((comp->prt1 & 4) ? 0 : ((comp->prt0 & 0x20) | ((comp->prt0 & 0xc0) >> 3))));
}

// in

Z80EX_BYTE p1mInFE(ZXComp* comp, Z80EX_WORD port) {
	return keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->levPlay ? 0x40 : 0x00);
}

Z80EX_BYTE p1mIn1F(ZXComp* comp, Z80EX_WORD port) {
	return joyInput(comp->joy);
}

Z80EX_BYTE p1mInFFFD(ZXComp* comp, Z80EX_WORD port) {
	return tsIn(comp->ts, 0xfffd);
}

Z80EX_BYTE p1mInFADF(ZXComp* comp, Z80EX_WORD port) {
	comp->mouse->used = 1;
	return comp->mouse->enable ? comp->mouse->buttons : 0xff;
}

Z80EX_BYTE p1mInFBDF(ZXComp* comp, Z80EX_WORD port) {
	comp->mouse->used = 1;
	return comp->mouse->enable ? comp->mouse->xpos : 0xff;
}

Z80EX_BYTE p1mInFFDF(ZXComp* comp, Z80EX_WORD port) {
	comp->mouse->used = 1;
	return comp->mouse->enable ? comp->mouse->ypos : 0xff;
}

Z80EX_BYTE p1mInBFF7(ZXComp* comp, Z80EX_WORD port) {
	return (comp->prt1 & 0x80) ? cmsRd(comp) : 0xff;
}

// out

void p1mOutFE(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->vid->nextbrd = val & 0x07;
	if (!comp->vid->border4t) comp->vid->brdcol = val & 0x07;
	comp->beeplev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}

void p1mOut7FFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	if ((comp->prt1 & 4) && (comp->prt0 & 0x20)) return;
	comp->prt0 = val;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	p1mMapMem(comp);
}

void p1mOutBFFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	tsOut(comp->ts, 0xbffd, val);
}

void p1mOutFFFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	tsOut(comp->ts, 0xfffd, val);
}

void p1mOutBFF7(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	if (comp->prt1 & 0x80) cmsWr(comp,val);
}

void p1mOutDFF7(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	if (comp->prt1 & 0x80) comp->cmos.adr = val;
}

void p1mOutEFF7(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->prt1 = val;
	vidSetMode(comp->vid,(val & 0x01) ? VID_ALCO : VID_NORMAL);
	zxSetFrq(comp, (val & 0x10) ? 7.0 : 3.5);
	p1mMapMem(comp);
}

xPort p1mPortMap[] = {
	{0x0003,0x00fe,1,0,&p1mInFE,	&p1mOutFE},
	{0x8002,0x7ffd,1,0,NULL,	&p1mOut7FFD},
	{0xc002,0xbffd,1,0,NULL,	&p1mOutBFFD},
	{0xc002,0xfffd,1,0,&p1mInFFFD,	&p1mOutFFFD},
	{0xf008,0xeff7,1,0,NULL,	&p1mOutEFF7},
	{0xf008,0xbff7,1,0,&p1mInBFF7,	&p1mOutBFF7},
	{0xf008,0xdff7,1,0,NULL,	&p1mOutDFF7},
	{0x05a3,0xfadf,0,0,&p1mInFADF,	NULL},
	{0x05a3,0xfbdf,0,0,&p1mInFBDF,	NULL},
	{0x05a3,0xffdf,0,0,&p1mInFFDF,	NULL},
	{0x00ff,0x001f,0,0,&p1mIn1F,	NULL},		// TODO : ORLY (x & FF = 1F)
	{0x0000,0x0000,1,0,NULL,NULL}
};

void p1mOut(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int dos) {
	bdiOut(comp->bdi, port, val, dos);
	hwOut(p1mPortMap, comp, port, val, dos);
}

Z80EX_BYTE p1mIn(ZXComp* comp, Z80EX_WORD port, int dos) {
	Z80EX_BYTE res = 0xff;
	if (bdiIn(comp->bdi, port, &res, dos)) return res;
	res = hwIn(p1mPortMap, comp, port, dos);
	return res;
}
