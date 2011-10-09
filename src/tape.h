#ifndef _TAPE_H
#define _TAPE_H

#include <vector>
#include <string>
#include <fstream>
#include <stdint.h>

#define	TYPE_TAP	0
#define	TYPE_TZX	1

#define	TAPE_ON		1
#define TAPE_REC	(1<<1)
#define TAPE_CANSAVE	(1<<2)	// can be saved as tap
#define TAPE_WAIT	(1<<3)	// wait for 1st impulse (at save)

#define	TBF_BREAK	1	// stop tape on start of this block

#define	TAPE_HEAD	0
#define	TAPE_DATA	1

struct TapeBlockInfo {
	int type;
	int flags;
	std::string name;
	int size;
	int time;
	int curtime;
};

class TapeBlock {
	public:
		TapeBlock();
		uint64_t pause;					// pause in ms after block
		int flags;
		uint32_t plen,s1len,s2len,len0,len1;	// signals len
		uint32_t pdur;				// pilot tone length
		int32_t datapos;					// 1st data signal pos
		std::vector<uint32_t> data;			// signals
		int32_t gettime(int32_t);
		int32_t getsize();
		uint8_t getbyte(int32_t);
		std::string getheader();
		void normSignals();
};

class Tape {
	public:
		Tape();
		std::string path;
		uint32_t block;		// tape block
		uint32_t pos;		// tap position
		int32_t flags;
		bool signal;
		bool toutold;
		bool outsig;
		int32_t lastick;		// cpu.tick at last sync
		int64_t siglen;		// остаток длины текущего сигнала
		TapeBlock tmpblock;	// временный блок. сюда ведется запись
		std::vector<TapeBlock> data;
		uint8_t getbyte(int32_t,int32_t);
		std::vector<uint8_t> getdata(int32_t,int32_t,int32_t);
		void eject();
		void stop(int);
		bool startplay();
		void startrec();
		void sync();
		void storeblock();
		void swapblocks(int32_t,int32_t);
		TapeBlock parse(std::ifstream*,uint32_t,std::vector<int32_t>,uint8_t);
		void load(std::string,uint8_t);
		void save(std::string,uint8_t);
		std::vector<TapeBlockInfo> getInfo();
};

//extern Tape *tape;

#endif
