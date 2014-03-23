#include "filetypes.h"

int loadFDI(Floppy* flp,const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;

	unsigned char* buf = new unsigned char[14];
	Sector sct;
	Sector trkimg[256];
	int i,j,scnt;
	unsigned char fcnt,tmp;
	unsigned short tmpa,tmpb,slen;
	size_t tmpd,tmph,tmpt,tmps,cpos;

	file.read((char*)buf,14);
	if (strncmp((const char*)buf,"FDI",3) != 0) return ERR_FDI_SIGN;
	bool err = (buf[3] != 0);
	tmpa = buf[4] + (buf[5] << 8);		// cylinders
	tmpb = buf[6] + (buf[7] << 8);		// heads
	if ((tmpb != 1) && (tmpb != 2)) return ERR_FDI_HEAD;
	tmpd = buf[10] + (buf[11] << 8);	// sectors data pos
	tmph = buf[12] + (buf[13] << 8) + 14;	// track headers data pos
	file.seekg(tmph);				// read tracks data
	for (i = 0; i < tmpa; i++) {
		for (j = 0; j < tmpb; j++) {
//			trkimg.clear();
			tmpt = tmpd + getlen(&file,4);
			file.seekg(2,std::ios_base::cur);		// skip 2 bytes
			tmp = file.get();			// sectors in disk;
			for (scnt=0; scnt < tmp; scnt++) {
				sct.cyl = file.get();
				sct.side = file.get();
				sct.sec = file.get();
				sct.len = file.get();
				fcnt = file.get();				// flag
				sct.type = (fcnt & 0x80) ? 0xf8 : 0xfb;
				tmps = tmpt + getlen(&file,2);
				cpos = file.tellg();			// remember current pos
				file.seekg(tmps);
				slen = (128 << sct.len);		// sector len
				sct.data = new unsigned char[slen];
				file.read((char*)sct.data,slen);	// read sector data
				file.seekg(cpos);
				trkimg[scnt] = sct;
				//trkimg.push_back(sct);
			}
			flpFormTrack(flp, (i << 1) + j, trkimg, tmp);
		}
	}
	flp->protect = err ? 1 : 0;
	flp->insert = 1;
	loadBoot(flp);
	flp->changed = 0;
	flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
	strcpy(flp->path,name);

	return ERR_OK;
}
