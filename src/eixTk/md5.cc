// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

#ifdef DEBUG_MD5
#include <iostream>
#endif

#include "eixTk/inttypes.h"
#include "eixTk/md5.h"
#include "eixTk/null.h"

using std::string;

inline static uint32_t
md5F(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) | ((~x) & z);
}

inline static uint32_t
md5G(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & z) | (y & (~z));
}

inline static uint32_t
md5H(uint32_t x, uint32_t y, uint32_t z)
{
	return x ^ y ^ z;
}

inline static uint32_t
md5I(uint32_t x, uint32_t y, uint32_t z)
{
	return y ^ (x | (~z));
}

inline static uint32_t
md5rotate(uint32_t x, unsigned int c)
{
	x &= 0xFFFFFFFFUL;
	return (x << c) | (x >> (32-c));
}

#define md5call(func, a, b, c, d, s, ac, x) do { \
	a += func(b, c, d) + x + ac; \
	a = (md5rotate(a, s) + b) & 0xFFFFFFFFUL; \
} while(0)

static const uint32_t sinlistF[16] = {
	0xD76AA478UL, 0xE8C7B756UL, 0x242070DBUL, 0xC1BDCEEEUL,
	0xF57C0FAFUL, 0x4787C62AUL, 0xA8304613UL, 0xFD469501UL,
	0x698098D8UL, 0x8B44F7AFUL, 0xFFFF5BB1UL, 0x895CD7BEUL,
	0x6B901122UL, 0xFD987193UL, 0xA679438EUL, 0x49B40821UL
};

static const uint32_t sinlistG[16] = {
	0xF61E2562UL, 0xC040B340UL, 0x265E5A51UL, 0xE9B6C7AAUL,
	0xD62F105DUL, 0x02441453UL, 0xD8A1E681UL, 0xE7D3FBC8UL,
	0x21E1CDE6UL, 0xC33707D6UL, 0xF4D50D87UL, 0x455A14EDUL,
	0xA9E3E905UL, 0xFCEFA3F8UL, 0x676F02D9UL, 0x8D2A4C8AUL
};

static const uint32_t sinlistH[16] = {
	0xFFFA3942UL, 0x8771F681UL, 0x6D9D6122UL, 0xFDE5380CUL,
	0xA4BEEA44UL, 0x4BDECFA9UL, 0xF6BB4B60UL, 0xBEBFBC70UL,
	0x289B7EC6UL, 0xEAA127FAUL, 0xD4EF3085UL, 0x04881D05UL,
	0xD9D4D039UL, 0xE6DB99E5UL, 0x1FA27CF8UL, 0xC4AC5665UL
};

static const uint32_t sinlistI[16] = {
	0xF4292244UL, 0x432AFF97UL, 0xAB9423A7UL, 0xFC93A039UL,
	0x655B59C3UL, 0x8F0CCC92UL, 0xFFEFF47DUL, 0x85845DD1UL,
	0x6FA87E4FUL, 0xFE2CE6E0UL, 0xA3014314UL, 0x4E0811A1UL,
	0xF7537E82UL, 0xBD3AF235UL, 0x2AD7D2BBUL, 0xEB86D391UL
};

static const unsigned int permutG[16] = {
	1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12
};

static const unsigned int permutH[16] = {
	5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2
};

static const unsigned int permutI[16] = {
	0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9
};

static const uint32_t md5init[4] = {
	0x67452301UL, 0xEFCDAB89UL, 0x98BADCFEUL, 0x10325476UL
};

static void
md5fill(const char *buffer, uint32_t *mybuf, unsigned int len)
{
	while(len != 0) {
		*(mybuf++) =
			buffer[3] * 0x1000000UL +
			buffer[2] * 0x10000UL +
			buffer[1] * 0x100UL +
			buffer[0];
		buffer += 4;
		--len;
	}
}

static void
md5chunk(const uint32_t *mybuf, uint32_t *resarr)
{
	uint32_t a(resarr[0]);
	uint32_t b(resarr[1]);
	uint32_t c(resarr[2]);
	uint32_t d(resarr[3]);

	int i(0);
	while(i < 16) {
		md5call(md5F, a, b, c, d,  7, sinlistF[i], mybuf[i]);
		++i;
		md5call(md5F, d, a, b, c, 12, sinlistF[i], mybuf[i]);
		++i;
		md5call(md5F, c, d, a, b, 17, sinlistF[i], mybuf[i]);
		++i;
		md5call(md5F, b, c, d, a, 22, sinlistF[i], mybuf[i]);
		++i;
	}
	i = 0;
	while(i < 16) {
		md5call(md5G, a, b, c, d,  5, sinlistG[i], mybuf[permutG[i]]);
		++i;
		md5call(md5G, d, a, b, c,  9, sinlistG[i], mybuf[permutG[i]]);
		++i;
		md5call(md5G, c, d, a, b, 14, sinlistG[i], mybuf[permutG[i]]);
		++i;
		md5call(md5G, b, c, d, a, 20, sinlistG[i], mybuf[permutG[i]]);
		++i;
	}
	i = 0;
	while(i < 16) {
		md5call(md5H, a, b, c, d,  4, sinlistH[i], mybuf[permutH[i]]);
		++i;
		md5call(md5H, d, a, b, c, 11, sinlistH[i], mybuf[permutH[i]]);
		++i;
		md5call(md5H, c, d, a, b, 16, sinlistH[i], mybuf[permutH[i]]);
		++i;
		md5call(md5H, b, c, d, a, 23, sinlistH[i], mybuf[permutH[i]]);
		++i;
	}
	i = 0;
	while(i < 16) {
		md5call(md5I, a, b, c, d,  6, sinlistI[i], mybuf[permutI[i]]);
		++i;
		md5call(md5I, d, a, b, c, 10, sinlistI[i], mybuf[permutI[i]]);
		++i;
		md5call(md5I, c, d, a, b, 15, sinlistI[i], mybuf[permutI[i]]);
		++i;
		md5call(md5I, b, c, d, a, 21, sinlistI[i], mybuf[permutI[i]]);
		++i;
	}

	resarr[0] = (resarr[0] + a) & 0xFFFFFFFFUL;
	resarr[1] = (resarr[1] + b) & 0xFFFFFFFFUL;
	resarr[2] = (resarr[2] + c) & 0xFFFFFFFFUL;
	resarr[3] = (resarr[3] + d) & 0xFFFFFFFFUL;
}

