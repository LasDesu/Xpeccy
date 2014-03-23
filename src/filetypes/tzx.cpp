#include <stdlib.h>
#include "filetypes.h"

unsigned int getlen(std::ifstream *file,unsigned char n) {
	unsigned int len = (unsigned)(file->get());
	if (n > 1) len |= (file->get() << 8);
	if (n > 2) len |= (file->get() << 16);
	if (n > 3) len |= (file->get() << 24);
	return len;
}

int loadTZX(Tape* tape, const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;

	int sigLens[] = {PILOTLEN,SYNC1LEN,SYNC2LEN,SIGN0LEN,SIGN1LEN,SYNC3LEN,-1};
	int altLens[] = {PILOTLEN,SYNC1LEN,SYNC2LEN,SIGN0LEN,SIGN1LEN,SYNC3LEN,-1};
	unsigned char* buf = new unsigned char[256];
	unsigned char tmp;
	unsigned int paulen,len;
	int i;
	unsigned int loopc = 0;
	TapeBlock block,altBlock;
	char* blockBuf = NULL;
	std::streampos loopos = 0;

	file.read((char*)buf,10);
	if ((strncmp((const char*)buf,"ZXTape!",7) != 0) || (buf[7]!=0x1a)) return ERR_TZX_SIGN;

	tapEject(tape);
	tape->path = (char*)realloc(tape->path,sizeof(char) * (strlen(name) + 1));
	strcpy(tape->path,name);
	tape->isData = 1;

	while (!file.eof()) {
		tmp = file.get();		// block type (will be FF @ eof)
		if (file.eof()) break;		// is it the end?
		switch (tmp) {
			case 0x10:
				paulen = getlen(&file,2);
				len = getlen(&file,2);
				blockBuf = (char*)realloc(blockBuf,len * sizeof(char));
				file.read(blockBuf,len);
				block = tapDataToBlock(blockBuf,len,sigLens);
				block.pause = paulen;
				blkAddSignal(&block,sigLens[5]);
				tapAddBlock(tape,block);
				blkClear(&block);
				//block.data.clear();
				break;
			case 0x11:
				altLens[0] = getlen(&file,2);	// pilot
				altLens[1] = getlen(&file,2);	// sync1
				altLens[2] = getlen(&file,2);	// sync2
				altLens[3] = getlen(&file,2);	// 0
				altLens[4] = getlen(&file,2);	// 1
				altLens[6] = getlen(&file,2);	// pilot pulses
				file.get();
				paulen = getlen(&file,2);
				len = getlen(&file,3);
				blockBuf = (char*)realloc(blockBuf,len * sizeof(char));
				file.read(blockBuf,len);
				block = tapDataToBlock(blockBuf,len,altLens);
				block.pause = paulen;
				blkAddSignal(&block,sigLens[5]);
				tapAddBlock(tape,block);
				blkClear(&block);
				tape->isData = 0;
				break;
			case 0x12:
				paulen = getlen(&file,2);
				len = getlen(&file,2);
				while (len>0) {
					blkAddSignal(&block,paulen);
					len--;
				}
				tape->isData = 0;
				break;
			case 0x13:
				len = file.get();
				while (len>0) {
					paulen = getlen(&file,2);
					blkAddSignal(&block,paulen);
					len--;
				}
				tape->isData = 0;
				break;
			case 0x14:
				altLens[0] = 0;
				altLens[1] = 0;
				altLens[2] = 0;
				altLens[5] = 0;
				altLens[6] = 0;		// no pilot, syncs
				altLens[3] = getlen(&file,2);
				altLens[4] = getlen(&file,2);
				file.get();
				paulen = getlen(&file,2);
				len = getlen(&file,3);
				blockBuf = (char*)realloc(blockBuf,len * sizeof(char));
				file.read(blockBuf,len);
				altBlock = tapDataToBlock(blockBuf,len,altLens);
				block.len0 = altBlock.len0;
				block.len1 = altBlock.len1;
				block.dataPos = -1;
				for (i = 0; i < altBlock.sigCount; i++) {
					blkAddSignal(&block,altBlock.sigData[i]);
				}
				block.pause = paulen;
				tape->isData = 0;
				break;
/*
			case 0x15:
				file.seekg(5,std::ios::cur);
				len = getlen(&file,3);
				file.seekg(len,std::ios::cur);
				break;
			case 0x18:
				len = getlen(&file,4);
				file.seekg(len,std::ios::cur);
				break;
			case 0x19:
				len = getlen(&file,4);
				file.seekg(len,std::ios::cur);
				break;
*/
			case 0x20:
				len = getlen(&file,2);
				blkAddSignal(&block,len);
				break;
			case 0x21:
				len = file.get();
				file.seekg(len,std::ios::cur);
				break;
			case 0x22:
				if (block.sigCount != 0) {
					blkAddSignal(&block,sigLens[5]);
					block.hasBytes = 0;
					tapAddBlock(tape,block);
					blkClear(&block);
				}
				break;
			case 0x23:
				file.seekg(2,std::ios::cur);
				break;
			case 0x24:
				loopc = getlen(&file,2);
				loopos = file.tellg();
				break;
			case 0x25:
				if (loopc>0) {
					loopc--;
					if (loopc != 0) file.seekg(loopos,std::ios::beg);
				}
				break;
//			case 0x26:			// TODO: block calls sequence
//				len = getlen(&file,2);
//				file.seekg(len << 1,std::ios::cur);
//				break;
			case 0x27:
				break;
//			case 0x28:			// TODO: select blocks
//				len = getlen(&file,2);
//				file.seekg(len<<1,std::ios::cur);
//				break;
			case 0x2a:
				file.seekg(4,std::ios::cur);
				break;
			case 0x2b:
				file.seekg(5,std::ios::cur);
				break;
			case 0x30:
				len = file.get();
				file.seekg(len,std::ios::cur);
				break;
			case 0x31:
				file.get();
				len = file.get();
				file.seekg(len,std::ios::cur);
				break;
			case 0x32:
				len = getlen(&file,2);
				file.seekg(len,std::ios::cur);
				break;
			case 0x33:
				len = getlen(&file,1);
				file.seekg(len*3,std::ios::cur);
				break;
			case 0x34:
				file.seekg(4,std::ios::cur);
				len = getlen(&file,4);
				file.seekg(len,std::ios::cur);
				break;
			case 0x5a:
				file.seekg(9,std::ios::cur);
				break;
			default:
				printf("TZX unsupported block: %.2X\n",tmp);
				return ERR_TZX_UNKNOWN;
		}
	}
	if (block.sigCount != 0) {
//		blkAddSignal(&block,sigLens[5]);		// FIXME: not really
		block.hasBytes = 0;
		tapAddBlock(tape,block);
		blkClear(&block);
	}
	if (blockBuf != NULL) free(blockBuf);
	return ERR_OK;
}
