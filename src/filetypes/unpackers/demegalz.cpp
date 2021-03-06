#include "unpackers.h"

WORD demegalz(BYTE* src, BYTE* dst) {
//	BYTE* from = src;
	BYTE* to = dst;
	BBStream s(src,PAK_MLZ);
	*(to++) = s.getByte();
	signed short offset = 0;
	unsigned char len;
	bool work = true;
	while (!s.error() && work) {
		while (s.getBit()) {
			*to = s.getByte();
			to++;
		}
		len = s.getBits(2);
		switch (len) {
			case 0x00:
				offset = s.getBits(3) - 8;
				len = 1;
				break;
			case 0x01:
				offset = s.getByte() - 256;
				len = 2;
				break;
			case 0x02:
				if (s.getBit()) {
					offset = (0xf0 + s.getBits(4) - 1) << 8;
					offset += s.getByte();
				} else {
					offset = s.getByte() - 256;
				}
				len = 3;
				break;
			default:
				len = 1;
				while (!s.getBit() && (len < 9)) {
					len++;
				}
				switch (len) {
					case 1: len = 4 + s.getBit(); break;
					case 2: len = 6 + s.getBits(2); break;
					case 3: len = 10 + s.getBits(3); break;
					case 4: len = 18 + s.getBits(4); break;
					case 5: len = 34 + s.getBits(5); break;
					case 6: len = 66 + s.getBits(6); break;
					case 7: offset = 130 + s.getBits(7);
						len = (offset < 255) ? (offset & 0xff) : 0xff;
						break;
					default: len = 0; work = false; break;
				}
				if (s.getBit()) {
					offset = (0xf0 + s.getBits(4) - 1) << 8;
					offset += s.getByte();
				} else {
					offset = s.getByte() - 256;
				}
				break;
		}
		while (len > 0) {
			*to = to[offset]; to++;
			len--;
		}

	}
	return (to - dst);
}