typedef off_t Md5DataLen;

static void
calc_md5sum(const char *buffer, Md5DataLen totalsize, uint32_t *resarr)
{
	resarr[0] = md5init[0];
	resarr[1] = md5init[1];
	resarr[2] = md5init[2];
	resarr[3] = md5init[3];
	uint32_t mybuf[16];
	Md5DataLen restsize(totalsize);
	while(restsize >= 64) {
		md5fill(buffer, mybuf, 16);
		md5chunk(mybuf, resarr);
		restsize -= 64;
		buffer += 64;
	}
	unsigned int len(restsize/4);
	md5fill(buffer, mybuf, len);
	buffer += 4 * len;
	switch(restsize % 4) {
		case 3:
			mybuf[len] = 0x80UL * 0x1000000UL +
				buffer[2] * 0x10000UL +
				buffer[1] * 0x100UL +
				buffer[0];
			break;
		case 2:
			mybuf[len] = 0x80UL * 0x10000UL +
				buffer[1] * 0x100UL +
				buffer[0];
			break;
		case 1:
			mybuf[len] = 0x80UL * 0x100UL +
				buffer[0];
			break;
		/* case 0: */
		default:
			mybuf[len] = 0x80UL;
			break;
	}
	if(++len > 14) {
		while(len < 16) {
			mybuf[len++] = 0;
		}
		md5chunk(mybuf, resarr);
		len = 0;
	}
	while(len < 14) {
		mybuf[len++] = 0;
	}
	mybuf[14] = (totalsize % 8192) * 8;
	mybuf[15] = totalsize / 8192;
	md5chunk(mybuf, resarr);
}

#ifdef DEBUG_MD5
#include "eixTk/unused.h"

using std::cout;
using std::endl;

static void
debug_md5(const uint32_t *resarr)
{
	cout << "MD5 Debug: ";
	for(int i(0); i < 4; ++i) {
		uint32_t res(resarr[i]);
		for(int j(0); j < 8; ++j) {
			char c;
			if((j & 1) == 0) {
				c = ((res / 16) % 16);
			} else {
				c = (res % 16);
				res /= 256;
			}
			if(c < 10) {
				cout << static_cast<char>('0' + c);
			} else {
				c -= 10;
				cout << static_cast<char>('a' + c);
			}
		}
	}
	cout << endl;
}

bool
verify_md5sum(const char *file ATTRIBUTE_UNUSED, const char *md5sum ATTRIBUTE_UNUSED)
{
	UNUSED(file);
	UNUSED(md5sum);
	const char buffer[] = "";
	uint32_t resarr[4];
	calc_md5sum(buffer, 0, resarr);
	debug_md5(resarr);
	return false;
}
#endif

bool
verify_md5sum(const char *file, const string &md5sum)
{
	if(md5sum.size() != 32) {
		return false;
	}
	if(md5sum.find_first_not_of("0123456789abcdefABCDEF") != string::npos) {
		return false;
	}
	char *filebuffer(NULLPTR);
	int fd(open(file, O_RDONLY));
	if(fd == -1) {
		return false;
	}
	struct stat st;
	if(fstat(fd, &st)) {
		close(fd);
		return false;
	}
	if(st.st_size != 0) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
		filebuffer = static_cast<char *>(mmap(NULLPTR, st.st_size, PROT_READ, MAP_SHARED, fd, 0));
#pragma GCC diagnostic pop
		close(fd);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
		if (filebuffer == MAP_FAILED) {
#pragma GCC diagnostic pop
			return false;
		}
	}
	uint32_t resarr[4];
	calc_md5sum(filebuffer, st.st_size, resarr);
	if(filebuffer != NULLPTR) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
		munmap(filebuffer, st.st_size);
#pragma GCC diagnostic pop
	}
	string::size_type curr(0);
	for(int i(0); i < 4; ++i) {
		uint32_t res(resarr[i]);
		for(int j(0); j < 8; ++j) {
			char c;
			if((j & 1) == 0) {
				c = ((res / 16) % 16);
			} else {
				c = (res % 16);
				res /= 256;
			}
			if(c < 10) {
				if(md5sum[curr] != ('0' + c)) {
					return false;
				}
			} else {
				c -= 10;
				if(	(md5sum[curr] != ('a' + c)) &&
					(md5sum[curr] != ('A' + c)) ) {
					return false;
				}
			}
			++curr;
		}
	}
	return true;
}
